/*
 * Copyright (c) 2004-2019 Apple, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#include <err.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/acl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <syslog.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/xattr.h>
#include <sys/attr.h>
#include <sys/syscall.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/acl.h>
#include <libkern/OSByteOrder.h>
#include <membership.h>
#include <fts.h>
#include <libgen.h>
#include <sys/clonefile.h>
#include <System/sys/content_protection.h>

#ifdef VOL_CAP_FMT_DECMPFS_COMPRESSION
# include <Kernel/sys/decmpfs.h>
#endif

#include <TargetConditionals.h>
#if !TARGET_OS_IPHONE
#include <quarantine.h>

#define	XATTR_QUARANTINE_NAME qtn_xattr_name
#else /* TARGET_OS_IPHONE */
#define qtn_file_t void *
#define QTN_SERIALIZED_DATA_MAX 0
static void * qtn_file_alloc(void) { return NULL; }
static int qtn_file_init_with_fd(__unused void *x, __unused int y) { return -1; }
static int qtn_file_init_with_path(__unused void *x, __unused const char *path) { return -1; }
static int qtn_file_init_with_data(__unused void *x, __unused const void *data, __unused size_t len) { return -1; }
static void qtn_file_free(__unused void *x) { return; }
static int qtn_file_apply_to_fd(__unused void *x, __unused int y) { return 0; }
static char *qtn_error(__unused int x) { return NULL; }
static int qtn_file_to_data(__unused void *x, __unused char *y, __unused size_t *z) { return -1; }
static void *qtn_file_clone(__unused void *x) { return NULL; }
static uint32_t qtn_file_get_flags(__unused void *x) { return 0; }
static int qtn_file_set_flags(__unused void *x, __unused uint32_t flags) { return 0; }
#define	XATTR_QUARANTINE_NAME "figgledidiggledy"
#define QTN_FLAG_DO_NOT_TRANSLOCATE 0
#endif /* TARGET_OS_IPHONE */

#include "copyfile.h"
#include "copyfile_private.h"
#include "xattr_flags.h"

#define XATTR_ROOT_INSTALLED_NAME "com.apple.root.installed"

enum cfInternalFlags {
	cfDelayAce                = 1 << 0, /* set if ACE shouldn't be set until post-order traversal */
	cfMakeFileInvisible       = 1 << 1, /* set if kFinderInvisibleMask is on src */
	cfSawDecmpEA              = 1 << 2, /* set if we've seen a com.apple.decmpfs xattr */
	cfSrcProtSupportValid     = 1 << 3, /* set if cfSrcSupportsCProtect is valid */
	cfSrcSupportsCProtect     = 1 << 4, /* set if src supports MNT_CPROTECT */
	cfDstProtSupportValid     = 1 << 5, /* set if cfDstSupportsCProtect is valid */
	cfDstSupportsCProtect     = 1 << 6, /* set if dst supports MNT_CPROTECT */
};

#define COPYFILE_MNT_CPROTECT_MASK (cfSrcProtSupportValid | cfSrcSupportsCProtect | cfDstProtSupportValid | cfDstSupportsCProtect)

/*
 * The state structure keeps track of
 * the source filename, the destination filename, their
 * associated file-descriptors, the stat information for the
 * source file, the security information for the source file,
 * the flags passed in for the copy, a pointer to place statistics
 * (not currently implemented), debug flags, and a pointer to callbacks
 * (not currently implemented).
 */
struct _copyfile_state
{
	char *src;
	char *dst;
	int src_fd;
	int dst_fd;
	struct stat sb;
	filesec_t fsec;
	copyfile_flags_t flags;
	unsigned int	internal_flags;
	void *stats;
	uint32_t debug;
	copyfile_callback_t	statuscb;
	void	*ctx;
	qtn_file_t qinfo;	/* Quarantine information -- probably NULL */
	filesec_t original_fsec;
	filesec_t permissive_fsec;
	off_t totalCopied;
	int err;
	char *xattr_name;
	xattr_operation_intent_t copyIntent;
	bool was_cloned;
};

#define GET_PROT_CLASS(fd) fcntl((fd), F_GETPROTECTIONCLASS)
#define SET_PROT_CLASS(fd, prot_class) fcntl((fd), F_SETPROTECTIONCLASS, (prot_class))

struct acl_entry {
	u_int32_t       ae_magic;
#define _ACL_ENTRY_MAGIC        0xac1ac101
	u_int32_t       ae_tag;
	guid_t          ae_applicable;
	u_int32_t       ae_flags;
	u_int32_t       ae_perms;
};

#define PACE(ace) \
	do { \
		struct acl_entry *__t = (struct acl_entry*)(ace); \
		fprintf(stderr, "%s(%d): " #ace " = { flags = %#x, perms = %#x }\n", __FUNCTION__, __LINE__, __t->ae_flags, __t->ae_perms); \
	} while (0)

#define PACL(ace) \
	do { \
		ssize_t __l; char *__cp = acl_to_text(ace, &__l); \
		fprintf(stderr, "%s(%d):  " #ace " = %s\n", __FUNCTION__, __LINE__, __cp ? __cp : "(null)"); \
	} while (0)

static int
acl_compare_permset_np(acl_permset_t p1, acl_permset_t p2)
{
	struct pm { u_int32_t ap_perms; } *ps1, *ps2;
	ps1 = (struct pm*) p1;
	ps2 = (struct pm*) p2;

	return ((ps1->ap_perms == ps2->ap_perms) ? 1 : 0);
}


static int
doesdecmpfs(int fd) {
#ifdef DECMPFS_XATTR_NAME
	int rv;
	struct attrlist attrs;
	char volroot[MAXPATHLEN + 1];
	struct statfs sfs;
	struct {
		uint32_t length;
		vol_capabilities_attr_t volAttrs;
	} volattrs;

	(void)fstatfs(fd, &sfs);
	strlcpy(volroot, sfs.f_mntonname, sizeof(volroot));

	memset(&attrs, 0, sizeof(attrs));
	attrs.bitmapcount = ATTR_BIT_MAP_COUNT;
	attrs.volattr = ATTR_VOL_CAPABILITIES;

	rv = getattrlist(volroot, &attrs, &volattrs, sizeof(volattrs), 0);

	if (rv != -1 &&
		(volattrs.volAttrs.capabilities[VOL_CAPABILITIES_FORMAT] & VOL_CAP_FMT_DECMPFS_COMPRESSION) &&
		(volattrs.volAttrs.valid[VOL_CAPABILITIES_FORMAT] & VOL_CAP_FMT_DECMPFS_COMPRESSION)) {
		return 1;
	}
#endif
	return 0;
}

static int
does_copy_protection(int fd)
{
	struct statfs sfs;

	if (fstatfs(fd, &sfs) == -1)
		return -1;

	return ((sfs.f_flags & MNT_CPROTECT) == MNT_CPROTECT);
}

static void
sort_xattrname_list(void *start, size_t length)
{
	char **ptrs = NULL;
	int nel;
	char *tmp;
	int indx = 0;

	/* If it's not a proper C string at the end, don't do anything */
	if (((char*)start)[length] != 0)
		return;
	/*
	 * In order to sort the list of names, we need to
	 * make a list of pointers to strings.  To do that,
	 * we need to run through the buffer, and find the
	 * beginnings of strings.
	 */
	nel = 10;	// Most files don't have many EAs
	ptrs = (char**)calloc(nel, sizeof(char*));

	if (ptrs == NULL)
		goto done;

#ifdef DEBUG
	{
		char *curPtr = start;
		while (curPtr < (char*)start + length) {
			printf("%s\n", curPtr);
			curPtr += strlen(curPtr) + 1;
		}
	}
#endif

	tmp = ptrs[indx++] = (char*)start;

	while ((tmp = memchr(tmp, 0, ((char*)start + length) - tmp))) {
		if (indx == nel) {
			nel += 10;
			ptrs = realloc(ptrs, sizeof(char**) * nel);
			if (ptrs == NULL)
				goto done;
		}
		ptrs[indx++] = ++tmp;
	}
#ifdef DEBUG
	printf("Unsorted:\n");
	for (nel = 0; nel < indx-1; nel++) {
		printf("\tEA %d = `%s'\n", nel, ptrs[nel]);
	}
#endif
	qsort_b(ptrs, indx-1, sizeof(char*), ^(const void *left, const void *right) {
		int rv;
		char *lstr = *(char**)left, *rstr = *(char**)right;
		rv = strcmp(lstr, rstr);
		return rv;
	});
#ifdef DEBUG
	printf("Sorted:\n");
	for (nel = 0; nel < indx-1; nel++) {
		printf("\tEA %d = `%s'\n", nel, ptrs[nel]);
	}
#endif
	/*
	 * Now that it's sorted, we need to make a copy, so we can
	 * move the strings around into the new order.  Then we
	 * copy that on top of the old buffer, and we're done.
	 */
	char *copy = malloc(length);
	if (copy) {
		int i;
		char *curPtr = copy;

		for (i = 0; i < indx-1; i++) {
			size_t len = strlen(ptrs[i]);
			memcpy(curPtr, ptrs[i], len+1);
			curPtr += len+1;
		}
		memcpy(start, copy, length);
		free(copy);
	}

done:
	if (ptrs)
		free(ptrs);
	return;
}

/*
 * Internally, the process is broken into a series of
 * private functions.
 */
static int copyfile_open	(copyfile_state_t);
static int copyfile_close	(copyfile_state_t);
static int copyfile_data	(copyfile_state_t);
static int copyfile_stat	(copyfile_state_t);
static int copyfile_security	(copyfile_state_t);
static int copyfile_xattr	(copyfile_state_t);
static int copyfile_pack	(copyfile_state_t);
static int copyfile_unpack	(copyfile_state_t);

static copyfile_flags_t copyfile_check	(copyfile_state_t);
static filesec_t copyfile_fix_perms(copyfile_state_t, filesec_t *);
static int copyfile_preamble(copyfile_state_t *s, copyfile_flags_t flags);
static int copyfile_internal(copyfile_state_t state, copyfile_flags_t flags);
static int copyfile_unset_posix_fsec(filesec_t);
static int copyfile_quarantine(copyfile_state_t);

#define COPYFILE_DEBUG (1<<31)
#define COPYFILE_DEBUG_VAR "COPYFILE_DEBUG"

// These macros preserve the value of errno.
#ifndef _COPYFILE_TEST
# define copyfile_warn(str, ...) \
	do { \
		errno_t _errsv = errno; \
		syslog(LOG_WARNING, str ": %m", ## __VA_ARGS__); \
		errno = _errsv; \
	} while (0)
# define copyfile_debug(d, str, ...) \
	do { \
		if (s && (d <= s->debug)) {\
			errno_t _errsv = errno; \
			syslog(LOG_DEBUG, "%s:%d:%s() " str "\n", __FILE__, __LINE__ , __FUNCTION__, ## __VA_ARGS__); \
			errno = _errsv; \
		} \
	} while (0)
#else
#define copyfile_warn(str, ...) \
	do { \
		errno_t _errsv = errno; \
		fprintf(stderr, "%s:%d:%s() " str ": %s\n", __FILE__, __LINE__ , __FUNCTION__, ## __VA_ARGS__, (errno) ? strerror(errno) : ""); \
		errno = _errsv; \
	} while (0)
# define copyfile_debug(d, str, ...) \
	do { \
		if (s && (d <= s->debug)) {\
			errno_t _errsv = errno; \
			fprintf(stderr, "%s:%d:%s() " str "\n", __FILE__, __LINE__ , __FUNCTION__, ## __VA_ARGS__); \
			errno = _errsv; \
		} \
	} while(0)
#endif

static int copyfile_quarantine(copyfile_state_t s)
{
	int rv = 0;
	if (s->qinfo == NULL)
	{
		int error;
		s->qinfo = qtn_file_alloc();
		if (s->qinfo == NULL)
		{
			rv = -1;
			goto done;
		}
		if ((error = qtn_file_init_with_fd(s->qinfo, s->src_fd)) != 0)
		{
			qtn_file_free(s->qinfo);
			s->qinfo = NULL;
			rv = -1;
			goto done;
		}
	}
done:
	return rv;
}

static int
add_uberace(acl_t *acl)
{
	acl_entry_t entry;
	acl_permset_t permset;
	uuid_t qual;

	if (mbr_uid_to_uuid(getuid(), qual) != 0)
		goto error_exit;

	/*
	 * First, we create an entry, and give it the special name
	 * of ACL_FIRST_ENTRY, thus guaranteeing it will be first.
	 * After that, we clear out all the permissions in it, and
	 * add three permissions:  WRITE_DATA, WRITE_ATTRIBUTES, and
	 * WRITE_EXTATTRIBUTES.  We put these into an ACE that allows
	 * the functionality, and put this into the ACL.
	 */
	if (acl_create_entry_np(acl, &entry, ACL_FIRST_ENTRY) == -1)
		goto error_exit;
	if (acl_get_permset(entry, &permset) == -1) {
		copyfile_warn("acl_get_permset");
		goto error_exit;
	}
	if (acl_clear_perms(permset) == -1) {
		copyfile_warn("acl_clear_permset");
		goto error_exit;
	}
	if (acl_add_perm(permset, ACL_WRITE_DATA) == -1) {
		copyfile_warn("add ACL_WRITE_DATA");
		goto error_exit;
	}
	if (acl_add_perm(permset, ACL_WRITE_ATTRIBUTES) == -1) {
		copyfile_warn("add ACL_WRITE_ATTRIBUTES");
		goto error_exit;
	}
	if (acl_add_perm(permset, ACL_WRITE_EXTATTRIBUTES) == -1) {
		copyfile_warn("add ACL_WRITE_EXTATTRIBUTES");
		goto error_exit;
	}
	if (acl_add_perm(permset, ACL_APPEND_DATA) == -1) {
		copyfile_warn("add ACL_APPEND_DATA");
		goto error_exit;
	}
	if (acl_add_perm(permset, ACL_WRITE_SECURITY) == -1) {
		copyfile_warn("add ACL_WRITE_SECURITY");
		goto error_exit;
	}
	if (acl_set_tag_type(entry, ACL_EXTENDED_ALLOW) == -1) {
		copyfile_warn("set ACL_EXTENDED_ALLOW");
		goto error_exit;
	}

	if(acl_set_permset(entry, permset) == -1) {
		copyfile_warn("acl_set_permset");
		goto error_exit;
	}
	if(acl_set_qualifier(entry, qual) == -1) {
		copyfile_warn("acl_set_qualifier");
		goto error_exit;
	}

	return 0;
error_exit:
	return -1;
}

static int
is_uberace(acl_entry_t ace)
{
	int retval = 0;
	acl_permset_t perms, tperms;
	acl_t tacl;
	acl_entry_t tentry;
	acl_tag_t tag;
	guid_t *qual = NULL;
	uuid_t myuuid;

	// Who am I, and who is the ACE for?
	mbr_uid_to_uuid(geteuid(), myuuid);
	qual = (guid_t*)acl_get_qualifier(ace);

	// Need to create a temporary acl, so I can get the uberace template.
	tacl = acl_init(1);
	if (tacl == NULL) {
		goto done;
	}
	add_uberace(&tacl);
	if (acl_get_entry(tacl, ACL_FIRST_ENTRY, &tentry) != 0) {
		goto done;
	}
	acl_get_permset(tentry, &tperms);

	// Now I need to get
	acl_get_tag_type(ace, &tag);
	acl_get_permset(ace, &perms);

	if (tag == ACL_EXTENDED_ALLOW &&
		(memcmp(qual, myuuid, sizeof(myuuid)) == 0) &&
		acl_compare_permset_np(tperms, perms))
		retval = 1;

done:

	if (qual)
		acl_free(qual);

	if (tacl)
		acl_free(tacl);

	return retval;
}

static void
remove_uberace(int fd, struct stat *sbuf)
{
	filesec_t fsec = NULL;
	acl_t acl = NULL;
	acl_entry_t entry;
	struct stat sb;

	fsec = filesec_init();
	if (fsec == NULL) {
		goto noacl;
	}

	if (fstatx_np(fd, &sb, fsec) != 0) {
		if (errno == ENOTSUP)
			goto noacl;
		goto done;
	}

	if (filesec_get_property(fsec, FILESEC_ACL, &acl) != 0) {
		goto done;
	}

	if (acl_get_entry(acl, ACL_FIRST_ENTRY, &entry) == 0) {
		if (is_uberace(entry))
		{
			mode_t m = sbuf->st_mode & ~S_IFMT;

			if (acl_delete_entry(acl, entry) != 0 ||
				filesec_set_property(fsec, FILESEC_ACL, &acl) != 0 ||
				filesec_set_property(fsec, FILESEC_MODE, &m) != 0 ||
				fchmodx_np(fd, fsec) != 0)
				goto noacl;
		}
	}

done:
	if (acl)
		acl_free(acl);
	if (fsec)
		filesec_free(fsec);
	return;

noacl:
	fchmod(fd, sbuf->st_mode & ~S_IFMT);
	goto done;
}

static void
reset_security(copyfile_state_t s)
{
	/* If we haven't reset the file security information
	 * (COPYFILE_SECURITY is not set in flags)
	 * restore back the permissions the file had originally
	 *
	 * One of the reasons this seems so complicated is that
	 * it is partially at odds with copyfile_security().
	 *
	 * Simplisticly, we are simply trying to make sure we
	 * only copy what was requested, and that we don't stomp
	 * on what wasn't requested.
	 */

#ifdef COPYFILE_RECURSIVE
	if (s->dst_fd > -1) {
		struct stat sbuf;

		if (s->src_fd > -1 && (s->flags & COPYFILE_STAT))
			fstat(s->src_fd, &sbuf);
		else
			fstat(s->dst_fd, &sbuf);

		if (!(s->internal_flags & cfDelayAce))
			remove_uberace(s->dst_fd, &sbuf);
	}
#else
	if (s->permissive_fsec && (s->flags & COPYFILE_SECURITY) != COPYFILE_SECURITY) {
		if (s->flags & COPYFILE_ACL) {
			/* Just need to reset the BSD information -- mode, owner, group */
			(void)fchown(s->dst_fd, s->dst_sb.st_uid, s->dst_sb.st_gid);
			(void)fchmod(s->dst_fd, s->dst_sb.st_mode);
		} else {
			/*
			 * flags is either COPYFILE_STAT, or neither; if it's
			 * neither, then we restore both ACL and POSIX permissions;
			 * if it's STAT, however, then we only want to restore the
			 * ACL (which may be empty).  We do that by removing the
			 * POSIX information from the filesec object.
			 */
			if (s->flags & COPYFILE_STAT) {
				copyfile_unset_posix_fsec(s->original_fsec);
			}
			if (fchmodx_np(s->dst_fd, s->original_fsec) < 0 && errno != ENOTSUP)
				copyfile_warn("restoring security information");
		}
	}

	if (s->permissive_fsec) {
		filesec_free(s->permissive_fsec);
		s->permissive_fsec = NULL;
	}

	if (s->original_fsec) {
		filesec_free(s->original_fsec);
		s->original_fsec = NULL;
	}
#endif

	return;
}

/*
 * copytree -- recursively copy a hierarchy.
 *
 * Unlike normal copyfile(), copytree() can copy an entire hierarchy.
 * Care is taken to keep the ACLs set up correctly, in addition to the
 * normal copying that is done.  (When copying a hierarchy, we can't
 * get rid of the "allow-all-writes" ACE on a directory until we're done
 * copying the *contents* of the directory.)
 *
 * The other big difference from copyfile (for the moment) is that copytree()
 * will use a call-back function to pass along information about what is
 * about to be copied, and whether or not it succeeded.
 *
 * copytree() is called from copyfile() -- but copytree() itself then calls
 * copyfile() to copy each individual object.
 *
 * If COPYFILE_CLONE is passed, copytree() will clone (instead of copy)
 * regular files and symbolic links found in each directory.
 * Directories will still be copied normally.
 *
 * XXX - no effort is made to handle overlapping hierarchies at the moment.
 *
 */

static int
copytree(copyfile_state_t s)
{
	char *slash;
	int retval = 0;
	int (*sfunc)(const char *, struct stat *);
	copyfile_callback_t status = NULL;
	char srcisdir = 0, dstisdir = 0, dstexists = 0;
	struct stat sbuf;
	char *src, *dst;
	const char *dstpathsep = "";
#ifdef NOTYET
	char srcpath[PATH_MAX * 2 + 1], dstpath[PATH_MAX * 2 + 1];
#endif
	char *srcroot;
	FTS *fts = NULL;
	FTSENT *ftsent;
	ssize_t offset = 0;
	const char *paths[2] =  { 0 };
	unsigned int flags = 0;
	int fts_flags = FTS_NOCHDIR;
	dev_t last_dev = s->sb.st_dev;

	if (s == NULL) {
		errno = EINVAL;
		retval = -1;
		goto done;
	}
	if (s->flags & (COPYFILE_MOVE | COPYFILE_UNLINK | COPYFILE_CHECK | COPYFILE_PACK | COPYFILE_UNPACK | COPYFILE_CLONE_FORCE)) {
		errno = EINVAL;
		retval = -1;
		goto done;
	}

	flags = s->flags & (COPYFILE_ALL | COPYFILE_NOFOLLOW | COPYFILE_VERBOSE | COPYFILE_EXCL | COPYFILE_CLONE | COPYFILE_DATA_SPARSE);

	paths[0] = src = s->src;
	dst = s->dst;

	if (src == NULL || dst == NULL) {
		errno = EINVAL;
		retval = -1;
		goto done;
	}

	sfunc = (flags & COPYFILE_NOFOLLOW_SRC) ? lstat : stat;
	if ((sfunc)(src, &sbuf) == -1) {
		retval = -1;
		goto done;
	}
	if ((sbuf.st_mode & S_IFMT) == S_IFDIR) {
		srcisdir = 1;
	}

	sfunc = (flags & COPYFILE_NOFOLLOW_DST) ? lstat : stat;
	if ((sfunc)(dst, &sbuf) == -1) {
		if (errno != ENOENT) {
			retval = -1;
			goto done;
		}
	} else {
		dstexists = 1;
		if ((sbuf.st_mode & S_IFMT) == S_IFDIR) {
			dstisdir = 1;
		}
	}

#ifdef NOTYET
	// This doesn't handle filesystem crossing and case sensitivity
	// So there's got to be a better way

	if (realpath(src, srcpath) == NULL) {
		retval = -1;
		goto done;
	}

	if (realpath(dst, dstpath) == NULL &&
		(errno == ENOENT && realpath(dirname(dst), dstpath) == NULL)) {
		retval = -1;
		goto done;
	}
	if (strstr(srcpath, dstpath) != NULL) {
		errno = EINVAL;
		retval = -1;
		goto done;
	}
#endif
	srcroot = basename((char*)src);
	if (srcroot == NULL) {
		retval = -1;
		goto done;
	}

	/*
	 * To work on as well:
	 * We have a few cases when copying a hierarchy:
	 * 1)  src is a non-directory, dst is a directory;
	 * 2)  src is a non-directory, dst is a non-directory;
	 * 3)  src is a non-directory, dst does not exist;
	 * 4)  src is a directory, dst is a directory;
	 * 5)  src is a directory, dst is a non-directory;
	 * 6)  src is a directory, dst does not exist
	 *
	 * (1) copies src to dst/basename(src).
	 * (2) fails if COPYFILE_EXCL is set, otherwise copies src to dst.
	 * (3) and (6) copy src to the name dst.
	 * (4) copies the contents of src to the contents of dst.
	 * (5) is an error.
	 */

	if (dstisdir) {
		// copy /path/to/src to /path/to/dst/src
		// Append "/" and (fts_path - strlen(basename(src))) to dst?
		dstpathsep = "/";
		slash = strrchr(src, '/');
		if (slash == NULL)
			offset = 0;
		else
			offset = slash - src + 1;
	} else {
		// copy /path/to/src to /path/to/dst
		// append (fts_path + strlen(src)) to dst?
		dstpathsep = "";
		offset = strlen(src);
	}

	// COPYFILE_RECURSIVE is always done physically: see 11717978.
	fts_flags |= FTS_PHYSICAL;
	if (!(s->flags & (COPYFILE_NOFOLLOW_SRC|COPYFILE_CLONE))) {
		// Follow 'src', even if it's a symlink, unless instructed not to
		// or we're cloning, where we never follow symlinks.
		fts_flags |= FTS_COMFOLLOW;
	}

	fts = fts_open((char * const *)paths, fts_flags, NULL);

	status = s->statuscb;
	while ((ftsent = fts_read(fts)) != NULL) {
		int rv = 0;
		char *dstfile = NULL;
		int cmd = 0;
		copyfile_state_t tstate = copyfile_state_alloc();
		if (tstate == NULL) {
			errno = ENOMEM;
			retval = -1;
			break;
		}
		tstate->statuscb = s->statuscb;
		tstate->ctx = s->ctx;
		if (last_dev == ftsent->fts_dev) {
			tstate->internal_flags |= (s->internal_flags & COPYFILE_MNT_CPROTECT_MASK);
		} else {
			last_dev = ftsent->fts_dev;
		}
		asprintf(&dstfile, "%s%s%s", dst, dstpathsep, ftsent->fts_path + offset);
		if (dstfile == NULL) {
			copyfile_state_free(tstate);
			errno = ENOMEM;
			retval = -1;
			break;
		}
		switch (ftsent->fts_info) {
			case FTS_D:
				tstate->internal_flags |= cfDelayAce;
				cmd = COPYFILE_RECURSE_DIR;
				break;
			case FTS_SL:
			case FTS_SLNONE:
			case FTS_DEFAULT:
			case FTS_F:
				cmd = COPYFILE_RECURSE_FILE;
				break;
			case FTS_DP:
				cmd = COPYFILE_RECURSE_DIR_CLEANUP;
				break;
			case FTS_DNR:
			case FTS_ERR:
			case FTS_NS:
			case FTS_NSOK:
			default:
				errno = ftsent->fts_errno;
				if (status) {
					rv = (*status)(COPYFILE_RECURSE_ERROR, COPYFILE_ERR, tstate, ftsent->fts_path, dstfile, s->ctx);
					if (rv == COPYFILE_SKIP || rv == COPYFILE_CONTINUE) {
						errno = 0;
						goto skipit;
					}
					if (rv == COPYFILE_QUIT) {
						retval = -1;
						goto stopit;
					}
				} else {
					retval = -1;
					goto stopit;
				}
			case FTS_DOT:
				goto skipit;

		}

		if (cmd == COPYFILE_RECURSE_DIR || cmd == COPYFILE_RECURSE_FILE) {
			if (status) {
				rv = (*status)(cmd, COPYFILE_START, tstate, ftsent->fts_path, dstfile, s->ctx);
				if (rv == COPYFILE_SKIP) {
					if (cmd == COPYFILE_RECURSE_DIR) {
						rv = fts_set(fts, ftsent, FTS_SKIP);
						if (rv == -1) {
							rv = (*status)(0, COPYFILE_ERR, tstate, ftsent->fts_path, dstfile, s->ctx);
							if (rv == COPYFILE_QUIT)
								retval = -1;
						}
					}
					goto skipit;
				}
				if (rv == COPYFILE_QUIT) {
					retval = -1; errno = 0;
					goto stopit;
				}
			}
			// Since we don't support cloning directories this code depends on copyfile()
			// falling back to a regular directory copy.
			int tmp_flags = (cmd == COPYFILE_RECURSE_DIR) ? (flags & ~COPYFILE_STAT) : flags;
			rv = copyfile(ftsent->fts_path, dstfile, tstate, tmp_flags);
			if (rv < 0) {
				if (status) {
					rv = (*status)(cmd, COPYFILE_ERR, tstate, ftsent->fts_path, dstfile, s->ctx);
					if (rv == COPYFILE_QUIT) {
						retval = -1;
						goto stopit;
					} else
						rv = 0;
					goto skipit;
				} else {
					retval = -1;
					goto stopit;
				}
			}
			if (status) {
				rv = (*status)(cmd, COPYFILE_FINISH, tstate, ftsent->fts_path, dstfile, s->ctx);
				if (rv == COPYFILE_QUIT) {
					retval = -1; errno = 0;
					goto stopit;
				}
			}
		} else if (cmd == COPYFILE_RECURSE_DIR_CLEANUP) {
			if (status) {
				rv = (*status)(cmd, COPYFILE_START, tstate, ftsent->fts_path, dstfile, s->ctx);
				if (rv == COPYFILE_QUIT) {
					retval = -1; errno = 0;
					goto stopit;
				} else if (rv == COPYFILE_SKIP) {
					rv = 0;
					goto skipit;
				}
			}
			rv = copyfile(ftsent->fts_path, dstfile, tstate, (flags & COPYFILE_NOFOLLOW) | COPYFILE_STAT);
			if (rv < 0) {
				if (status) {
					rv = (*status)(COPYFILE_RECURSE_DIR_CLEANUP, COPYFILE_ERR, tstate, ftsent->fts_path, dstfile, s->ctx);
					if (rv == COPYFILE_QUIT) {
						retval = -1;
						goto stopit;
					} else if (rv == COPYFILE_SKIP || rv == COPYFILE_CONTINUE) {
						if (rv == COPYFILE_CONTINUE)
							errno = 0;
						retval = 0;
						goto skipit;
					}
				} else {
					retval = -1;
					goto stopit;
				}
			} else {
				if (status) {
					rv = (*status)(COPYFILE_RECURSE_DIR_CLEANUP, COPYFILE_FINISH, tstate, ftsent->fts_path, dstfile, s->ctx);
					if (rv == COPYFILE_QUIT) {
						retval = -1; errno = 0;
						goto stopit;
					}
				}
			}

			rv = 0;
		}
	skipit:
	stopit:
		s->internal_flags &= ~COPYFILE_MNT_CPROTECT_MASK;
		s->internal_flags |= (tstate->internal_flags & COPYFILE_MNT_CPROTECT_MASK);

		copyfile_state_free(tstate);
		free(dstfile);
		if (retval == -1)
			break;
	}

done:
	if (fts)
		fts_close(fts);

	copyfile_debug(1, "returning: %d errno %d\n", retval, errno);
	return retval;
}

/*
 * fcopyfile() is used to copy a source file descriptor to a destination file
 * descriptor.  This allows an application to figure out how it wants to open
 * the files (doing various security checks, perhaps), and then just pass in
 * the file descriptors.
 */
int fcopyfile(int src_fd, int dst_fd, copyfile_state_t state, copyfile_flags_t flags)
{
	int ret = 0;
	copyfile_state_t s = state;
	struct stat dst_sb;

	if (src_fd < 0 || dst_fd < 0)
	{
		errno = EINVAL;
		return -1;
	}

	if (copyfile_preamble(&s, flags) < 0)
		return -1;

	copyfile_debug(2, "set src_fd <- %d", src_fd);
	if (s->src_fd == -2 && src_fd > -1)
	{
		s->src_fd = src_fd;
		if (fstatx_np(s->src_fd, &s->sb, s->fsec) != 0)
		{
			if (errno == ENOTSUP || errno == EPERM)
				fstat(s->src_fd, &s->sb);
			else
			{
				copyfile_warn("fstatx_np on src fd %d", s->src_fd);
				return -1;
			}
		}
	}

	/* prevent copying on unsupported types */
	switch (s->sb.st_mode & S_IFMT)
	{
		case S_IFLNK:
		case S_IFDIR:
		case S_IFREG:
			break;
		default:
			errno = ENOTSUP;
			return -1;
	}

	copyfile_debug(2, "set dst_fd <- %d", dst_fd);
	if (s->dst_fd == -2 && dst_fd > -1)
		s->dst_fd = dst_fd;

	(void)fstat(s->dst_fd, &dst_sb);
	(void)fchmod(s->dst_fd, (dst_sb.st_mode & ~S_IFMT) | (S_IRUSR | S_IWUSR));

	(void)copyfile_quarantine(s);

	ret = copyfile_internal(s, flags);

	if (ret >= 0 && !(s->flags & COPYFILE_STAT))
	{
		(void)fchmod(s->dst_fd, dst_sb.st_mode & ~S_IFMT);
	}

	if (s->err) {
		errno = s->err;
		s->err = 0;
	}
	if (state == NULL) {
		int t = errno;
		copyfile_state_free(s);
		errno = t;
	}
	if (ret >= 0) {
		errno = 0;
	}

	return ret;
}

/*
 * This routine implements the clonefileat functionality
 * for copyfile. There are 2 kinds of clone flags, namely
 * 1. COPYFILE_CLONE_FORCE which is a 'force' clone flag.
 * 2. COPYFILE_CLONE which is a 'best try' flag.
 * In both cases, we inherit the flags provided
 * to copyfile call and clone the file.
 * Both these flags are equivalent to
 * (COPYFILE_EXCL | COPYFILE_ACL | COPYFILE_STAT | COPYFILE_XATTR | COPYFILE_DATA)
 * With force clone flag set, we return failure if cloning fails,
 * however, in case of best try flag, we fallback to the copy method.
 */

static int copyfile_clone(copyfile_state_t state)
{
	int ret = 0;
	// Since we don't allow cloning of directories, we must also forbid
	// cloning the target of symlinks (since that may be a directory).
	int cloneFlags = CLONE_NOFOLLOW;
	struct stat src_sb;

	if (lstat(state->src, &src_sb) != 0)
	{
		errno = EINVAL;
		return -1;
	}

	/*
	 * Support only for files and symbolic links.
	 * TODO:Remove this check when support for directories is added.
	 */
	if (S_ISREG(src_sb.st_mode) || S_ISLNK(src_sb.st_mode))
	{
		/*
		 * COPYFILE_UNLINK tells us to try removing the destination
		 * before we create it.  We don't care if the file doesn't
		 * exist, so we ignore ENOENT.
		 */
		if (state->flags & COPYFILE_UNLINK)
		{
			if (remove(state->dst) < 0 && errno != ENOENT)
			{
				return -1;
			}
		}
		ret = clonefileat(AT_FDCWD, state->src, AT_FDCWD, state->dst, cloneFlags);
		if (ret == 0) {
			/*
			 * We could also report the size of the single
			 * object that was cloned.  However, that's a lot
			 * more difficult when we eventually support
			 * cloning directories.  It seems reasonable to NOT
			 * report any bytes being "copied" in this scenario,
			 * and let the caller figure out how they want to
			 * deal.
			 */
			state->was_cloned = true;

			/*
			 * COPYFILE_MOVE tells us to attempt removing
			 * the source file after the copy, and to
			 * ignore any errors returned by remove(3).
			 */
			if (state->flags & COPYFILE_MOVE) {
				(void)remove(state->src);
			}
		}
	}
	else
	{
		errno = EINVAL;
		ret = -1;
	}
	return ret;
}

/*
 * the original copyfile() routine; this copies a source file to a destination
 * file.  Note that because we need to set the names in the state variable, this
 * is not just the same as opening the two files, and then calling fcopyfile().
 * Oh, if only life were that simple!
 */
int copyfile(const char *src, const char *dst, copyfile_state_t state, copyfile_flags_t flags)
{
	int ret = 0;
	int createdst = 0;
	copyfile_state_t s = state;
	struct stat dst_sb;

	if (src == NULL && dst == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	if (copyfile_preamble(&s, flags) < 0)
	{
		return -1;
	}

	/*
	 * This macro is... well, it's not the worst thing you can do with cpp, not
	 *  by a long shot.  Essentially, we are setting the filename (src or dst)
	 * in the state structure; since the structure may not have been cleared out
	 * before being used again, we do some of the cleanup here:  if the given
	 * filename (e.g., src) is set, and state->src is not equal to that, then
	 * we need to check to see if the file descriptor had been opened, and if so,
	 * close it.  After that, we set state->src to be a copy of the given filename,
	 * releasing the old copy if necessary.
	 */
#define COPYFILE_SET_FNAME(NAME, S) \
	do { \
		if (NAME != NULL) {									\
			if (S->NAME != NULL && strncmp(NAME, S->NAME, MAXPATHLEN)) {			\
				copyfile_debug(2, "replacing string %s (%s) -> (%s)", #NAME, NAME, S->NAME);\
				if (S->NAME##_fd != -2 && S->NAME##_fd > -1) {				\
					copyfile_debug(4, "closing %s fd: %d", #NAME, S->NAME##_fd);		\
					close(S->NAME##_fd);							\
					S->NAME##_fd = -2;							\
				}										\
			}										\
			if (S->NAME) {									\
				free(S->NAME);								\
				S->NAME = NULL;								\
			}										\
			if ((NAME) && (S->NAME = strdup(NAME)) == NULL)					\
				return -1;									\
		}											\
	} while (0)

	COPYFILE_SET_FNAME(src, s);
	COPYFILE_SET_FNAME(dst, s);

	if (s->flags & COPYFILE_RECURSIVE) {
		ret = copytree(s);
		goto exit;
	}

	if (s->flags & (COPYFILE_CLONE_FORCE | COPYFILE_CLONE))
	{
		ret = copyfile_clone(s);
		if (ret == 0) {
			goto exit;
		} else if (s->flags & COPYFILE_CLONE_FORCE) {
			goto error_exit;
		}
		// cloning failed. Inherit clonefile flags required for
		// falling back to copyfile.
		s->flags |= (COPYFILE_ACL | COPYFILE_EXCL | COPYFILE_NOFOLLOW_SRC |
					 COPYFILE_STAT | COPYFILE_XATTR | COPYFILE_DATA);

		s->flags &= ~COPYFILE_CLONE;
		flags = s->flags;
		ret = 0;
	}

	/*
	 * Get a copy of the source file's security settings
	 */
	if (s->original_fsec) {
		filesec_free(s->original_fsec);
		s->original_fsec = NULL;
	}
	if ((s->original_fsec = filesec_init()) == NULL)
		goto error_exit;

	if ((s->flags & COPYFILE_NOFOLLOW_DST) && lstat(s->dst, &dst_sb) == 0 &&
		((dst_sb.st_mode & S_IFMT) == S_IFLNK)) {
		if (s->permissive_fsec)
			free(s->permissive_fsec);
		s->permissive_fsec = NULL;
	} else if(statx_np(s->dst, &dst_sb, s->original_fsec) == 0)
	{
		/*
		 * copyfile_fix_perms() will make a copy of the permission set,
		 * and insert at the beginning an ACE that ensures we can write
		 * to the file and set attributes.
		 */

		if((s->permissive_fsec = copyfile_fix_perms(s, &s->original_fsec)) != NULL)
		{
			/*
			 * Set the permissions for the destination to our copy.
			 * We should get ENOTSUP from any filesystem that simply
			 * doesn't support it.
			 */
			if (chmodx_np(s->dst, s->permissive_fsec) < 0 && errno != ENOTSUP)
			{
				copyfile_warn("setting security information");
				filesec_free(s->permissive_fsec);
				s->permissive_fsec = NULL;
			}
		}
	} else if (errno == ENOENT) {
		createdst = 1;
	}

	/*
	 * If COPYFILE_CHECK is set in flags, then all we are going to do
	 * is see what kinds of things WOULD have been copied (see
	 * copyfile_check() below).  We return that value.
	 */
	if (COPYFILE_CHECK & flags)
	{
		ret = copyfile_check(s);
		goto exit;
	} else if ((ret = copyfile_open(s)) < 0)
		goto error_exit;

	(void)fcntl(s->src_fd, F_NOCACHE, 1);
	(void)fcntl(s->dst_fd, F_NOCACHE, 1);
#ifdef F_SINGLE_WRITER
	(void)fcntl(s->dst_fd, F_SINGLE_WRITER, 1);
#endif

	ret = copyfile_internal(s, flags);
	if (ret == -1)
		goto error_exit;

#ifdef COPYFILE_RECURSIVE
	if (!(flags & COPYFILE_STAT)) {
		if (!createdst)
		{
			/* Just need to reset the BSD information -- mode, owner, group */
			(void)fchown(s->dst_fd, dst_sb.st_uid, dst_sb.st_gid);
			(void)fchmod(s->dst_fd, dst_sb.st_mode);
		}
	}
#endif

	reset_security(s);

	if (s->src && (flags & COPYFILE_MOVE))
		(void)remove(s->src);

exit:
	if (ret >= 0) {
		errno = 0;
	}
	copyfile_debug(5, "returning %d errno %d\n", ret, errno);

	if (state == NULL) {
		int t = errno;
		copyfile_state_free(s);
		errno = t;
	}
	return ret;

error_exit:
	ret = -1;
	if (s && s->err) {
		errno = s->err;
		s->err = 0;
	}
	goto exit;
}

/*
 * Shared prelude to the {f,}copyfile().  This initializes the
 * state variable, if necessary, and also checks for both debugging
 * and disabling environment variables.
 */
static int copyfile_preamble(copyfile_state_t *state, copyfile_flags_t flags)
{
	copyfile_state_t s;

	if (*state == NULL)
	{
		if ((*state = copyfile_state_alloc()) == NULL)
			return -1;
	}

	s = *state;

	if (COPYFILE_DEBUG & flags)
	{
		char *e;
		if ((e = getenv(COPYFILE_DEBUG_VAR)))
		{
			errno = 0;
			s->debug = (uint32_t)strtol(e, NULL, 0);

			/* clamp s->debug to 1 if the environment variable is not parsable */
			if (s->debug == 0 && errno != 0)
				s->debug = 1;
		}
		copyfile_debug(2, "debug value set to: %d", s->debug);
	}

#if 0
	/* Temporarily disabled */
	if (getenv(COPYFILE_DISABLE_VAR) != NULL)
	{
		copyfile_debug(1, "copyfile disabled");
		return 2;
	}
#endif
	copyfile_debug(2, "setting flags: %d", s->flags);
	s->flags = flags;

	return 0;
}

/*
 * The guts of {f,}copyfile().
 * This looks through the flags in a particular order, and calls the
 * associated functions.
 */
static int copyfile_internal(copyfile_state_t s, copyfile_flags_t flags)
{
	int ret = 0;

	if (s->dst_fd < 0 || s->src_fd < 0)
	{
		copyfile_debug(1, "file descriptors not open (src: %d, dst: %d)", s->src_fd, s->dst_fd);
		s->err = EINVAL;
		return -1;
	}

	/*
	 * COPYFILE_PACK causes us to create an Apple Double version of the
	 * source file, and puts it into the destination file.  See
	 * copyfile_pack() below for all the gory details.
	 */
	if (COPYFILE_PACK & flags)
	{
		if ((ret = copyfile_pack(s)) < 0)
		{
			if (s->dst) unlink(s->dst);
			goto exit;
		}
		goto exit;
	}

	/*
	 * COPYFILE_UNPACK is the undoing of COPYFILE_PACK, obviously.
	 * The goal there is to take an Apple Double file, and turn it
	 * into a normal file (with data fork, resource fork, modes,
	 * extended attributes, ACLs, etc.).
	 */
	if (COPYFILE_UNPACK & flags)
	{
		if ((ret = copyfile_unpack(s)) < 0)
			goto error_exit;
		goto exit;
	}



	/*
	 * If we have quarantine info set, we attempt
	 * to apply it to dst_fd.  We don't care if
	 * it fails, not yet anyway.
	 */
	if (s->qinfo)
	{
		int qr;
		uint32_t q_flags;

		/*
		 * If COPYFILE_RUN_IN_PLACE is set, we need to add
		 * QTN_FLAG_DO_NOT_TRANSLOCATE to the qinfo flags.
		 *
		 * On iOS, qtn_file_get_flags & qtn_file_set_flags
		 * don't modify anything, always return 0, per static
		 * defines at top of this file, though we should never
		 * get here in that case as qinfo will always be NULL.
		 */
		if (COPYFILE_RUN_IN_PLACE & flags)
		{
			q_flags = 0;

			q_flags = qtn_file_get_flags(s->qinfo);
			q_flags |= QTN_FLAG_DO_NOT_TRANSLOCATE;

			if (qtn_file_set_flags(s->qinfo, q_flags) != 0) {
				s->err = errno = EINVAL;
				goto error_exit;
			}
		}

		qr = qtn_file_apply_to_fd(s->qinfo, s->dst_fd);
		if (qr != 0) {
			if (s->statuscb) {
				int rv;

				s->xattr_name = (char*)XATTR_QUARANTINE_NAME;
				rv = (*s->statuscb)(COPYFILE_COPY_XATTR, COPYFILE_ERR, s, s->src, s->dst, s->ctx);
				s->xattr_name = NULL;
				if (rv == COPYFILE_QUIT) {
					s->err = errno = (qr < 0 ? ENOTSUP : qr);
					goto error_exit;
				}
			} else {
				s->err = errno = (qr < 0 ? ENOTSUP : qr);
				goto error_exit;
			}
		}
	}

	/*
	 * COPYFILE_XATTR tells us to copy the extended attributes;
	 * this is seperate from the extended security (aka ACLs),
	 * however.  If we succeed in this, we continue to the next
	 * stage; if we fail, we return with an error value.  Note
	 * that we fail if the errno is ENOTSUP, but we don't print
	 * a warning in that case.
	 */
	if (COPYFILE_XATTR & flags)
	{
		if ((ret = copyfile_xattr(s)) < 0)
		{
			if (errno != ENOTSUP && errno != EPERM)
				copyfile_warn("error processing extended attributes");
			goto exit;
		}
	}

	/*
	 * Similar to above, this tells us whether or not to copy
	 * the non-meta data portion of the file.  We attempt to
	 * remove (via unlink) the destination file if we fail.
	 */
	if ((COPYFILE_DATA|COPYFILE_DATA_SPARSE) & flags)
	{
		if ((ret = copyfile_data(s)) < 0)
		{
			copyfile_warn("error processing data");
			if (s->dst && unlink(s->dst))
				copyfile_warn("%s: remove", s->src ? s->src : "(null src)");
			goto exit;
		}
	}

	/*
	 * COPYFILE_SECURITY requests that we copy the security, both
	 * extended and mundane (that is, ACLs and POSIX).
	 */
	if (COPYFILE_SECURITY & flags)
	{
		if ((ret = copyfile_security(s)) < 0)
		{
			copyfile_warn("error processing security information");
			goto exit;
		}
	}

	if (COPYFILE_STAT & flags)
	{
		if ((ret = copyfile_stat(s)) < 0)
		{
			copyfile_warn("error processing POSIX information");
			goto exit;
		}
	}

exit:
	return ret;

error_exit:
	ret = -1;
	goto exit;
}

/*
 * A publicly-visible routine, copyfile_state_alloc() sets up the state variable.
 */
copyfile_state_t copyfile_state_alloc(void)
{
	copyfile_state_t s = (copyfile_state_t) calloc(1, sizeof(struct _copyfile_state));

	if (s != NULL)
	{
		s->src_fd = -2;
		s->dst_fd = -2;
		if (s->fsec) {
			filesec_free(s->fsec);
			s->fsec = NULL;
		}
		s->fsec = filesec_init();
	} else
		errno = ENOMEM;

	return s;
}

/*
 * copyfile_state_free() returns the memory allocated to the state structure.
 * It also closes the file descriptors, if they've been opened.
 */
int copyfile_state_free(copyfile_state_t s)
{
	if (s != NULL)
	{
		if (s->fsec)
			filesec_free(s->fsec);

		if (s->original_fsec)
			filesec_free(s->original_fsec);

		if (s->permissive_fsec)
			filesec_free(s->permissive_fsec);

		if (s->qinfo)
			qtn_file_free(s->qinfo);

		if (copyfile_close(s) < 0)
		{
			copyfile_warn("error closing files");
			return -1;
		}
		if (s->xattr_name)
			free(s->xattr_name);
		if (s->dst)
			free(s->dst);
		if (s->src)
			free(s->src);
		free(s);
	}
	return 0;
}

/*
 * Should we worry if we can't close the source?  NFS says we
 * should, but it's pretty late for us at this point.
 */
static int copyfile_close(copyfile_state_t s)
{
	if (s->src && s->src_fd >= 0)
		close(s->src_fd);

	if (s->dst && s->dst_fd >= 0) {
		if (close(s->dst_fd))
			return -1;
	}

	return 0;
}

/*
 * The purpose of this function is to set up a set of permissions
 * (ACL and traditional) that lets us write to the file.  In the
 * case of ACLs, we do this by putting in a first entry that lets
 * us write data, attributes, and extended attributes.  In the case
 * of traditional permissions, we set the S_IWUSR (user-write)
 * bit.
 */
static filesec_t copyfile_fix_perms(copyfile_state_t s __unused, filesec_t *fsec)
{
	filesec_t ret_fsec = NULL;
	mode_t mode;
	acl_t acl = NULL;

	if ((ret_fsec = filesec_dup(*fsec)) == NULL)
		goto error_exit;

	if (filesec_get_property(ret_fsec, FILESEC_ACL, &acl) == 0)
	{
#ifdef COPYFILE_RECURSIVE
		if (add_uberace(&acl))
			goto error_exit;
#else
		acl_entry_t entry;
		acl_permset_t permset;
		uuid_t qual;

		if (mbr_uid_to_uuid(getuid(), qual) != 0)
			goto error_exit;

		/*
		 * First, we create an entry, and give it the special name
		 * of ACL_FIRST_ENTRY, thus guaranteeing it will be first.
		 * After that, we clear out all the permissions in it, and
		 * add three permissions:  WRITE_DATA, WRITE_ATTRIBUTES, and
		 * WRITE_EXTATTRIBUTES.  We put these into an ACE that allows
		 * the functionality, and put this into the ACL.
		 */
		if (acl_create_entry_np(&acl, &entry, ACL_FIRST_ENTRY) == -1)
			goto error_exit;
		if (acl_get_permset(entry, &permset) == -1)
			goto error_exit;
		if (acl_clear_perms(permset) == -1)
			goto error_exit;
		if (acl_add_perm(permset, ACL_WRITE_DATA) == -1)
			goto error_exit;
		if (acl_add_perm(permset, ACL_WRITE_ATTRIBUTES) == -1)
			goto error_exit;
		if (acl_add_perm(permset, ACL_WRITE_EXTATTRIBUTES) == -1)
			goto error_exit;
		if (acl_set_tag_type(entry, ACL_EXTENDED_ALLOW) == -1)
			goto error_exit;

		if(acl_set_permset(entry, permset) == -1)
			goto error_exit;
		if(acl_set_qualifier(entry, qual) == -1)
			goto error_exit;
#endif

		if (filesec_set_property(ret_fsec, FILESEC_ACL, &acl) != 0)
			goto error_exit;
	}

	/*
	 * This is for the normal, mundane, POSIX permission model.
	 * We make sure that we can write to the file.
	 */
	if (filesec_get_property(ret_fsec, FILESEC_MODE, &mode) == 0)
	{
		if ((mode & (S_IWUSR | S_IRUSR)) != (S_IWUSR | S_IRUSR))
		{
			mode |= S_IWUSR|S_IRUSR;
			if (filesec_set_property(ret_fsec, FILESEC_MODE, &mode) != 0)
				goto error_exit;
		}
	}

exit:
	if (acl)
		acl_free(acl);

	return ret_fsec;

error_exit:
	if (ret_fsec)
	{
		filesec_free(ret_fsec);
		ret_fsec = NULL;
	}
	goto exit;
}

/*
 * Used to clear out the BSD/POSIX security information from
 * a filesec
 */
static int
copyfile_unset_posix_fsec(filesec_t fsec)
{
	(void)filesec_set_property(fsec, FILESEC_OWNER, _FILESEC_UNSET_PROPERTY);
	(void)filesec_set_property(fsec, FILESEC_GROUP, _FILESEC_UNSET_PROPERTY);
	(void)filesec_set_property(fsec, FILESEC_MODE, _FILESEC_UNSET_PROPERTY);
	return 0;
}

/*
 * Used to remove acl information from a filesec_t
 * Unsetting the acl alone in Tiger was insufficient
 */
static int copyfile_unset_acl(copyfile_state_t s)
{
	int ret = 0;
	if (filesec_set_property(s->fsec, FILESEC_ACL, NULL) == -1)
	{
		copyfile_debug(5, "unsetting acl attribute on %s", s->dst ? s->dst : "(null dst)");
		++ret;
	}
	if (filesec_set_property(s->fsec, FILESEC_UUID, NULL) == -1)
	{
		copyfile_debug(5, "unsetting uuid attribute on %s", s->dst ? s->dst : "(null dst)");
		++ret;
	}
	if (filesec_set_property(s->fsec, FILESEC_GRPUUID, NULL) == -1)
	{
		copyfile_debug(5, "unsetting group uuid attribute on %s", s->dst ? s->dst : "(null dst)");
		++ret;
	}
	return ret;
}

/*
 * copyfile_open() does what one expects:  it opens up the files
 * given in the state structure, if they're not already open.
 * It also does some type validation, to ensure that we only
 * handle file types we know about.
 */
static int copyfile_open(copyfile_state_t s)
{
	int oflags = O_EXCL | O_CREAT | O_WRONLY;
	int islnk = 0, isdir = 0, isreg = 0;
	int osrc = 0, dsrc = 0;
	int prot_class = PROTECTION_CLASS_DEFAULT;
	int set_cprot_explicit = 0;
	int error = 0;

	if (s->src && s->src_fd == -2)
	{
		if ((COPYFILE_NOFOLLOW_SRC & s->flags ? lstatx_np : statx_np)
			(s->src, &s->sb, s->fsec))
		{
			copyfile_warn("stat on %s", s->src);
			return -1;
		}

		/* prevent copying on unsupported types */
		switch (s->sb.st_mode & S_IFMT)
		{
			case S_IFLNK:
				islnk = 1;
				if ((size_t)s->sb.st_size > SIZE_T_MAX) {
					s->err = ENOMEM;	/* too big for us to copy */
					return -1;
				}
				osrc = O_SYMLINK;
				break;
			case S_IFDIR:
				isdir = 1;
				break;
			case S_IFREG:
				isreg = 1;
				break;
			default:
				if (!(strcmp(s->src, "/dev/null") == 0 && (s->flags & COPYFILE_METADATA))) {
					s->err = ENOTSUP;
					return -1;
				}
		}
		/*
		 * If we're packing, then we are actually
		 * creating a file, no matter what the source
		 * was.
		 */
		if (s->flags & COPYFILE_PACK) {
			/*
			 * O_SYMLINK and O_NOFOLLOW are not compatible options:
			 * if the file is a symlink, and O_NOFOLLOW is specified,
			 * open will return ELOOP, whether or not O_SYMLINK is set.
			 * However, we know whether or not it was a symlink from
			 * the stat above (although there is a potentiaal for a race
			 * condition here, but it will err on the side of returning
			 * ELOOP from open).
			 */
			if (!islnk)
				osrc = (s->flags & COPYFILE_NOFOLLOW_SRC) ? O_NOFOLLOW : 0;
			isdir = islnk = 0;
		}

		if ((s->src_fd = open(s->src, O_RDONLY | osrc , 0)) < 0)
		{
			copyfile_warn("open on %s", s->src);
			return -1;
		}
		copyfile_debug(2, "open successful on source (%s)", s->src);

		(void)copyfile_quarantine(s);
	}

	if (s->dst && s->dst_fd == -2)
	{
		/*
		 * COPYFILE_UNLINK tells us to try removing the destination
		 * before we create it.  We don't care if the file doesn't
		 * exist, so we ignore ENOENT.
		 */
		if (COPYFILE_UNLINK & s->flags)
		{
			if (remove(s->dst) < 0 && errno != ENOENT)
			{
				copyfile_warn("%s: remove", s->dst);
				return -1;
			}
		}

		if (s->flags & COPYFILE_NOFOLLOW_DST) {
			struct stat st;

			dsrc = O_NOFOLLOW;
			if (lstat(s->dst, &st) != -1) {
				if ((st.st_mode & S_IFMT) == S_IFLNK)
					dsrc = O_SYMLINK;
			}
		}

		if (!(s->internal_flags & cfSrcProtSupportValid))
		{
			if ((error = does_copy_protection(s->src_fd)) > 0)
			{
				s->internal_flags |= cfSrcSupportsCProtect;
			}
			else if (error < 0)
			{
				copyfile_warn("does_copy_protection failed on (%s) with error <%d>", s->src, errno);
				return -1;
			}
			s->internal_flags |= cfSrcProtSupportValid;
		}

		/* copy protection is only valid for regular files and directories. */
		if ((isreg || isdir) && (s->internal_flags & cfSrcSupportsCProtect))
		{
			prot_class = GET_PROT_CLASS(s->src_fd);
			if (prot_class < 0)
			{
				copyfile_warn("GET_PROT_CLASS failed on (%s) with error <%d>", s->src, errno);
				return -1;
			}
		}

		if (islnk) {
			size_t sz = (size_t)s->sb.st_size + 1;
			char *bp;

			bp = calloc(1, sz);
			if (bp == NULL) {
				copyfile_warn("cannot allocate %zd bytes", sz);
				return -1;
			}
			if (readlink(s->src, bp, sz-1) == -1) {
				copyfile_warn("cannot readlink %s", s->src);
				free(bp);
				return -1;
			}
			if (symlink(bp, s->dst) == -1) {
				if (errno != EEXIST || (s->flags & COPYFILE_EXCL)) {
					copyfile_warn("Cannot make symlink %s", s->dst);
					free(bp);
					return -1;
				}
			}
			free(bp);
			s->dst_fd = open(s->dst, O_RDONLY | O_SYMLINK);
			if (s->dst_fd == -1) {
				copyfile_warn("Cannot open symlink %s for reading", s->dst);
				return -1;
			}
		} else if (isdir) {
			mode_t mode;
			mode = (s->sb.st_mode & ~S_IFMT) | S_IRWXU;

			if (mkdir(s->dst, mode) == -1) {
				if (errno != EEXIST || (s->flags & COPYFILE_EXCL)) {
					copyfile_warn("Cannot make directory %s", s->dst);
					return -1;
				}
			}
			s->dst_fd = open(s->dst, O_RDONLY | dsrc);
			if (s->dst_fd == -1) {
				copyfile_warn("Cannot open directory %s for reading", s->dst);
				return -1;
			}
			set_cprot_explicit = 1;
		} else while((s->dst_fd = open_dprotected_np(s->dst, oflags | dsrc, prot_class, 0, s->sb.st_mode | S_IWUSR)) < 0)
		{
			/*
			 * We set S_IWUSR because fsetxattr does not -- at the time this comment
			 * was written -- allow one to set an extended attribute on a file descriptor
			 * for a read-only file, even if the file descriptor is opened for writing.
			 * This will only matter if the file does not already exist.
			 */
			switch(errno)
			{
				case EEXIST:
					copyfile_debug(3, "open failed, retrying (%s)", s->dst);
					if (s->flags & COPYFILE_EXCL)
						break;
					oflags = oflags & ~O_CREAT;
					/* if O_CREAT isn't set in open_dprotected_np, it won't set protection class.
					 * Set the flag here so we know to do it later.
					 */
					set_cprot_explicit = 1;
					if (s->flags & (COPYFILE_PACK | COPYFILE_DATA))
					{
						copyfile_debug(4, "truncating existing file (%s)", s->dst);
						oflags |= O_TRUNC;
					}
					continue;
				case EACCES:
					if(chmod(s->dst, (s->sb.st_mode | S_IWUSR) & ~S_IFMT) == 0)
						continue;
					else {
						/*
						 * If we're trying to write to a directory to which we don't
						 * have access, the create above would have failed, but chmod
						 * here would have given us ENOENT.  But the real error is
						 * still one of access, so we change the errno we're reporting.
						 * This could cause confusion with a race condition.
						 */

						if (errno == ENOENT)
							errno = EACCES;
						break;
					}
				case EISDIR:
					copyfile_debug(3, "open failed because it is a directory (%s)", s->dst);
					if (((s->flags & COPYFILE_EXCL) ||
						 (!isdir && (s->flags & COPYFILE_DATA)))
						&& !(s->flags & COPYFILE_UNPACK))
						break;
					oflags = (oflags & ~(O_WRONLY|O_CREAT|O_TRUNC)) | O_RDONLY;
					continue;
			}
			copyfile_warn("open on %s", s->dst);
			return -1;
		}
		copyfile_debug(2, "open successful on destination (%s)", s->dst);

		if (s->internal_flags & cfSrcSupportsCProtect)
		{
			if (!(s->internal_flags & cfDstProtSupportValid))
			{
				if ((error = does_copy_protection(s->dst_fd)) > 0)
				{
					s->internal_flags |= cfDstSupportsCProtect;
				}
				else if (error < 0)
				{
					copyfile_warn("does_copy_protection failed on (%s) with error <%d>", s->dst, errno);
					return -1;
				}
				s->internal_flags |= cfDstProtSupportValid;
			}

			if ((isreg || isdir)
				&& set_cprot_explicit
				&& (s->internal_flags & cfDstSupportsCProtect))
			{
				/* Protection class is set in open_dprotected_np for regular files that aren't truncated.
				 * We set the protection class here for truncated files and directories.
				 */
				if (SET_PROT_CLASS(s->dst_fd, prot_class) != 0)
				{
					copyfile_warn("SET_PROT_CLASS failed on (%s) with error <%d>", s->dst, errno);
					return -1;
				}
			}
		}
	}

	if (s->dst_fd < 0 || s->src_fd < 0)
	{
		copyfile_debug(1, "file descriptors not open (src: %d, dst: %d)",
					   s->src_fd, s->dst_fd);
		s->err = EINVAL;
		return -1;
	}
	return 0;
}


/*
 * copyfile_check(), as described above, essentially tells you
 * what you'd have to copy, if you wanted it to copy the things
 * you asked it to copy.
 * In other words, if you pass in COPYFILE_ALL, and the file in
 * question had no extended attributes but did have an ACL, you'd
 * get back COPYFILE_ACL.
 */
static copyfile_flags_t copyfile_check(copyfile_state_t s)
{
	acl_t acl = NULL;
	copyfile_flags_t ret = 0;
	int nofollow = (s->flags & COPYFILE_NOFOLLOW_SRC);
	qtn_file_t qinfo;

	if (!s->src)
	{
		s->err = EINVAL;
		return -1;
	}

	/* check EAs */
	if (COPYFILE_XATTR & s->flags)
		if (listxattr(s->src, 0, 0, nofollow ? XATTR_NOFOLLOW : 0) > 0)
		{
			ret |= COPYFILE_XATTR;
		}

	if (COPYFILE_ACL & s->flags)
	{
		(COPYFILE_NOFOLLOW_SRC & s->flags ? lstatx_np : statx_np)
		(s->src, &s->sb, s->fsec);

		if (filesec_get_property(s->fsec, FILESEC_ACL, &acl) == 0)
			ret |= COPYFILE_ACL;
	}

	copyfile_debug(2, "check result: %d (%s)", ret, s->src);

	if (acl)
		acl_free(acl);

	if (s->qinfo) {
		/* If the state has had quarantine info set already, we use that */
		ret |= ((s->flags & COPYFILE_XATTR) ? COPYFILE_XATTR : COPYFILE_ACL);
	} else {
		qinfo = qtn_file_alloc();
		/*
		 * For quarantine information, we need to see if the source file
		 * has any.  Since it may be a symlink, however, and we may, or
		 * not be following, *and* there's no qtn* routine which can optionally
		 * follow or not follow a symlink, we need to instead work around
		 * this limitation.
		 */
		if (qinfo) {
			int fd;
			int qret = 0;
			struct stat sbuf;

			/*
			 * If we care about not following symlinks, *and* the file exists
			 * (which is to say, lstat doesn't return an error), *and* the file
			 * is a symlink, then we open it up (with O_SYMLINK), and use
			 * qtn_file_init_with_fd(); if none of that is true, however, then
			 * we can simply use qtn_file_init_with_path().
			 */
			if (nofollow
				&& lstat(s->src, &sbuf) == 0
				&& ((sbuf.st_mode & S_IFMT) == S_IFLNK)) {
				fd = open(s->src, O_RDONLY | O_SYMLINK);
				if (fd != -1) {
					if (!qtn_file_init_with_fd(qinfo, fd)) {
						qret |= ((s->flags & COPYFILE_XATTR) ? COPYFILE_XATTR : COPYFILE_ACL);
					}
					close(fd);
				}
			} else {
				if (!qtn_file_init_with_path(qinfo, s->src)) {
					qret |= ((s->flags & COPYFILE_XATTR) ? COPYFILE_XATTR : COPYFILE_ACL);
				}
			}
			qtn_file_free(qinfo);
			ret |= qret;
		}
	}
	return ret;
}

/*
 * Attempt to copy the data section of a file sparsely.
 * Requires that the source and destination file systems support sparse files.
 * Also requires that the source file descriptor's offset is a multiple of the smaller of the
 * source and destination file systems' block size.
 * In practice, this means that we refuse to perform copies that are only partially sparse.
 * Returns 0 if the source sparse file was copied, -1 on an unrecoverable error that
 * callers should propagate, and ENOTSUP where this routine refuses to copy the source file.
 * In this final case, callers are free to attempt a full copy.
 */
static int copyfile_data_sparse(copyfile_state_t s, size_t input_blk_size, size_t output_blk_size)
{
	int src_fd = s->src_fd, dst_fd = s->dst_fd, rc = 0;
	off_t src_start, dst_start, src_size = s->sb.st_size;
	off_t first_hole_offset, next_hole_offset, current_src_offset, next_src_offset;
	ssize_t nread;
	size_t iosize = MIN(input_blk_size, output_blk_size);
	copyfile_callback_t status = s->statuscb;
	char *bp = NULL;
	bool use_punchhole = true;
	errno = 0;

	// Sanity checks.
	if (!(s->flags & COPYFILE_DATA_SPARSE)) {
		// Don't attempt this unless the right flags are passed.
		return ENOTSUP;
	} else if (src_size <= 0) {
		// The file size of our source is invalid; there's nothing to copy.
		errno = EINVAL;
		goto error_exit;
	}

	// Since a major underlying filesystem requires that holes are block-aligned,
	// we only punch holes if we can guarantee that all holes from the source can
	// be holes in the destination, which requires that the source filesystem's block size
	// be an integral multiple of the destination filesystem's block size.
	if (input_blk_size % output_blk_size != 0) {
		use_punchhole = false;
	}

	// Get the starting src/dest file descriptor offsets.
	src_start = lseek(src_fd, 0, SEEK_CUR);
	dst_start = lseek(dst_fd, 0, SEEK_CUR);
	if (src_start < 0 || src_start >= src_size || dst_start < 0) {
		/*
		 * Invalid starting source/destination offset:
		 * Either < 0 which is plainly invalid (lseek may have failed),
		 * or > EOF which means that the copy operation is undefined,
		 * as by definition there is no data past EOF.
		 */
		if (errno == 0) {
			errno = EINVAL;
		}
		copyfile_warn("Invalid file descriptor offset, cannot perform a sparse copy");
		goto error_exit;
	} else if (src_start != (off_t) roundup(src_start, iosize) ||
			   dst_start != (off_t) roundup(dst_start, iosize)) {
		// If the starting offset isn't a multiple of the iosize, we can't do an entire sparse copy.
		// Fall back to copyfile_data(), which will perform a full copy from the starting position.
		return ENOTSUP;
	}

	// Make sure that there is at least one hole in this [part of the] file.
	first_hole_offset = lseek(src_fd, src_start, SEEK_HOLE);
	if (first_hole_offset == -1 || first_hole_offset == src_size) {
		/*
		 * Either an error occurred, the src starting position is EOF, or there are no
		 * holes in this [portion of the] source file. Regardless, we rewind the source file
		 * and return ENOTSUP so copyfile_data() can attempt a full copy.
		 */
		if (lseek(src_fd, src_start, SEEK_SET) == -1) {
			goto error_exit;
		}
		return ENOTSUP;
	}

	// We are ready to begin copying.
	// First, truncate the destination file to zero out any existing contents.
	// Then, truncate it again to its eventual size.
	if (ftruncate(dst_fd, dst_start) == -1) {
		copyfile_warn("Could not zero destination file before copy");
		goto error_exit;
	} else if (ftruncate(dst_fd, dst_start + src_size - src_start) == -1) {
		copyfile_warn("Could not set destination file size before copy");
		goto error_exit;
	}

	// Set the source's offset to the first data section.
	current_src_offset = lseek(src_fd, src_start, SEEK_DATA);
	if (current_src_offset == -1) {
		if (errno == ENXIO) {
			// There are no more data sections in the file, so there's nothing to copy.
			goto set_total_copied;
		}
		goto error_exit;
	}

	// Now, current_src_offset points at the start of src's first data region.
	// Update dst_fd to point to the same offset (respecting its start).
	if (lseek(dst_fd, dst_start + current_src_offset - src_start, SEEK_SET) == -1) {
		copyfile_warn("failed to set dst to first data section");
		goto error_exit;
	}

	// Allocate a temporary buffer to copy data sections into.
	bp = malloc(iosize);
	if (bp == NULL) {
		copyfile_warn("No memory for copy buffer");
		goto error_exit;
	}

	/*
	 * Performing a sparse copy:
	 * While our source fd points to a data section (and is < EOF), read iosize bytes in.
	 * Then, write those bytes to the dest fd, using the same iosize.
	 * Finally, update our source and dest fds to point to the next data section.
	 */
	while ((nread = read(src_fd, bp, iosize)) > 0) {
		ssize_t nwritten;
		size_t left = nread;
		void *ptr = bp;
		int loop = 0;

		while (left > 0) {
			nwritten = write(dst_fd, ptr, left);
			switch (nwritten) {
				case 0:
					if (++loop > 5) {
						copyfile_warn("writing to output %d times resulted in 0 bytes written", loop);
						errno = EAGAIN;
						goto error_exit;
					}
					break;
				case -1:
					copyfile_warn("writing to output file failed");
					if (status) {
						int rv = (*status)(COPYFILE_COPY_DATA, COPYFILE_ERR, s, s->src, s->dst, s->ctx);
						if (rv == COPYFILE_SKIP) {	// Skip the data copy
							errno = 0;
							goto exit;
						} else if (rv == COPYFILE_CONTINUE) {	// Retry the write
							errno = 0;
							continue;
						}
					}
					// If we get here, we either have no callback or it didn't tell us to continue.
					goto error_exit;
					break;
				default:
					left -= nwritten;
					ptr = ((char*)ptr) + nwritten;
					loop = 0;
					break;
			}
			s->totalCopied += nwritten;
			if (status) {
				int rv = (*status)(COPYFILE_COPY_DATA, COPYFILE_PROGRESS, s, s->src, s->dst, s->ctx);
				if (rv == COPYFILE_QUIT) {
					errno = ECANCELED;
					goto error_exit;
				}
			}
		}
		current_src_offset += nread;

		// Find the next area of src_fd to copy.
		// Since data sections can be any length, we need see if current_src_offset points
		// at a hole.
		// If we get ENXIO, we're done copying (the last part of the file was a data section).
		// If this is not a hole, we do not need to alter current_src_offset yet.
		// If this is a hole, then we need to look for the next data section.
		next_hole_offset = lseek(src_fd, current_src_offset, SEEK_HOLE);
		if (next_hole_offset == -1) {
			if (errno == ENXIO) {
				break; // We're done copying data sections.
			}
			copyfile_warn("unable to find next hole in file during copy");
			goto error_exit;
		} else if (next_hole_offset != current_src_offset) {
			// Keep copying this data section (we must rewind src_fd to current_src_offset).
			if (lseek(src_fd, current_src_offset, SEEK_SET) == -1) {
				goto error_exit;
			}
			continue;
		}

		// If we get here, we need to find the next data section to copy.
		next_src_offset = lseek(src_fd, current_src_offset, SEEK_DATA);
		if (next_src_offset == -1) {
			if (errno == ENXIO) {
				// There are no more data sections in this file, so we're done with the copy.
				break;
			}

			copyfile_warn("unable to advance src to next data section");
			goto error_exit;
		}

		// Advance the dst_fd to match (taking into account where it started).
		if (lseek(dst_fd, dst_start + (next_src_offset - src_start), SEEK_SET) == -1) {
			copyfile_warn("unable to advance dst to next data section");
			goto error_exit;
		}

		current_src_offset = next_src_offset;
	}
	if (nread < 0) {
		copyfile_warn("error %d reading from %s", errno, s->src ? s->src : "(null src)");
		goto error_exit;
	}

	// Punch holes where possible if needed.
	if (use_punchhole) {
		struct fpunchhole punchhole_args;
		off_t hole_start = first_hole_offset, hole_end;
		bool trailing_hole = true;

		// First, reset the source and destination file descriptors.
		if (lseek(src_fd, src_start, SEEK_SET) == -1 || lseek(dst_fd, dst_start, SEEK_SET) == -1) {
			copyfile_warn("unable to reset file descriptors to punch holes");
			// We have still copied the data, so there's no need to return an error here.
			goto set_total_copied;
		}

		// Now, find holes in the source (first_hole_offset already points to a source hole),
		// determining their length by the presence of a data section.
		while ((hole_end = lseek(src_fd, hole_start + (off_t) iosize, SEEK_DATA)) != -1) {
			memset(&punchhole_args, 0, sizeof(punchhole_args));

			// Fix up the offset and length for the destination file.
			punchhole_args.fp_offset = hole_start - src_start + dst_start;
			punchhole_args.fp_length = hole_end - hole_start;
			if (fcntl(dst_fd, F_PUNCHHOLE, &punchhole_args) == -1) {
				copyfile_warn("unable to punch hole in destination file, offset %lld length %lld",
							  hole_start - src_start + dst_start, hole_end - hole_start);
				goto set_total_copied;
			}

			// Now, find the start of the next hole.
			hole_start = lseek(src_fd, hole_end, SEEK_HOLE);
			if (hole_start == -1 || hole_start == src_size) {
				// No more holes (or lseek failed), so break.
				trailing_hole = false;
				break;
			}
		}

		if ((hole_end == -1 || hole_start == -1) && errno != ENXIO) {
			// A call to lseek() failed. Hole punching is best effort, so exit.
			copyfile_warn("lseek during hole punching failed");
			goto set_total_copied;
		}

		// We will still have a trailing hole to punch if the last lseek(SEEK_HOLE) succeeded.
		if (trailing_hole) {
			// Since we can only punch iosize-aligned holes, we must make sure the last hole
			// is iosize-aligned. Unfortunately, no good truncate macros are in scope here,
			// so we must round down the end of the trailing hole to an iosize boundary ourselves.
			hole_end = (src_size % iosize == 0) ? src_size : roundup(src_size, iosize) - iosize;

			memset(&punchhole_args, 0, sizeof(punchhole_args));
			punchhole_args.fp_offset = hole_start - src_start + dst_start;
			punchhole_args.fp_length = hole_end - hole_start;
			if (fcntl(dst_fd, F_PUNCHHOLE, &punchhole_args) == -1) {
				copyfile_warn("unable to punch trailing hole in destination file, offset %lld",
							  hole_start - src_start + dst_start);
				goto set_total_copied;
			}
		}
	}

set_total_copied:
	// Since we don't know in advance how many bytes we're copying, we advance this number
	// as we copy, but to match copyfile_data() we set it here to the amount of bytes that would
	// have been transferred in a full copy.
	s->totalCopied = src_size - src_start;

exit:
	if (bp) {
		free(bp);
		bp = NULL;
	}

	return rc;

error_exit:
	s->err = errno;
	rc = -1;
	goto exit;
}

/*
 * Attempt to copy the data section of a file.  Using blockisize
 * is not necessarily the fastest -- it might be desirable to
 * specify a blocksize, somehow.  But it's a size that should be
 * guaranteed to work.
 */
static int copyfile_data(copyfile_state_t s)
{
	size_t blen;
	char *bp = 0;
	ssize_t nread;
	int ret = 0;
	size_t iBlocksize = 0, iMinblocksize = 0;
	size_t oBlocksize = 0, oMinblocksize = 0; // If 0, we don't support sparse copying.
	const size_t blocksize_limit = 1 << 30; // 1 GiB
	struct statfs sfs;
	copyfile_callback_t status = s->statuscb;

	/* Unless it's a normal file, we don't copy.  For now, anyway */
	if ((s->sb.st_mode & S_IFMT) != S_IFREG)
		return 0;

#ifdef VOL_CAP_FMT_DECMPFS_COMPRESSION
	if (s->internal_flags & cfSawDecmpEA) {
		if (s->sb.st_flags & UF_COMPRESSED) {
			if ((s->flags & COPYFILE_STAT) == 0) {
				if (fchflags(s->dst_fd, UF_COMPRESSED) == 0) {
					goto exit;
				}
			}
		}
	}
#endif

	// Calculate the input and output block sizes.
	// Our output block size can be no greater than our input block size.
	if (fstatfs(s->src_fd, &sfs) == -1) {
		iBlocksize = s->sb.st_blksize;
	} else {
		iBlocksize = sfs.f_iosize;
		iMinblocksize = sfs.f_bsize;
	}

	if (fstatfs(s->dst_fd, &sfs) == -1) {
		oBlocksize = iBlocksize;
	} else {
		oBlocksize = (sfs.f_iosize == 0) ? iBlocksize : MIN((size_t) sfs.f_iosize, iBlocksize);
		oMinblocksize = sfs.f_bsize;
	}

	// 6453525 and 34848916 require us to limit our blocksize to resonable values.
	if ((size_t) s->sb.st_size < iBlocksize && iMinblocksize > 0) {
		copyfile_debug(3, "rounding up block size from fsize: %lld to multiple of %zu\n", s->sb.st_size, iMinblocksize);
		iBlocksize = roundup((size_t) s->sb.st_size, iMinblocksize);
		oBlocksize = MIN(oBlocksize, iBlocksize);
	}

	if (iBlocksize > blocksize_limit) {
		iBlocksize = blocksize_limit;
		oBlocksize = MIN(oBlocksize, iBlocksize);
	}

	copyfile_debug(3, "input block size: %zu output block size: %zu\n", iBlocksize, oBlocksize);

	s->totalCopied = 0;

	// If requested, attempt a sparse copy.
	if (s->flags & COPYFILE_DATA_SPARSE) {
		// Check if the source & destination volumes both support sparse files.
		long min_hole_size = MIN(fpathconf(s->src_fd, _PC_MIN_HOLE_SIZE),
								 fpathconf(s->dst_fd, _PC_MIN_HOLE_SIZE));

		// If holes are supported on both the source and dest volumes, make sure our min_hole_size
		// is reasonable: if it's smaller than the source/dest block size,
		// our copy performance will suffer (and we may not create sparse files).
		if (iMinblocksize > 0 && oMinblocksize > 0 && (size_t) min_hole_size >= iMinblocksize
			&& (size_t) min_hole_size >= oMinblocksize) {
			// Do the copy.
			ret = copyfile_data_sparse(s, iMinblocksize, oMinblocksize);

			// If we returned an error, exit gracefully.
			// If sparse copying is not supported, we try full copying if allowed by our caller.
			if (ret == 0) {
				goto exit;
			} else if (ret != ENOTSUP) {
				goto exit;
			}
			ret = 0;
		}

		// Make sure we're allowed to perform non-sparse copying.
		if (!(s->flags & COPYFILE_DATA)) {
			ret = -1;
			errno = ENOTSUP;
			goto exit;
		}
	}

	if ((bp = malloc(iBlocksize)) == NULL)
		return -1;

	blen = iBlocksize;

	/* If supported, do preallocation for Xsan / HFS / apfs volumes */
#ifdef F_PREALLOCATE
	{
		off_t dst_bytes_allocated = 0;
		struct stat dst_sb;

		if (fstat(s->dst_fd, &dst_sb) == 0) {
			// The destination may already have
			// preallocated space we can use.
			dst_bytes_allocated = dst_sb.st_blocks * S_BLKSIZE;
		}

		if (dst_bytes_allocated < s->sb.st_size) {
			fstore_t fst;

			fst.fst_flags = 0;
			fst.fst_posmode = F_PEOFPOSMODE;
			fst.fst_offset = 0;
			fst.fst_length = s->sb.st_size - dst_bytes_allocated;

			copyfile_debug(3, "preallocating %lld bytes on destination", fst.fst_length);
			/* Ignore errors; this is merely advisory. */
			(void)fcntl(s->dst_fd, F_PREALLOCATE, &fst);
		}
	}
#endif

	while ((nread = read(s->src_fd, bp, blen)) > 0)
	{
		ssize_t nwritten;
		size_t left = nread;
		void *ptr = bp;
		int loop = 0;

		while (left > 0) {
			nwritten = write(s->dst_fd, ptr, MIN(left, oBlocksize));
			switch (nwritten) {
				case 0:
					if (++loop > 5) {
						copyfile_warn("writing to output %d times resulted in 0 bytes written", loop);
						ret = -1;
						s->err = EAGAIN;
						goto exit;
					}
					break;
				case -1:
					copyfile_warn("writing to output file got error");
					if (status) {
						int rv = (*status)(COPYFILE_COPY_DATA, COPYFILE_ERR, s, s->src, s->dst, s->ctx);
						if (rv == COPYFILE_SKIP) {	// Skip the data copy
							ret = 0;
							goto exit;
						}
						if (rv == COPYFILE_CONTINUE) {	// Retry the write
							errno = 0;
							continue;
						}
					}
					ret = -1;
					goto exit;
				default:
					left -= nwritten;
					ptr = ((char*)ptr) + nwritten;
					loop = 0;
					break;
			}
			s->totalCopied += nwritten;
			if (status) {
				int rv = (*status)(COPYFILE_COPY_DATA, COPYFILE_PROGRESS,  s, s->src, s->dst, s->ctx);
				if (rv == COPYFILE_QUIT) {
					ret = -1; s->err = errno = ECANCELED;
					goto exit;
				}
			}
		}
	}
	if (nread < 0)
	{
		copyfile_warn("reading from %s", s->src ? s->src : "(null src)");
		ret = -1;
		goto exit;
	}

	if (ftruncate(s->dst_fd, s->totalCopied) < 0)
	{
		ret = -1;
		goto exit;
	}

exit:
	if (ret == -1)
	{
		s->err = errno;
	}
	free(bp);
	return ret;
}

/*
 * copyfile_security() will copy the ACL set, and the
 * POSIX set.  Complexities come when dealing with
 * inheritied permissions, and when dealing with both
 * POSIX and ACL permissions.
 */
static int copyfile_security(copyfile_state_t s)
{
	int copied = 0;
	struct stat sb;
	acl_t acl_src = NULL, acl_tmp = NULL, acl_dst = NULL;
	int ret = 0;
	filesec_t tmp_fsec = NULL;
	filesec_t fsec_dst = filesec_init();

	if (fsec_dst == NULL)
		return -1;


	if (COPYFILE_ACL & s->flags)
	{
		if (filesec_get_property(s->fsec, FILESEC_ACL, &acl_src))
		{
			if (errno == ENOENT)
				acl_src = NULL;
			else
				goto error_exit;
		}

		/* grab the destination acl
		 cannot assume it's empty due to inheritance
		 */
		if(fstatx_np(s->dst_fd, &sb, fsec_dst))
			goto error_exit;

		if (filesec_get_property(fsec_dst, FILESEC_ACL, &acl_dst))
		{
			if (errno == ENOENT)
				acl_dst = NULL;
			else
				goto error_exit;
		}

		if (acl_src == NULL && acl_dst == NULL)
			goto no_acl;

		acl_tmp = acl_init(4);
		if (acl_tmp == NULL)
			goto error_exit;

		if (acl_src) {
			acl_entry_t ace = NULL;
			acl_entry_t tmp = NULL;
			for (copied = 0;
				 acl_get_entry(acl_src,
							   ace == NULL ? ACL_FIRST_ENTRY : ACL_NEXT_ENTRY,
							   &ace) == 0;)
			{
				acl_flagset_t flags = { 0 };
				acl_get_flagset_np(ace, &flags);
				if (!acl_get_flag_np(flags, ACL_ENTRY_INHERITED))
				{
					if ((ret = acl_create_entry(&acl_tmp, &tmp)) == -1)
						goto error_exit;

					if ((ret = acl_copy_entry(tmp, ace)) == -1)
						goto error_exit;

					copyfile_debug(2, "copied acl entry from %s to %s",
								   s->src ? s->src : "(null src)",
								   s->dst ? s->dst : "(null tmp)");
					copied++;
				}
			}
		}
		if (acl_dst) {
			acl_entry_t ace = NULL;
			acl_entry_t tmp = NULL;
			acl_flagset_t flags = { 0 };
			for (copied = 0;acl_get_entry(acl_dst,
										  ace == NULL ? ACL_FIRST_ENTRY : ACL_NEXT_ENTRY,
										  &ace) == 0;)
			{
				acl_get_flagset_np(ace, &flags);
				if (acl_get_flag_np(flags, ACL_ENTRY_INHERITED))
				{
					if ((ret = acl_create_entry(&acl_tmp, &tmp)) == -1)
						goto error_exit;

					if ((ret = acl_copy_entry(tmp, ace)) == -1)
						goto error_exit;

					copyfile_debug(2, "copied acl entry from %s to %s",
								   s->src ? s->src : "(null dst)",
								   s->dst ? s->dst : "(null tmp)");
					copied++;
				}
			}
		}
		if (!filesec_set_property(s->fsec, FILESEC_ACL, &acl_tmp))
		{
			copyfile_debug(3, "altered acl");
		}
	}
no_acl:
	/*
	 * The following code is attempting to ensure that only the requested
	 * security information gets copied over to the destination file.
	 * We essentially have four cases:  COPYFILE_ACL, COPYFILE_STAT,
	 * COPYFILE_(STAT|ACL), and none (in which case, we wouldn't be in
	 * this function).
	 *
	 * If we have both flags, we copy everything; if we have ACL but not STAT,
	 * we remove the POSIX information from the filesec object, and apply the
	 * ACL; if we have STAT but not ACL, then we just use fchmod(), and ignore
	 * the extended version.
	 */
	tmp_fsec = filesec_dup(s->fsec);
	if (tmp_fsec == NULL) {
		goto error_exit;
	}

	switch (COPYFILE_SECURITY & s->flags) {
		case COPYFILE_ACL:
			copyfile_unset_posix_fsec(tmp_fsec);
			/* FALLTHROUGH */
		case COPYFILE_ACL | COPYFILE_STAT:
			if (fchmodx_np(s->dst_fd, tmp_fsec) < 0) {
				acl_t acl = NULL;
				/*
				 * The call could have failed for a number of reasons, since
				 * it does a number of things:  it changes the mode of the file,
				 * sets the owner and group, and applies an ACL (if one exists).
				 * The typical failure is going to be trying to set the group of
				 * the destination file to match the source file, when the process
				 * doesn't have permission to put files in that group.  We try to
				 * work around this by breaking the steps out and doing them
				 * discretely.  We don't care if the fchown fails, but we do care
				 * if the mode or ACL can't be set.  For historical reasons, we
				 * simply log those failures, however.
				 *
				 * Big warning here:  we may NOT have COPYFILE_STAT set, since
				 * we fell-through from COPYFILE_ACL.  So check for the fchmod.
				 */

#define NS(x)	((x) ? (x) : "(null string)")
				if ((s->flags & COPYFILE_STAT) &&
					fchmod(s->dst_fd, s->sb.st_mode) == -1) {
					copyfile_warn("could not change mode of destination file %s to match source file %s", NS(s->dst), NS(s->src));
				}
				(void)fchown(s->dst_fd, s->sb.st_uid, s->sb.st_gid);
				if (filesec_get_property(tmp_fsec, FILESEC_ACL, &acl) == 0) {
					if (acl_set_fd(s->dst_fd, acl) == -1) {
						copyfile_warn("could not apply acl to destination file %s from source file %s", NS(s->dst), NS(s->src));
					}
					acl_free(acl);
				}
			}
#undef NS
			break;
		case COPYFILE_STAT:
			(void)fchmod(s->dst_fd, s->sb.st_mode);
			break;
	}
	filesec_free(tmp_fsec);
exit:
	filesec_free(fsec_dst);
	if (acl_src) acl_free(acl_src);
	if (acl_dst) acl_free(acl_dst);
	if (acl_tmp) acl_free(acl_tmp);

	return ret;

error_exit:
	ret = -1;
	goto exit;

}

/*
 * Attempt to set the destination file's stat information -- including
 * flags and time-related fields -- to the source's.
 * Note that we must set file flags *last*, as setting a flag like
 * UF_IMMUTABLE can prevent us from setting other attributes.
 */
static int copyfile_stat(copyfile_state_t s)
{
	unsigned int added_flags = 0, dst_flags = 0;
	struct attrlist attrlist;
	struct stat dst_sb;
	struct {
		/* Order of these structs matters for setattrlist. */
		struct timespec mod_time;
		struct timespec acc_time;
	} ma_times;

	/* Try to set m/atimes using setattrlist(), for nanosecond precision. */
	memset(&attrlist, 0, sizeof(attrlist));
	attrlist.bitmapcount = ATTR_BIT_MAP_COUNT;
	attrlist.commonattr = ATTR_CMN_MODTIME | ATTR_CMN_ACCTIME;
	ma_times.mod_time = s->sb.st_mtimespec;
	ma_times.acc_time = s->sb.st_atimespec;
	(void)fsetattrlist(s->dst_fd, &attrlist, &ma_times, sizeof(ma_times), 0);

	/* If this fails, we don't care */
	(void)fchown(s->dst_fd, s->sb.st_uid, s->sb.st_gid);

	/* This may have already been done in copyfile_security() */
	(void)fchmod(s->dst_fd, s->sb.st_mode & ~S_IFMT);

	/*
	 * NFS doesn't support chflags; ignore errors as a result, since
	 * we don't return failure for this.
	 */
	if (s->internal_flags & cfMakeFileInvisible)
		added_flags |= UF_HIDDEN;

	/*
	 * We need to check if certain flags were set on the destination
	 * by the kernel.  If they were, don't drop them.
	 */
	if (fstat(s->dst_fd, &dst_sb))
		return -1;
	added_flags |= (dst_sb.st_flags & COPYFILE_PRESERVE_FLAGS);

	/*
	 * The caller requested that copyfile attempts to preserve UF_TRACKED
	 * on the destination. This can be used to avoid dangling docIDs when
	 * the copy races against a process that sets the flag on newly created
	 * documents for instance.
	 */
	if (s->flags & COPYFILE_PRESERVE_DST_TRACKED) {
		added_flags |= (dst_sb.st_flags & UF_TRACKED);
	}

	/* Copy file flags, masking out any we don't want to preserve */
	dst_flags = (s->sb.st_flags & ~COPYFILE_OMIT_FLAGS) | added_flags;
	(void)fchflags(s->dst_fd, dst_flags);

	return 0;
}

/*
 * Similar to copyfile_security() in some ways; this
 * routine copies the extended attributes from the source,
 * and sets them on the destination.
 * The procedure is pretty simple, even if it is verbose:
 * for each named attribute on the destination, get its name, and
 * remove it.  We should have none after that.
 * For each named attribute on the source, get its name, get its
 * data, and set it on the destination.
 */
static int copyfile_xattr(copyfile_state_t s)
{
	char *name;
	char *namebuf, *end;
	ssize_t xa_size;
	void *xa_dataptr;
	ssize_t bufsize = 4096;
	ssize_t asize;
	ssize_t nsize;
	int ret = 0;
	int look_for_decmpea = 0;

	/* delete EAs on destination */
	if ((nsize = flistxattr(s->dst_fd, 0, 0, 0)) > 0)
	{
		if ((namebuf = (char *) malloc(nsize)) == NULL)
			return -1;
		else
			nsize = flistxattr(s->dst_fd, namebuf, nsize, 0);

		if (nsize > 0) {
			/*
			 * With this, end points to the last byte of the allocated buffer
			 * This *should* be NUL, from flistxattr, but if it's not, we can
			 * set it anyway -- it'll result in a truncated name, which then
			 * shouldn't match when we get them later.
			 */
			end = namebuf + nsize - 1;
			if (*end != 0)
				*end = 0;
			for (name = namebuf; name <= end; name += strlen(name) + 1) {
				/* If the quarantine information shows up as an EA, we skip over it */
				if (strncmp(name, XATTR_QUARANTINE_NAME, end - name) == 0) {
					continue;
				}
				fremovexattr(s->dst_fd, name,0);
			}
		}
		free(namebuf);
	} else
		if (nsize < 0)
		{
			if (errno == ENOTSUP || errno == EPERM)
				return 0;
			else
				return -1;
		}

#ifdef DECMPFS_XATTR_NAME
	if ((s->flags & COPYFILE_DATA) &&
		(s->sb.st_flags & UF_COMPRESSED) &&
		doesdecmpfs(s->src_fd) &&
		doesdecmpfs(s->dst_fd)) {
		look_for_decmpea = XATTR_SHOWCOMPRESSION;
	}
#endif

	/* get name list of EAs on source */
	if ((nsize = flistxattr(s->src_fd, 0, 0, look_for_decmpea)) < 0)
	{
		if (errno == ENOTSUP || errno == EPERM)
			return 0;
		else
			return -1;
	} else
		if (nsize == 0)
			return 0;

	if ((namebuf = (char *) malloc(nsize)) == NULL)
		return -1;
	else
		nsize = flistxattr(s->src_fd, namebuf, nsize, look_for_decmpea);

	if (nsize <= 0) {
		free(namebuf);
		return (int)nsize;
	}

	/*
	 * With this, end points to the last byte of the allocated buffer
	 * This *should* be NUL, from flistxattr, but if it's not, we can
	 * set it anyway -- it'll result in a truncated name, which then
	 * shouldn't match when we get them later.
	 */
	end = namebuf + nsize - 1;
	if (*end != 0)
		*end = 0;

	if ((xa_dataptr = (void *) malloc(bufsize)) == NULL) {
		free(namebuf);
		return -1;
	}

	for (name = namebuf; name <= end; name += strlen(name) + 1)
	{
		if (s->xattr_name) {
			free(s->xattr_name);
			s->xattr_name = NULL;
		}

		/* If the quarantine information shows up as an EA, we skip over it */
		if (strncmp(name, XATTR_QUARANTINE_NAME, end - name) == 0)
			continue;

		if ((xa_size = fgetxattr(s->src_fd, name, 0, 0, 0, look_for_decmpea)) < 0)
		{
			continue;
		}

		if (xa_size > bufsize)
		{
			void *tdptr = xa_dataptr;
			bufsize = xa_size;
			if ((xa_dataptr =
				 (void *) realloc((void *) xa_dataptr, bufsize)) == NULL)
			{
				free(tdptr);
				ret = -1;
				continue;
			}
		}

		if ((asize = fgetxattr(s->src_fd, name, xa_dataptr, xa_size, 0, look_for_decmpea)) < 0)
		{
			continue;
		}

		if (xa_size != asize)
			xa_size = asize;

#ifdef DECMPFS_XATTR_NAME
		if (strncmp(name, DECMPFS_XATTR_NAME, end-name) == 0)
		{
			decmpfs_disk_header *hdr = xa_dataptr;

			/*
			 * If the EA has the decmpfs name, but is too
			 * small, or doesn't have the right magic number,
			 * or isn't the right type, we'll just skip it.
			 * This means it won't end up in the destination
			 * file, and data copy will happen normally.
			 */
			if ((size_t)xa_size < sizeof(decmpfs_disk_header)) {
				continue;
			}
			if (OSSwapLittleToHostInt32(hdr->compression_magic) != DECMPFS_MAGIC) {
				continue;
			}
			/*
			 * From AppleFSCompression documentation:
			 * "It is incumbent on the aware copy engine to identify
			 *  the type of compression being used, and to perform an
			 *  unaware copy of any file it does not recognize."
			 *
			 * Compression Types are defined in:
			 * "AppleFSCompression/Common/compressorCommon.h"
			 *
			 * Unfortunately, they don't provide a way to dynamically
			 * determine what possible compression_type values exist,
			 * so we have to update this every time a new compression_type
			 * is added. Types 7->10 were added in 10.10, Types 11 & 12
			 * were added in 10.11.
			 *
			 * Ubiquity faulting file compression type 0x80000001 are
			 * deprecated as of Yosemite, per rdar://17714998 don't copy the
			 * decmpfs xattr on these files, zero byte files are safer
			 * than a fault nobody knows how to handle.
			 */
			switch (OSSwapLittleToHostInt32(hdr->compression_type)) {
				case 3:  /* zlib-compressed data in xattr */
				case 4:  /* 64k chunked zlib-compressed data in resource fork */

				case 7:  /* LZVN-compressed data in xattr */
				case 8:  /* 64k chunked LZVN-compressed data in resource fork */

				case 9:  /* uncompressed data in xattr (similar to but not identical to CMP_Type1) */
				case 10: /* 64k chunked uncompressed data in resource fork */

				case 11: /* LZFSE-compressed data in xattr */
				case 12: /* 64k chunked LZFSE-compressed data in resource fork */

					/* valid compression type, we want to copy. */
					break;

				case 5: /* specifies de-dup within the generation store. Don't copy decmpfs xattr. */
					copyfile_debug(3, "compression_type <5> on attribute com.apple.decmpfs for src file %s is not copied.",
								   s->src ? s->src : "(null string)");
					continue;

				case 6: /* unused */
				case 0x80000001: /* faulting files are deprecated, don't copy decmpfs xattr */
				default:
					copyfile_warn("Invalid compression_type <%d> on attribute %s for src file %s",
				      OSSwapLittleToHostInt32(hdr->compression_type), name, s->src ? s->src : "(null string)");
					continue;
			}
			s->internal_flags |= cfSawDecmpEA;
		}
#endif

		// If we have a copy intention stated, and the EA is to be ignored, we ignore it
		if (s->copyIntent
			&& xattr_preserve_for_intent(name, s->copyIntent) == 0)
			continue;

		s->xattr_name = strdup(name);

		if (s->statuscb) {
			int rv;
			rv = (*s->statuscb)(COPYFILE_COPY_XATTR, COPYFILE_START, s, s->src, s->dst, s->ctx);
			if (rv == COPYFILE_QUIT) {
				s->err = ECANCELED;
				goto out;
			} else if (rv == COPYFILE_SKIP) {
				continue;
			}
		}
		if (fsetxattr(s->dst_fd, name, xa_dataptr, xa_size, 0, look_for_decmpea) < 0)
		{
			int error = errno;
			if (error == EPERM && strcmp(name, XATTR_ROOT_INSTALLED_NAME) == 0) {
				//Silently ignore if we fail to set XATTR_ROOT_INSTALLED_NAME
				errno = error;
				continue;
			}
			else if (s->statuscb)
			{
				int rv;
				error = errno;
				if (s->xattr_name == NULL)
					s->xattr_name = strdup(name);
				rv = (*s->statuscb)(COPYFILE_COPY_XATTR, COPYFILE_ERR, s, s->src, s->dst, s->ctx);
				if (rv == COPYFILE_QUIT)
				{
					s->err = ECANCELED;
					ret = -1;
					goto out;
				}
			}
			else
			{
				errno = error;
				ret = -1;
				copyfile_warn("could not set attributes %s on destination file descriptor", name);
				continue;
			}
		}
		if (s->statuscb) {
			int rv;
			if (s->xattr_name == NULL)
				s->xattr_name = strdup(name);

			rv = (*s->statuscb)(COPYFILE_COPY_XATTR, COPYFILE_FINISH, s, s->src, s->dst, s->ctx);
			if (rv == COPYFILE_QUIT) {
				s->err = ECANCELED;
				goto out;
			}
		}
	}
out:
	if (namebuf)
		free(namebuf);
	free((void *) xa_dataptr);
	if (s->xattr_name) {
		free(s->xattr_name);
		s->xattr_name = NULL;
	}
	return ret;
}

/*
 * API interface into getting data from the opaque data type.
 */
int copyfile_state_get(copyfile_state_t s, uint32_t flag, void *ret)
{
	if (ret == NULL)
	{
		errno = EFAULT;
		return -1;
	}

	switch(flag)
	{
		case COPYFILE_STATE_SRC_FD:
			*(int*)ret = s->src_fd;
			break;
		case COPYFILE_STATE_DST_FD:
			*(int*)ret = s->dst_fd;
			break;
		case COPYFILE_STATE_SRC_FILENAME:
			*(char**)ret = s->src;
			break;
		case COPYFILE_STATE_DST_FILENAME:
			*(char**)ret = s->dst;
			break;
		case COPYFILE_STATE_QUARANTINE:
			*(qtn_file_t*)ret = s->qinfo;
			break;
#if 0
		case COPYFILE_STATE_STATS:
			ret = s->stats.global;
			break;
		case COPYFILE_STATE_PROGRESS_CB:
			ret = s->callbacks.progress;
			break;
#endif
#ifdef	COPYFILE_STATE_STATUS_CB
		case COPYFILE_STATE_STATUS_CB:
			*(copyfile_callback_t*)ret = s->statuscb;
			break;
		case COPYFILE_STATE_STATUS_CTX:
			*(void**)ret = s->ctx;
			break;
		case COPYFILE_STATE_COPIED:
			*(off_t*)ret = s->totalCopied;
			break;
#endif
#ifdef COPYFILE_STATE_XATTRNAME
		case COPYFILE_STATE_XATTRNAME:
			*(char**)ret = s->xattr_name;
			break;
#endif
#ifdef COPYFILE_STATE_INTENT
		case COPYFILE_STATE_INTENT:
			*(xattr_operation_intent_t*)ret = s->copyIntent;
			break;
#endif
		case COPYFILE_STATE_WAS_CLONED:
			*(bool *)ret = s->was_cloned;
			break;
		default:
			errno = EINVAL;
			ret = NULL;
			return -1;
	}
	return 0;
}

/*
 * Public API for setting state data (remember that the state is
 * an opaque data type).
 */
int copyfile_state_set(copyfile_state_t s, uint32_t flag, const void * thing)
{
#define copyfile_set_string(DST, SRC) \
	do {					\
		if (SRC != NULL) {			\
			DST = strdup((char *)SRC);		\
		} else {				\
			if (DST != NULL) {			\
				free(DST);			\
			}					\
			DST = NULL;				\
		}					\
	} while (0)

	if (thing == NULL)
	{
		errno = EFAULT;
		return  -1;
	}

	switch(flag)
	{
		case COPYFILE_STATE_SRC_FD:
			s->src_fd = *(int*)thing;
			break;
		case COPYFILE_STATE_DST_FD:
			s->dst_fd = *(int*)thing;
			break;
		case COPYFILE_STATE_SRC_FILENAME:
			copyfile_set_string(s->src, thing);
			break;
		case COPYFILE_STATE_DST_FILENAME:
			copyfile_set_string(s->dst, thing);
			break;
		case COPYFILE_STATE_QUARANTINE:
			if (s->qinfo)
			{
				qtn_file_free(s->qinfo);
				s->qinfo = NULL;
			}
			if (*(qtn_file_t*)thing)
				s->qinfo = qtn_file_clone(*(qtn_file_t*)thing);
			break;
#if 0
		case COPYFILE_STATE_STATS:
			s->stats.global = thing;
			break;
		case COPYFILE_STATE_PROGRESS_CB:
			s->callbacks.progress = thing;
			break;
#endif
#ifdef COPYFILE_STATE_STATUS_CB
		case COPYFILE_STATE_STATUS_CB:
			s->statuscb = (copyfile_callback_t)thing;
			break;
		case COPYFILE_STATE_STATUS_CTX:
			s->ctx = (void*)thing;
			break;
#endif
#ifdef COPYFILE_STATE_INTENT
		case COPYFILE_STATE_INTENT:
			s->copyIntent = *(xattr_operation_intent_t*)thing;
			break;
#endif
		default:
			errno = EINVAL;
			return -1;
	}
	return 0;
#undef copyfile_set_string
}


/*
 * Make this a standalone program for testing purposes by
 * defining _COPYFILE_TEST.
 */
#ifdef _COPYFILE_TEST
#define COPYFILE_OPTION(x) { #x, COPYFILE_ ## x },

struct {char *s; int v;} opts[] = {
	COPYFILE_OPTION(ACL)
	COPYFILE_OPTION(STAT)
	COPYFILE_OPTION(XATTR)
	COPYFILE_OPTION(DATA)
	COPYFILE_OPTION(SECURITY)
	COPYFILE_OPTION(METADATA)
	COPYFILE_OPTION(ALL)
	COPYFILE_OPTION(NOFOLLOW_SRC)
	COPYFILE_OPTION(NOFOLLOW_DST)
	COPYFILE_OPTION(NOFOLLOW)
	COPYFILE_OPTION(EXCL)
	COPYFILE_OPTION(MOVE)
	COPYFILE_OPTION(UNLINK)
	COPYFILE_OPTION(PACK)
	COPYFILE_OPTION(UNPACK)
	COPYFILE_OPTION(CHECK)
	COPYFILE_OPTION(CLONE)
	COPYFILE_OPTION(CLONE_FORCE)
	COPYFILE_OPTION(VERBOSE)
	COPYFILE_OPTION(RECURSIVE)
	COPYFILE_OPTION(DEBUG)
	COPYFILE_OPTION(CLONE)
	COPYFILE_OPTION(CLONE_FORCE)
	COPYFILE_OPTION(DATA_SPARSE)
	{NULL, 0}
};

int main(int c, char *v[])
{
	int i, ret;
	int flags = 0;
	copyfile_state_t state = NULL;

	if (c < 3)
		errx(1, "insufficient arguments");

	while(c-- > 3)
	{
		for (i = 0; opts[i].s != NULL; ++i)
		{
			if (strcasecmp(opts[i].s, v[c]) == 0)
			{
				printf("option %d: %s <- %d\n", c, opts[i].s, opts[i].v);
				flags |= opts[i].v;
				break;
			}
		}
	}

	if (flags & COPYFILE_DEBUG) {
		state = copyfile_state_alloc();
		state->debug = 10; // Turn on all debug statements
	}
	ret = copyfile(v[1], v[2], state, flags);
	if (state) {
		(void)copyfile_state_free(state);
	}

	return ret;
}
#endif
/*
 * Apple Double Create
 *
 * Create an Apple Double "._" file from a file's extented attributes
 *
 * Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
 */

#define	XATTR_MAXATTRLEN   (16*1024*1024)


/*
 Typical "._" AppleDouble Header File layout:
 ------------------------------------------------------------
 MAGIC          0x00051607
 VERSION        0x00020000
 FILLER         0
 COUNT          2
 .-- AD ENTRY[0]    Finder Info Entry (must be first)
 .--+-- AD ENTRY[1]    Resource Fork Entry (must be last)
 |  '-> FINDER INFO
 |      /////////////  Fixed Size Data (32 bytes)
 |      EXT ATTR HDR
 |      /////////////
 |      ATTR ENTRY[0] --.
 |      ATTR ENTRY[1] --+--.
 |      ATTR ENTRY[2] --+--+--.
 |         ...          |  |  |
 |      ATTR ENTRY[N] --+--+--+--.
 |      ATTR DATA 0   <-'  |  |  |
 |      ////////////       |  |  |
 |      ATTR DATA 1   <----'  |  |
 |      /////////////         |  |
 |      ATTR DATA 2   <-------'  |
 |      /////////////            |
 |         ...                   |
 |      ATTR DATA N   <----------'
 |      /////////////
 |                      Attribute Free Space
 |
 '----> RESOURCE FORK
 /////////////   Variable Sized Data
 /////////////
 /////////////
 /////////////
 /////////////
 /////////////
 ...
 /////////////

 ------------------------------------------------------------

 NOTE: The EXT ATTR HDR, ATTR ENTRY's and ATTR DATA's are
 stored as part of the Finder Info.  The length in the Finder
 Info AppleDouble entry includes the length of the extended
 attribute header, attribute entries, and attribute data.
 */


/*
 * On Disk Data Structures
 *
 * Note: Motorola 68K alignment and big-endian.
 *
 * See RFC 1740 for additional information about the AppleDouble file format.
 *
 */

#define ADH_MAGIC     0x00051607
#define ADH_VERSION   0x00020000
#define ADH_MACOSX    "Mac OS X        "

/*
 * AppleDouble Entry ID's
 */
#define AD_DATA          1   /* Data fork */
#define AD_RESOURCE      2   /* Resource fork */
#define AD_REALNAME      3   /* File's name on home file system */
#define AD_COMMENT       4   /* Standard Mac comment */
#define AD_ICONBW        5   /* Mac black & white icon */
#define AD_ICONCOLOR     6   /* Mac color icon */
#define AD_UNUSED        7   /* Not used */
#define AD_FILEDATES     8   /* File dates; create, modify, etc */
#define AD_FINDERINFO    9   /* Mac Finder info & extended info */
#define AD_MACINFO      10   /* Mac file info, attributes, etc */
#define AD_PRODOSINFO   11   /* Pro-DOS file info, attrib., etc */
#define AD_MSDOSINFO    12   /* MS-DOS file info, attributes, etc */
#define AD_AFPNAME      13   /* Short name on AFP server */
#define AD_AFPINFO      14   /* AFP file info, attrib., etc */
#define AD_AFPDIRID     15   /* AFP directory ID */
#define AD_ATTRIBUTES   AD_FINDERINFO


#define ATTR_FILE_PREFIX   "._"
#define ATTR_HDR_MAGIC     0x41545452   /* 'ATTR' */

#define ATTR_BUF_SIZE      4096        /* default size of the attr file and how much we'll grow by */

/* Implementation Limits */
#define ATTR_MAX_SIZE      (16*1024*1024)	/* 16 megabyte maximum attribute data size */
#define ATTR_MAX_NAME_LEN  128
#define ATTR_MAX_HDR_SIZE  (65536+18)

/*
 * Note: ATTR_MAX_HDR_SIZE is the largest attribute header
 * size supported (including the attribute entries). All of
 * the attribute entries must reside within this limit.
 */


#define FINDERINFOSIZE	32

typedef struct apple_double_entry
{
	u_int32_t   type;     /* entry type: see list, 0 invalid */
	u_int32_t   offset;   /* entry data offset from the beginning of the file. */
	u_int32_t   length;   /* entry data length in bytes. */
} __attribute__((aligned(2), packed)) apple_double_entry_t;


typedef struct apple_double_header
{
	u_int32_t   magic;         /* == ADH_MAGIC */
	u_int32_t   version;       /* format version: 2 = 0x00020000 */
	u_int32_t   filler[4];
	u_int16_t   numEntries;	   /* number of entries which follow */
	apple_double_entry_t   entries[2];  /* 'finfo' & 'rsrc' always exist */
	u_int8_t    finfo[FINDERINFOSIZE];  /* Must start with Finder Info (32 bytes) */
	u_int8_t    pad[2];        /* get better alignment inside attr_header */
} __attribute__((aligned(2), packed)) apple_double_header_t;


/* Entries are aligned on 4 byte boundaries */
typedef struct attr_entry
{
	u_int32_t   offset;    /* file offset to data */
	u_int32_t   length;    /* size of attribute data */
	u_int16_t   flags;
	u_int8_t    namelen;   /* length of name including NULL termination char */
	u_int8_t    name[1];   /* NULL-terminated UTF-8 name (up to 128 bytes max) */
} __attribute__((aligned(2), packed)) attr_entry_t;



/* Header + entries must fit into 64K */
typedef struct attr_header
{
	apple_double_header_t  appledouble;
	u_int32_t   magic;        /* == ATTR_HDR_MAGIC */
	u_int32_t   debug_tag;    /* for debugging == file id of owning file */
	u_int32_t   total_size;   /* total size of attribute header + entries + data */
	u_int32_t   data_start;   /* file offset to attribute data area */
	u_int32_t   data_length;  /* length of attribute data area */
	u_int32_t   reserved[3];
	u_int16_t   flags;
	u_int16_t   num_attrs;
} __attribute__((aligned(2), packed)) attr_header_t;

/* Empty Resource Fork Header */
/* This comes by way of xnu's vfs_xattr.c */
typedef struct rsrcfork_header {
	u_int32_t	fh_DataOffset;
	u_int32_t	fh_MapOffset;
	u_int32_t	fh_DataLength;
	u_int32_t	fh_MapLength;
	u_int8_t	systemData[112];
	u_int8_t	appData[128];
	u_int32_t	mh_DataOffset;
	u_int32_t	mh_MapOffset;
	u_int32_t	mh_DataLength;
	u_int32_t	mh_MapLength;
	u_int32_t	mh_Next;
	u_int16_t	mh_RefNum;
	u_int8_t	mh_Attr;
	u_int8_t	mh_InMemoryAttr;
	u_int16_t	mh_Types;
	u_int16_t	mh_Names;
	u_int16_t	typeCount;
} __attribute__((aligned(2), packed)) rsrcfork_header_t;
#define RF_FIRST_RESOURCE    256
#define RF_NULL_MAP_LENGTH    30
#define RF_EMPTY_TAG  "This resource fork intentionally left blank   "

static const rsrcfork_header_t empty_rsrcfork_header = {
	OSSwapHostToBigInt32(RF_FIRST_RESOURCE),	// fh_DataOffset
	OSSwapHostToBigInt32(RF_FIRST_RESOURCE),	// fh_MapOffset
	0,						// fh_DataLength
	OSSwapHostToBigInt32(RF_NULL_MAP_LENGTH),	// fh_MapLength
	{ RF_EMPTY_TAG, },				// systemData
	{ 0 },						// appData
	OSSwapHostToBigInt32(RF_FIRST_RESOURCE),	// mh_DataOffset
	OSSwapHostToBigInt32(RF_FIRST_RESOURCE),	// mh_MapOffset
	0,						// mh_DataLength
	OSSwapHostToBigInt32(RF_NULL_MAP_LENGTH),	// mh_MapLength
	0,						// mh_Next
	0,						// mh_RefNum
	0,						// mh_Attr
	0,						// mh_InMemoryAttr
	OSSwapHostToBigInt16(RF_NULL_MAP_LENGTH - 2),	// mh_Types
	OSSwapHostToBigInt16(RF_NULL_MAP_LENGTH),	// mh_Names
	OSSwapHostToBigInt16(-1),			// typeCount
};

#define SWAP16(x)	OSSwapBigToHostInt16(x)
#define SWAP32(x)	OSSwapBigToHostInt32(x)
#define SWAP64(x)	OSSwapBigToHostInt64(x)

#define ATTR_ALIGN 3L  /* Use four-byte alignment */

#define ATTR_ENTRY_LENGTH(namelen)  \
	((sizeof(attr_entry_t) - 1 + (namelen) + ATTR_ALIGN) & (~ATTR_ALIGN))

#define ATTR_NEXT(ae)  \
	(attr_entry_t *)((u_int8_t *)(ae) + ATTR_ENTRY_LENGTH((ae)->namelen))

#define	XATTR_SECURITY_NAME	  "com.apple.acl.text"

/*
 * Endian swap Apple Double header
 */
static void
swap_adhdr(apple_double_header_t *adh)
{
#if BYTE_ORDER == LITTLE_ENDIAN
	int count;
	int i;

	count = (adh->magic == ADH_MAGIC) ? adh->numEntries : SWAP16(adh->numEntries);

	adh->magic      = SWAP32 (adh->magic);
	adh->version    = SWAP32 (adh->version);
	adh->numEntries = SWAP16 (adh->numEntries);

	for (i = 0; i < count; i++)
	{
		adh->entries[i].type   = SWAP32 (adh->entries[i].type);
		adh->entries[i].offset = SWAP32 (adh->entries[i].offset);
		adh->entries[i].length = SWAP32 (adh->entries[i].length);
	}
#else
	(void)adh;
#endif
}

/*
 * Endian swap a single attr_entry_t
 */
static void
swap_attrhdr_entry(attr_entry_t *ae)
{
#if BYTE_ORDER == LITTLE_ENDIAN
	ae->offset = SWAP32 (ae->offset);
	ae->length = SWAP32 (ae->length);
	ae->flags  = SWAP16 (ae->flags);
#else
	(void)ae;
#endif
}

/*
 * For a validated/endian swapped attr_header_t*
 * ah, endian swap all of the entries.
 */
static void
swap_attrhdr_entries(attr_header_t *ah)
{
#if BYTE_ORDER == LITTLE_ENDIAN
	int i;
	int count;
	attr_entry_t *entry;
	attr_entry_t *next;

	/* If we're in copyfile_pack, num_args is native endian,
	 * if we're in _unpack, num_args is big endian. Use
	 * the magic number to test for  endianess.
	 */
	count = (ah->magic == ATTR_HDR_MAGIC) ? ah->num_attrs : SWAP16(ah->num_attrs);

	entry = (attr_entry_t *)(&ah[1]);
	for (i = 0; i < count; i++) {
		next = ATTR_NEXT(entry);
		swap_attrhdr_entry(entry);
		entry = next;
	}
#else
	(void)ah;
#endif
}

/*
 * Endian swap extended attributes header
 */
static void
swap_attrhdr(attr_header_t *ah)
{
#if BYTE_ORDER == LITTLE_ENDIAN
	ah->magic       = SWAP32 (ah->magic);
	ah->debug_tag   = SWAP32 (ah->debug_tag);
	ah->total_size  = SWAP32 (ah->total_size);
	ah->data_start  = SWAP32 (ah->data_start);
	ah->data_length = SWAP32 (ah->data_length);
	ah->flags       = SWAP16 (ah->flags);
	ah->num_attrs   = SWAP16 (ah->num_attrs);
#else
	(void)ah;
#endif
}

static const u_int32_t emptyfinfo[8] = {0};

/*
 * Given an Apple Double file in src, turn it into a
 * normal file (possibly with multiple forks, EAs, and
 * ACLs) in dst.
 */
static int copyfile_unpack(copyfile_state_t s)
{
	ssize_t bytes;
	void * buffer, * endptr, * dataptr = NULL;
	apple_double_header_t *adhdr;
	ssize_t hdrsize;
	int error = 0;

	if (s->sb.st_size < ATTR_MAX_HDR_SIZE)
		hdrsize = (ssize_t)s->sb.st_size;
	else
		hdrsize = ATTR_MAX_HDR_SIZE;

	buffer = calloc(1, hdrsize);
	if (buffer == NULL) {
		copyfile_debug(1, "copyfile_unpack: calloc(1, %zu) returned NULL", hdrsize);
		error = -1;
		goto exit;
	} else
		endptr = (char*)buffer + hdrsize;

	bytes = pread(s->src_fd, buffer, hdrsize, 0);

	if (bytes < 0)
	{
		copyfile_debug(1, "pread returned: %zd", bytes);
		error = -1;
		goto exit;
	}
	if (bytes < hdrsize)
	{
		copyfile_debug(1,
					   "pread couldn't read entire header: %d of %d",
					   (int)bytes, (int)s->sb.st_size);
		error = -1;
		goto exit;
	}
	adhdr = (apple_double_header_t *)buffer;

	/*
	 * Check for Apple Double file.
	 */
	if ((size_t)bytes < sizeof(apple_double_header_t) - 2 ||
		SWAP32(adhdr->magic) != ADH_MAGIC ||
		SWAP32(adhdr->version) != ADH_VERSION ||
		SWAP16(adhdr->numEntries) != 2 ||
		SWAP32(adhdr->entries[0].type) != AD_FINDERINFO)
	{
		if (COPYFILE_VERBOSE & s->flags)
			copyfile_warn("Not a valid Apple Double header");
		error = -1;
		goto exit;
	}
	swap_adhdr(adhdr);

	/*
	 * Remove any extended attributes on the target.
	 */

	if ((bytes = flistxattr(s->dst_fd, 0, 0, 0)) > 0)
	{
		char *namebuf, *name;

		if ((namebuf = (char*) malloc(bytes)) == NULL)
		{
			s->err = ENOMEM;
			goto exit;
		}
		bytes = flistxattr(s->dst_fd, namebuf, bytes, 0);

		if (bytes > 0)
			for (name = namebuf; name < namebuf + bytes; name += strlen(name) + 1)
				(void)fremovexattr(s->dst_fd, name, 0);

		free(namebuf);
	}
	else if (bytes < 0)
	{
		if (errno != ENOTSUP && errno != EPERM)
			goto exit;
	}

	/*
	 * Extract the extended attributes.
	 *
	 * >>>  WARNING <<<
	 * This assumes that the data is already in memory (not
	 * the case when there are lots of attributes or one of
	 * the attributes is very large.
	 */
	if (adhdr->entries[0].length > FINDERINFOSIZE)
	{
		attr_header_t *attrhdr;
		attr_entry_t *entry;
		int count;
		int i;

		if ((size_t)hdrsize < sizeof(attr_header_t)) {
			copyfile_warn("bad attribute header:  %zu < %zu", hdrsize, sizeof(attr_header_t));
			error = -1;
			goto exit;
		}

		attrhdr = (attr_header_t *)buffer;
		swap_attrhdr(attrhdr);
		if (attrhdr->magic != ATTR_HDR_MAGIC)
		{
			if (COPYFILE_VERBOSE & s->flags)
				copyfile_warn("bad attribute header");
			error = -1;
			goto exit;
		}
		count = attrhdr->num_attrs;
		entry = (attr_entry_t *)&attrhdr[1];

		for (i = 0; i < count; i++)
		{
			/*
			 * First we do some simple sanity checking.
			 * +) See if entry is within the buffer's range;
			 *
			 * +) Check the attribute name length; if it's longer than the
			 * maximum, we truncate it down.  (We could error out as well;
			 * I'm not sure which is the better way to go here.)
			 *
			 * +) If, given the name length, it goes beyond the end of
			 * the buffer, error out.
			 *
			 * +) If the last byte isn't a NUL, make it a NUL.  (Since we
			 * truncated the name length above, we truncate the name here.)
			 *
			 * +) If entry->offset is so large that it causes dataptr to
			 * go beyond the end of the buffer -- or, worse, so large that
			 * it wraps around! -- we error out.
			 *
			 * +) If entry->length would cause the entry to go beyond the
			 * end of the buffer (or, worse, wrap around to before it),
			 * *or* if the length is larger than the hdrsize, we error out.
			 * (An explanation of that:  what we're checking for there is
			 * the small range of values such that offset+length would cause
			 * it to go beyond endptr, and then wrap around past buffer.  We
			 * care about this because we are passing entry->length down to
			 * fgetxattr() below, and an erroneously large value could cause
			 * problems there.  By making sure that it's less than hdrsize,
			 * which has already been sanity-checked above, we're safe.
			 * That may mean that the check against < buffer is unnecessary.)
			 */
			if ((void*)entry >= endptr || (void*)entry < buffer) {
				if (COPYFILE_VERBOSE & s->flags)
					copyfile_warn("Incomplete or corrupt attribute entry");
				error = -1;
				s->err = EINVAL;
				goto exit;
			}

			if (((char*)entry + sizeof(*entry)) > (char*)endptr) {
				if (COPYFILE_VERBOSE & s->flags)
					copyfile_warn("Incomplete or corrupt attribute entry");
				error = -1;
				s->err = EINVAL;
				goto exit;
			}

			/*
			 * Endian swap the entry we're looking at. Previously
			 * we did this swap as part of swap_attrhdr, but that
			 * allowed a maliciously constructed file to overrun
			 * our allocation. Instead do the swap after we've verified
			 * the entry struct is within the buffer's range.
			 */
			swap_attrhdr_entry(entry);

			if (entry->namelen < 2) {
				if (COPYFILE_VERBOSE & s->flags)
					copyfile_warn("Corrupt attribute entry (only %d bytes)", entry->namelen);
				error = -1;
				s->err = EINVAL;
				goto exit;
			}

			if (entry->namelen > XATTR_MAXNAMELEN + 1) {
				if (COPYFILE_VERBOSE & s->flags)
					copyfile_warn("Corrupt attribute entry (name length is %d bytes)", entry->namelen);
				error = -1;
				s->err = EINVAL;
				goto exit;
			}

			if ((void*)(entry->name + entry->namelen) > endptr) {
				if (COPYFILE_VERBOSE & s->flags)
					copyfile_warn("Incomplete or corrupt attribute entry");
				error = -1;
				s->err = EINVAL;
				goto exit;
			}

			/* Because namelen includes the NUL, we check one byte back */
			if (entry->name[entry->namelen-1] != 0) {
				if (COPYFILE_VERBOSE & s->flags)
					copyfile_warn("Corrupt attribute entry (name is not NUL-terminated)");
				error = -1;
				s->err = EINVAL;
				goto exit;
			}

			copyfile_debug(3, "extracting \"%s\" (%d bytes) at offset %u",
						   entry->name, entry->length, entry->offset);

#if 0
			dataptr = (char *)attrhdr + entry->offset;

			if (dataptr > endptr || dataptr < buffer) {
				copyfile_debug(1, "Entry %d overflows:  offset = %u", i, entry->offset);
				error = -1;
				s->err = EINVAL;	/* Invalid buffer */
				goto exit;
			}

			if (((char*)dataptr + entry->length) > (char*)endptr ||
				(((char*)dataptr + entry->length) < (char*)buffer) ||
				(entry->length > (size_t)hdrsize)) {
				if (COPYFILE_VERBOSE & s->flags)
					copyfile_warn("Incomplete or corrupt attribute entry");
				copyfile_debug(1, "Entry %d length overflows:  offset = %u, length = %u",
							   i, entry->offset, entry->length);
				error = -1;
				s->err = EINVAL;	/* Invalid buffer */
				goto exit;
			}

#else
			dataptr = malloc(entry->length);
			if (dataptr == NULL) {
				copyfile_debug(1, "no memory for %u bytes\n", entry->length);
				error = -1;
				s->err = ENOMEM;
				goto exit;
			}
			if (pread(s->src_fd, dataptr, entry->length, entry->offset) != (ssize_t)entry->length) {
				copyfile_debug(1, "failed to read %u bytes at offset %u\n", entry->length, entry->offset);
				error = -1;
				s->err = EINVAL;
				goto exit;
			}
#endif

			if (strcmp((char*)entry->name, XATTR_QUARANTINE_NAME) == 0)
			{
				qtn_file_t tqinfo = NULL;

				if (s->qinfo == NULL)
				{
					tqinfo = qtn_file_alloc();
					if (tqinfo)
					{
						int x;
						if ((x = qtn_file_init_with_data(tqinfo, dataptr, entry->length)) != 0)
						{
							copyfile_warn("qtn_file_init_with_data failed: %s", qtn_error(x));
							qtn_file_free(tqinfo);
							tqinfo = NULL;
						}
					}
				}
				else
				{
					tqinfo = s->qinfo;
				}
				if (tqinfo)
				{
					int x;
					x = qtn_file_apply_to_fd(tqinfo, s->dst_fd);
					if (x != 0) {
						copyfile_warn("qtn_file_apply_to_fd failed: %s", qtn_error(x));
						if (s->statuscb) {
							int rv;
							s->xattr_name = (char*)XATTR_QUARANTINE_NAME;
							rv = (*s->statuscb)(COPYFILE_COPY_XATTR, COPYFILE_ERR, s, s->src, s->dst, s->ctx);
							s->xattr_name = NULL;
							if (rv == COPYFILE_QUIT) {
								error = s->err = x < 0 ? ENOTSUP : errno;
								goto exit;
							}
						} else {
							error = s->err = x < 0 ? ENOTSUP : errno;
							goto exit;
						}
					}
				}
				if (tqinfo && !s->qinfo)
				{
					qtn_file_free(tqinfo);
				}
			}
			/* Look for ACL data */
			else if (strcmp((char*)entry->name, XATTR_SECURITY_NAME) == 0)
			{
				acl_t acl;
				struct stat sb;
				int retry = 1;
				char *tcp = dataptr;

				if (entry->length == 0) {
					/* Not sure how we got here, but we had one case
					 * where it was 0.  In a normal EA, we can have a 0-byte
					 * payload.  That means nothing in this case, so we'll
					 * simply skip the EA.
					 */
					error = 0;
					goto acl_done;
				}
				/*
				 * acl_from_text() requires a NUL-terminated string.  The ACL EA,
				 * however, may not be NUL-terminated.  So in that case, we need to
				 * copy it to a +1 sized buffer, to ensure it's got a terminated string.
				 */
				if (tcp[entry->length - 1] != 0) {
					char *tmpstr = malloc(entry->length + 1);
					if (tmpstr == NULL) {
						error = -1;
						goto exit;
					}
					// Can't use strlcpy here: tcp is not NUL-terminated!
					memcpy(tmpstr, tcp, entry->length);
					tmpstr[entry->length] = 0;
					acl = acl_from_text(tmpstr);
					free(tmpstr);
				} else {
					acl = acl_from_text(tcp);
				}

				if (acl != NULL)
				{
					filesec_t fsec_tmp;

					if ((fsec_tmp = filesec_init()) == NULL)
						error = -1;
					else if((error = fstatx_np(s->dst_fd, &sb, fsec_tmp)) < 0)
						error = -1;
					else if (filesec_set_property(fsec_tmp, FILESEC_ACL, &acl) < 0)
						error = -1;
					else {
						while (fchmodx_np(s->dst_fd, fsec_tmp) < 0)
						{
			    if (errno == ENOTSUP)
				{
					if (retry && !copyfile_unset_acl(s))
					{
						retry = 0;
						continue;
					}
				}
			    copyfile_warn("setting security information");
			    error = -1;
			    break;
						}
					}
					acl_free(acl);
					filesec_free(fsec_tmp);

				acl_done:
					if (error == -1)
						goto exit;
				}
			}
			/* And, finally, everything else */
			else
			{
				if (s->copyIntent ||
					xattr_preserve_for_intent((char*)entry->name, s->copyIntent) == 1) {
					if (s->statuscb) {
						int rv;
						s->xattr_name = strdup((char*)entry->name);
						s->totalCopied = 0;
						rv = (*s->statuscb)(COPYFILE_COPY_XATTR, COPYFILE_START, s, s->src, s->dst, s->ctx);
						if (s->xattr_name) {
							free(s->xattr_name);
							s->xattr_name = NULL;
						}
						if (rv == COPYFILE_QUIT) {
							s->err = ECANCELED;
							error = -1;
							goto exit;
						}
					}
					//Silently ignore failure to set XATTR_ROOT_INSTALLED_NAME
					int result = fsetxattr(s->dst_fd, (char *)entry->name, dataptr, entry->length, 0, 0);
					int errorcode = errno;
					if (result == -1 && !(errorcode == EPERM &&
										 strcmp((char*)entry->name, XATTR_ROOT_INSTALLED_NAME) == 0)) {
						errno = errorcode;
						if (COPYFILE_VERBOSE & s->flags)
							copyfile_warn("error %d setting attribute %s", errorcode, entry->name);
						if (s->statuscb) {
							int rv;

							s->xattr_name = strdup((char*)entry->name);
							rv = (s->statuscb)(COPYFILE_COPY_XATTR, COPYFILE_ERR, s, s->src, s->dst, s->ctx);
							if (s->xattr_name) {
								free(s->xattr_name);
								s->xattr_name = NULL;
							}
							if (rv == COPYFILE_QUIT) {
								error = -1;
								goto exit;
							}
						} else {
							error = -1;
							goto exit;
						}
					} else if (s->statuscb) {
						int rv;
						errno = errorcode;
						s->xattr_name = strdup((char*)entry->name);
						s->totalCopied = entry->length;
						rv = (*s->statuscb)(COPYFILE_COPY_XATTR, COPYFILE_FINISH, s, s->src, s->dst, s->ctx);
						if (s->xattr_name) {
							free(s->xattr_name);
							s->xattr_name = NULL;
						}
						if (rv == COPYFILE_QUIT) {
							error = -1;
							s->err = ECANCELED;
							goto exit;
						}
					} else {
						errno = errorcode;
					}
				}
			}
			if (dataptr) {
				free(dataptr);
				dataptr = NULL;
			}
			entry = ATTR_NEXT(entry);
		}
	}

	/*
	 * Extract the Finder Info.
	 */
	if (adhdr->entries[0].offset > (hdrsize - sizeof(emptyfinfo))) {
		error = -1;
		goto exit;
	}

	if (bcmp((u_int8_t*)buffer + adhdr->entries[0].offset, emptyfinfo, sizeof(emptyfinfo)) != 0)
	{
		uint16_t *fFlags;
		uint8_t *newFinfo;
		enum { kFinderInvisibleMask = 1 << 14 };

		newFinfo = (u_int8_t*)buffer + adhdr->entries[0].offset;
		fFlags = (uint16_t*)&newFinfo[8];
		copyfile_debug(3, " extracting \"%s\" (32 bytes)", XATTR_FINDERINFO_NAME);
		if (s->statuscb) {
			int rv;
			s->xattr_name = (char*)XATTR_FINDERINFO_NAME;
			rv = (*s->statuscb)(COPYFILE_COPY_XATTR, COPYFILE_START, s, s->src, s->dst, s->ctx);
			s->xattr_name = NULL;
			if (rv == COPYFILE_QUIT) {
				error = -1;
				s->err = ECANCELED;
				goto exit;
			} else if (rv == COPYFILE_SKIP) {
				goto skip_fi;
			}
		}
		error = fsetxattr(s->dst_fd, XATTR_FINDERINFO_NAME, (u_int8_t*)buffer + adhdr->entries[0].offset, sizeof(emptyfinfo), 0, 0);
		if (error) {
			if (s->statuscb) {
				int rv;
				s->xattr_name = (char *)XATTR_FINDERINFO_NAME;
				rv = (*s->statuscb)(COPYFILE_COPY_XATTR, COPYFILE_ERR, s, s->src, s->dst, s->ctx);
				s->xattr_name = NULL;
				if (rv == COPYFILE_QUIT) {
					error = -1;
					s->err = ECANCELED;
					goto exit;
				}
			}
			goto exit;
		} else if (s->statuscb) {
			int rv;
			s->xattr_name = (char *)XATTR_FINDERINFO_NAME;
			rv = (*s->statuscb)(COPYFILE_COPY_XATTR, COPYFILE_FINISH, s, s->src, s->dst, s->ctx);
			s->xattr_name = NULL;
			if (rv == COPYFILE_QUIT) {
				error = -1;
				s->err = ECANCELED;
				goto exit;
			}
		}
		if (SWAP16(*fFlags) & kFinderInvisibleMask)
			s->internal_flags |= cfMakeFileInvisible;
	}
skip_fi:

	/*
	 * Extract the Resource Fork.
	 */
	if (adhdr->entries[1].type == AD_RESOURCE &&
		adhdr->entries[1].length > 0)
	{
		void * rsrcforkdata = NULL;
		size_t length;
		off_t offset;
		struct stat sb;
		struct attrlist attrlist;
		struct {
			/* Order of these structs matters for setattrlist. */
			struct timespec mod_time;
			struct timespec acc_time;
		} ma_times;

		length = adhdr->entries[1].length;
		offset = adhdr->entries[1].offset;
		rsrcforkdata = malloc(length);

		if (rsrcforkdata == NULL) {
			copyfile_debug(1, "could not allocate %zu bytes for rsrcforkdata",
						   length);
			error = -1;
			goto bad;
		}

		if (fstat(s->dst_fd, &sb) < 0)
		{
			copyfile_debug(1, "couldn't stat destination file");
			error = -1;
			goto bad;
		}

		bytes = pread(s->src_fd, rsrcforkdata, length, offset);
		if (bytes < (ssize_t)length)
		{
			if (bytes == -1)
			{
				copyfile_debug(1, "couldn't read resource fork");
			}
			else
			{
				copyfile_debug(1,
							   "couldn't read resource fork (only read %d bytes of %d)",
							   (int)bytes, (int)length);
			}
			error = -1;
			goto bad;
		}
		if (s->statuscb) {
			int rv;
			s->xattr_name = (char *)XATTR_RESOURCEFORK_NAME;
			rv = (*s->statuscb)(COPYFILE_COPY_XATTR, COPYFILE_START, s, s->src, s->dst, s->ctx);
			s->xattr_name = NULL;
			if (rv == COPYFILE_QUIT) {
				error = -1;
				s->err = ECANCELED;
				if (rsrcforkdata)
					free(rsrcforkdata);
				goto exit;
			} else if (rv == COPYFILE_SKIP) {
				goto bad;
			}
		}
		error = fsetxattr(s->dst_fd, XATTR_RESOURCEFORK_NAME, rsrcforkdata, bytes, 0, 0);
		if (error)
		{
			/*
			 * For filesystems that do not natively support named attributes,
			 * the kernel creates an AppleDouble file that -- for compatabilty
			 * reasons -- has a resource fork containing nothing but a rsrcfork_header_t
			 * structure that says there are no resources.  So, if fsetxattr has
			 * failed, and the resource fork is that empty structure, *and* the
			 * target file is a directory, then we do nothing with it.
			 */
			if ((bytes == sizeof(rsrcfork_header_t)) &&
				((sb.st_mode & S_IFMT) == S_IFDIR)  &&
				(memcmp(rsrcforkdata, &empty_rsrcfork_header, bytes) == 0)) {
				copyfile_debug(2, "not setting empty resource fork on directory");
				error = errno = 0;
				goto bad;
			}
			if (s->statuscb) {
				int rv;
				s->xattr_name = (char *)XATTR_RESOURCEFORK_NAME;
				rv = (*s->statuscb)(COPYFILE_COPY_XATTR, COPYFILE_ERR, s, s->src, s->dst, s->ctx);
				s->xattr_name = NULL;
				if (rv == COPYFILE_CONTINUE) {
					error = errno = 0;
					goto bad;
				}
			}
			copyfile_debug(1, "error %d setting resource fork attribute", error);
			error = -1;
			goto bad;
		} else if (s->statuscb) {
			int rv;
			s->xattr_name = (char *)XATTR_RESOURCEFORK_NAME;
			rv = (*s->statuscb)(COPYFILE_COPY_XATTR, COPYFILE_FINISH, s, s->src, s->dst, s->ctx);
			s->xattr_name = NULL;
			if (rv == COPYFILE_QUIT) {
				error = -1;
				s->err = ECANCELED;
				if (rsrcforkdata)
					free(rsrcforkdata);
				goto exit;
			}
		}
		copyfile_debug(3, "extracting \"%s\" (%d bytes)",
					   XATTR_RESOURCEFORK_NAME, (int)length);

		if (!(s->flags & COPYFILE_STAT))
		{
			/* Try to set m/atimes using setattrlist(), for nanosecond precision. */
			memset(&attrlist, 0, sizeof(attrlist));
			attrlist.bitmapcount = ATTR_BIT_MAP_COUNT;
			attrlist.commonattr = ATTR_CMN_MODTIME | ATTR_CMN_ACCTIME;
			ma_times.mod_time = sb.st_mtimespec;
			ma_times.acc_time = sb.st_atimespec;
			if (fsetattrlist(s->dst_fd, &attrlist, &ma_times, sizeof(ma_times), 0) != 0) {
				copyfile_warn("%s: set times", s->dst ? s->dst : "(null dst)");
			}
		}
	bad:
		if (rsrcforkdata)
			free(rsrcforkdata);
	}

	if (COPYFILE_STAT & s->flags)
	{
		error = copyfile_stat(s);
	}
exit:
	if (buffer) free(buffer);
	if (dataptr) free(dataptr);
	return error;
}

static int copyfile_pack_quarantine(copyfile_state_t s, void **buf, ssize_t *len)
{
	int ret = 0;
	char qbuf[QTN_SERIALIZED_DATA_MAX];
	size_t qlen = sizeof(qbuf);

	if (s->qinfo == NULL)
	{
		ret = -1;
		goto done;
	}

	if (qtn_file_to_data(s->qinfo, qbuf, &qlen) != 0)
	{
		ret = -1;
		goto done;
	}

	*buf = malloc(qlen);
	if (*buf)
	{
		memcpy(*buf, qbuf, qlen);
		*len = qlen;
	}
done:
	return ret;
}

static int copyfile_pack_acl(copyfile_state_t s, void **buf, ssize_t *len)
{
	int ret = 0;
	acl_t acl = NULL;
	char *acl_text;

	if (filesec_get_property(s->fsec, FILESEC_ACL, &acl) < 0)
	{
		if (errno != ENOENT)
		{
			ret = -1;
			if (COPYFILE_VERBOSE & s->flags)
				copyfile_warn("getting acl");
		}
		*len = 0;
		goto exit;
	}

	if ((acl_text = acl_to_text(acl, len)) != NULL)
	{
		/*
		 * acl_to_text() doesn't include the NUL at the endo
		 * in it's count (*len).  It does, however, promise to
		 * return a valid C string, so we need to up the count
		 * by 1.
		 */
		*len = *len + 1;
		*buf = malloc(*len);
		if (*buf)
			memcpy(*buf, acl_text, *len);
		else
			*len = 0;
		acl_free(acl_text);
	}
	copyfile_debug(2, "copied acl (%ld) %p", *len, *buf);
exit:
	if (acl)
		acl_free(acl);
	return ret;
}

static int copyfile_pack_rsrcfork(copyfile_state_t s, attr_header_t *filehdr)
{
	ssize_t datasize;
	char *databuf = NULL;
	int ret = 0;

	/*
	 * XXX
	 * do COPYFILE_COPY_XATTR here; no need to
	 * the work if we want to skip.
	 */

	if (s->statuscb)
	{
		int rv;

		s->xattr_name = (char*)XATTR_RESOURCEFORK_NAME;

		rv = (*s->statuscb)(COPYFILE_COPY_XATTR, COPYFILE_START, s, s->src, s->dst, s->ctx);
		s->xattr_name = NULL;
		if (rv == COPYFILE_SKIP) {
			ret = 0;
			goto done;
		}
		if (rv == COPYFILE_QUIT) {
			ret = -1;
			s->err = ECANCELED;
			goto done;
		}
	}
	/* Get the resource fork size */
	if ((datasize = fgetxattr(s->src_fd, XATTR_RESOURCEFORK_NAME, NULL, 0, 0, 0)) < 0)
	{
		if (COPYFILE_VERBOSE & s->flags)
			copyfile_warn("skipping attr \"%s\" due to error %d", XATTR_RESOURCEFORK_NAME, errno);
		return -1;
	}

	if (datasize > INT_MAX) {
		s->err = EINVAL;
		ret = -1;
		goto done;
	}

	if (s->statuscb) {
		int rv;
		s->xattr_name = (char*)XATTR_RESOURCEFORK_NAME;

		s->totalCopied = 0;
		rv = (*s->statuscb)(COPYFILE_COPY_XATTR, COPYFILE_PROGRESS, s, s->src, s->dst, s->ctx);
		s->xattr_name = NULL;
		if (rv == COPYFILE_QUIT) {
			s->err = ECANCELED;
			ret = -1;
			goto done;
		}
	}
	if ((databuf = malloc(datasize)) == NULL)
	{
		copyfile_warn("malloc");
		ret = -1;
		goto done;
	}

	if (fgetxattr(s->src_fd, XATTR_RESOURCEFORK_NAME, databuf, datasize, 0, 0) != datasize)
	{
		if (COPYFILE_VERBOSE & s->flags)
			copyfile_warn("couldn't read entire resource fork");
		ret = -1;
		goto done;
	}

	/* Write the resource fork to disk. */
	if (pwrite(s->dst_fd, databuf, datasize, filehdr->appledouble.entries[1].offset) != datasize)
	{
		if (COPYFILE_VERBOSE & s->flags)
			copyfile_warn("couldn't write resource fork");
	}
	if (s->statuscb)
	{
		int rv;
		rv = (*s->statuscb)(COPYFILE_COPY_XATTR, COPYFILE_FINISH, s, s->src, s->dst, s->ctx);
		if (rv == COPYFILE_QUIT) {
			ret = -1;
			goto done;
		}
	}
	copyfile_debug(3, "copied %zd bytes of \"%s\" data @ offset 0x%08x",
				   datasize, XATTR_RESOURCEFORK_NAME, filehdr->appledouble.entries[1].offset);
	filehdr->appledouble.entries[1].length = (u_int32_t)datasize;

done:
	if (ret == -1 && s->statuscb)
	{
		int rv;
		rv = (*s->statuscb)(COPYFILE_COPY_XATTR, COPYFILE_ERR, s, s->src, s->dst, s->ctx);
		if (rv == COPYFILE_CONTINUE)
			ret = 0;
	}
	if (s->xattr_name) {
		s->xattr_name = NULL;
	}
	if (databuf)
		free(databuf);

	/*
	 * XXX
	 * Do status callback here
	 * If ret == -1, then error callback
	 */
	return ret;
}

/*
 * The opposite of copyfile_unpack(), obviously.
 */
static int copyfile_pack(copyfile_state_t s)
{
	char *attrnamebuf = NULL, *endnamebuf;
	void *databuf = NULL;
	attr_header_t *filehdr, *endfilehdr;
	attr_entry_t *entry;
	ssize_t listsize = 0;
	char *nameptr;
	size_t namelen;
	size_t entrylen;
	ssize_t datasize;
	size_t offset = 0;
	int hasrsrcfork = 0;
	int error = 0;
	int seenq = 0;	// Have we seen any quarantine info already?

	filehdr = (attr_header_t *) calloc(1, ATTR_MAX_HDR_SIZE);

	if (filehdr == NULL) {
		error = -1;
		goto exit;
	} else {
		endfilehdr = (attr_header_t*)(((char*)filehdr) + ATTR_MAX_HDR_SIZE);
	}

	attrnamebuf = calloc(1, ATTR_MAX_HDR_SIZE);
	if (attrnamebuf == NULL) {
		error = -1;
		goto exit;
	} else {
		endnamebuf = ((char*)attrnamebuf) + ATTR_MAX_HDR_SIZE;
	}

	/*
	 * Fill in the Apple Double Header defaults.
	 */
	filehdr->appledouble.magic              = ADH_MAGIC;
	filehdr->appledouble.version            = ADH_VERSION;
	filehdr->appledouble.numEntries         = 2;
	filehdr->appledouble.entries[0].type    = AD_FINDERINFO;
	filehdr->appledouble.entries[0].offset  = (u_int32_t)__builtin_offsetof(apple_double_header_t, finfo);
	filehdr->appledouble.entries[0].length  = FINDERINFOSIZE;
	filehdr->appledouble.entries[1].type    = AD_RESOURCE;
	filehdr->appledouble.entries[1].offset  = (u_int32_t)__builtin_offsetof(apple_double_header_t, pad);
	filehdr->appledouble.entries[1].length  = 0;
	bcopy(ADH_MACOSX, filehdr->appledouble.filler, sizeof(filehdr->appledouble.filler));

	/*
	 * Fill in the initial Attribute Header.
	 */
	filehdr->magic       = ATTR_HDR_MAGIC;
	filehdr->debug_tag   = 0;
	filehdr->data_start  = (u_int32_t)sizeof(attr_header_t);

	/*
	 * Collect the attribute names.
	 */
	entry = (attr_entry_t *)((char *)filehdr + sizeof(attr_header_t));

	/*
	 * Test if there are acls to copy
	 */
	if (COPYFILE_ACL & s->flags)
	{
		acl_t temp_acl = NULL;
		if (filesec_get_property(s->fsec, FILESEC_ACL, &temp_acl) < 0)
		{
			copyfile_debug(2, "no acl entries found (errno = %d)", errno);
		} else
		{
			offset = strlen(XATTR_SECURITY_NAME) + 1;
			strcpy(attrnamebuf, XATTR_SECURITY_NAME);
			endnamebuf = attrnamebuf + offset;
		}
		if (temp_acl)
			acl_free(temp_acl);
	}

	if (COPYFILE_XATTR & s->flags)
	{
		ssize_t left = ATTR_MAX_HDR_SIZE - offset;
		if ((listsize = flistxattr(s->src_fd, attrnamebuf + offset, left, 0)) <= 0)
		{
			copyfile_debug(2, "no extended attributes found (%d)", errno);
		}
		if (listsize > left)
		{
			copyfile_debug(1, "extended attribute list too long");
			listsize = left;
		}

		endnamebuf = attrnamebuf + offset + (listsize > 0 ? listsize : 0);
		if (endnamebuf > (attrnamebuf + ATTR_MAX_HDR_SIZE)) {
			error = -1;
			goto exit;
		}

		if (listsize > 0)
			sort_xattrname_list(attrnamebuf, endnamebuf - attrnamebuf);

		for (nameptr = attrnamebuf; nameptr < endnamebuf; nameptr += namelen)
		{
			namelen = strlen(nameptr) + 1;
			/* Skip over FinderInfo or Resource Fork names */
			if (strcmp(nameptr, XATTR_FINDERINFO_NAME) == 0 ||
				strcmp(nameptr, XATTR_RESOURCEFORK_NAME) == 0) {
				continue;
			}
			if (strcmp(nameptr, XATTR_QUARANTINE_NAME) == 0) {
				seenq = 1;
			}

			/* The system should prevent this from happening, but... */
			if (namelen > XATTR_MAXNAMELEN + 1) {
				namelen = XATTR_MAXNAMELEN + 1;
			}
			if (s->copyIntent &&
				xattr_preserve_for_intent(nameptr, s->copyIntent) == 0) {
				// Skip it
				size_t amt = endnamebuf - (nameptr + namelen);
				memmove(nameptr, nameptr + namelen, amt);
				endnamebuf -= namelen;
				/* Set namelen to 0 so continue doesn't miss names */
				namelen = 0;
				continue;
			}

			if (s->statuscb) {
				int rv;
				char eaname[namelen];
				bcopy(nameptr, eaname, namelen);
				eaname[namelen - 1] = 0; // Just to be sure!
				s->xattr_name = eaname;
				rv = (*s->statuscb)(COPYFILE_COPY_XATTR, COPYFILE_START, s, s->src, s->dst, s->ctx);
				s->xattr_name = NULL;
				if (rv == COPYFILE_QUIT) {
					error = -1;
					s->err = ECANCELED;
					goto exit;
				} else if (rv == COPYFILE_SKIP) {
					size_t amt = endnamebuf - (nameptr + namelen);
					memmove(nameptr, nameptr + namelen, amt);
					endnamebuf -= namelen;
					/* Set namelen to 0 so continue doesn't miss names */
					namelen = 0;
					continue;
				}
			}
			entry->namelen = namelen;
			entry->flags = 0;
			if (nameptr + namelen > endnamebuf) {
				error = -1;
				goto exit;
			}

			bcopy(nameptr, &entry->name[0], namelen);
			copyfile_debug(2, "copied name [%s]", entry->name);

			entrylen = ATTR_ENTRY_LENGTH(namelen);
			entry = (attr_entry_t *)(((char *)entry) + entrylen);

			if ((void*)entry >= (void*)endfilehdr) {
				error = -1;
				goto exit;
			}

			/* Update the attributes header. */
			filehdr->num_attrs++;
			filehdr->data_start += (u_int32_t)entrylen;
		}
	}

	/*
	 * If we have any quarantine data, we always pack it.
	 * But if we've already got it in the EA list, don't put it in again.
	 */
	if (s->qinfo && !seenq)
	{
		ssize_t left = ATTR_MAX_HDR_SIZE - offset;
		/* strlcpy returns number of bytes copied, but we need offset to point to the next byte */
		offset += strlcpy(attrnamebuf + offset, XATTR_QUARANTINE_NAME, left) + 1;
	}

	seenq = 0;
	/*
	 * Collect the attribute data.
	 */
	entry = (attr_entry_t *)((char *)filehdr + sizeof(attr_header_t));

	for (nameptr = attrnamebuf; nameptr < endnamebuf; nameptr += namelen + 1)
	{
		namelen = strlen(nameptr);

		if (strcmp(nameptr, XATTR_SECURITY_NAME) == 0)
			copyfile_pack_acl(s, &databuf, &datasize);
		else if (s->qinfo && strcmp(nameptr, XATTR_QUARANTINE_NAME) == 0)
		{
			copyfile_pack_quarantine(s, &databuf, &datasize);
		}
		/* Check for Finder Info. */
		else if (strcmp(nameptr, XATTR_FINDERINFO_NAME) == 0)
		{
			if (s->statuscb)
			{
				int rv;
				s->xattr_name = (char*)XATTR_FINDERINFO_NAME;
				rv = (*s->statuscb)(COPYFILE_COPY_XATTR, COPYFILE_START, s, s->src, s->dst, s->ctx);
				s->xattr_name = NULL;
				if (rv == COPYFILE_QUIT)
				{
					s->xattr_name = NULL;
					s->err = ECANCELED;
					error = -1;
					goto exit;
				}
				else if (rv == COPYFILE_SKIP)
				{
					s->xattr_name = NULL;
					continue;
				}
				s->totalCopied = 0;
				rv = (*s->statuscb)(COPYFILE_COPY_XATTR, COPYFILE_PROGRESS, s, s->src, s->dst, s->ctx);
				s->xattr_name = NULL;
				if (rv == COPYFILE_QUIT)
				{
					s->err = ECANCELED;
					error = -1;
					goto exit;
				}
			}
			datasize = fgetxattr(s->src_fd, nameptr, (u_int8_t*)filehdr + filehdr->appledouble.entries[0].offset, 32, 0, 0);
			if (datasize < 0)
			{
				if (s->statuscb) {
					int rv;
					s->xattr_name = strdup(nameptr);
					rv = (*s->statuscb)(COPYFILE_COPY_XATTR, COPYFILE_ERR, s, s->src, s->dst, s->ctx);
					if (s->xattr_name) {
						free(s->xattr_name);
						s->xattr_name = NULL;
					}
					if (rv == COPYFILE_QUIT) {
						error = -1;
						goto exit;
					}
				}
				if (COPYFILE_VERBOSE & s->flags)
					copyfile_warn("skipping attr \"%s\" due to error %d", nameptr, errno);
			} else if (datasize != 32)
			{
				if (COPYFILE_VERBOSE & s->flags)
					copyfile_warn("unexpected size (%ld) for \"%s\"", datasize, nameptr);
			} else
			{
				if (COPYFILE_VERBOSE & s->flags)
					copyfile_warn(" copied 32 bytes of \"%s\" data @ offset 0x%08x",
								  XATTR_FINDERINFO_NAME, filehdr->appledouble.entries[0].offset);
				if (s->statuscb) {
					int rv;
					s->xattr_name = strdup(nameptr);
					rv = (*s->statuscb)(COPYFILE_COPY_XATTR, COPYFILE_FINISH, s, s->src, s->dst, s->ctx);
					if (s->xattr_name) {
						free(s->xattr_name);
						s->xattr_name = NULL;
					}
					if (rv == COPYFILE_QUIT) {
						error = -1;
						goto exit;
					}
				}
			}
			continue;  /* finder info doesn't have an attribute entry */
		}
		/* Check for Resource Fork. */
		else if (strcmp(nameptr, XATTR_RESOURCEFORK_NAME) == 0)
		{
			hasrsrcfork = 1;
			continue;
		} else
		{
			/* Just a normal attribute. */
			if (s->statuscb)
			{
				int rv;
				s->xattr_name = strdup(nameptr);
				s->totalCopied = 0;
				rv = (*s->statuscb)(COPYFILE_COPY_XATTR, COPYFILE_PROGRESS, s, s->src, s->dst, s->ctx);
				if (s->xattr_name) {
					free(s->xattr_name);
					s->xattr_name = NULL;
				}
				/*
				 * Due to the nature of the packed file, we can't skip at this point.
				 */
				if (rv == COPYFILE_QUIT)
				{
					s->err = ECANCELED;
					error = -1;
					goto exit;
				}
			}
			datasize = fgetxattr(s->src_fd, nameptr, NULL, 0, 0, 0);
			if (datasize == 0)
				goto next;
			if (datasize < 0)
			{
				if (COPYFILE_VERBOSE & s->flags)
					copyfile_warn("skipping attr \"%s\" due to error %d", nameptr, errno);
				if (s->statuscb)
				{
					int rv;
					s->xattr_name = strdup(nameptr);
					rv = (*s->statuscb)(COPYFILE_COPY_XATTR, COPYFILE_ERR, s, s->src, s->dst, s->ctx);
					if (s->xattr_name) {
						free(s->xattr_name);
						s->xattr_name = NULL;
					}
					if (rv == COPYFILE_QUIT)
					{
						s->err = ECANCELED;
						error = -1;
						goto exit;
					}
				}
				goto next;
			}
			if (datasize > XATTR_MAXATTRLEN)
			{
				if (COPYFILE_VERBOSE & s->flags)
					copyfile_warn("skipping attr \"%s\" (too big)", nameptr);
				goto next;
			}
			databuf = malloc(datasize);
			if (databuf == NULL) {
				error = -1;
				continue;
			}
			datasize = fgetxattr(s->src_fd, nameptr, databuf, datasize, 0, 0);
			if (s->statuscb) {
				int rv;
				s->xattr_name = strdup(nameptr);
				rv = (*s->statuscb)(COPYFILE_COPY_XATTR, COPYFILE_FINISH, s, s->src, s->dst, s->ctx);
				if (s->xattr_name) {
					free(s->xattr_name);
					s->xattr_name = NULL;
				}
				if (rv == COPYFILE_QUIT) {
					s->err = ECANCELED;
					error = -1;
					goto exit;
				}
			}
		}

		entry->length = (u_int32_t)datasize;
		entry->offset = filehdr->data_start + filehdr->data_length;

		filehdr->data_length += (u_int32_t)datasize;
#if 0
		/*
		 * >>>  WARNING <<<
		 * This assumes that the data is fits in memory (not
		 * the case when there are lots of attributes or one of
		 * the attributes is very large.
		 */
		if (entry->offset > ATTR_MAX_SIZE ||
			(entry->offset + datasize > ATTR_MAX_SIZE)) {
			error = 1;
		} else {
			bcopy(databuf, (char*)filehdr + entry->offset, datasize);
		}
#else
		if (pwrite(s->dst_fd, databuf, datasize, entry->offset) != datasize) {
			error = 1;
		}
#endif
		free(databuf);

		copyfile_debug(3, "copied %ld bytes of \"%s\" data @ offset 0x%08x", datasize, nameptr, entry->offset);
	next:
		/* bump to next entry */
		entrylen = ATTR_ENTRY_LENGTH(entry->namelen);
		entry = (attr_entry_t *)((char *)entry + entrylen);
	}

	/* Now we know where the resource fork data starts. */
	filehdr->appledouble.entries[1].offset = (filehdr->data_start + filehdr->data_length);

	/* We also know the size of the "Finder Info entry. */
	filehdr->appledouble.entries[0].length =
	filehdr->appledouble.entries[1].offset - filehdr->appledouble.entries[0].offset;

	filehdr->total_size  = filehdr->appledouble.entries[1].offset;

	/* Copy Resource Fork. */
	if (hasrsrcfork && (error = copyfile_pack_rsrcfork(s, filehdr)))
		goto exit;

	/* Write the header to disk. */
	datasize = filehdr->data_start;

	swap_adhdr(&filehdr->appledouble);
	swap_attrhdr(filehdr);
	swap_attrhdr_entries(filehdr);

	if (pwrite(s->dst_fd, filehdr, datasize, 0) != datasize)
	{
		if (COPYFILE_VERBOSE & s->flags)
			copyfile_warn("couldn't write file header");
		error = -1;
		goto exit;
	}
exit:
	if (filehdr) free(filehdr);
	if (attrnamebuf) free(attrnamebuf);

	if (error)
		return error;
	else
		return copyfile_stat(s);
}
