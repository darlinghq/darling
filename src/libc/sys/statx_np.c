/*
 * Copyright (c) 2004, 2006 Apple Computer, Inc. All rights reserved.
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
#include <sys/types.h>
#include <sys/acl.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <string.h>
#include <stdio.h>

#define ACL_MIN_SIZE_HEURISTIC (KAUTH_FILESEC_SIZE(16))

static int	statx_syscall(void *obj, void  *sbptr, void *fsacl, size_t *fsacl_size);
static int	fstatx_syscall(void *obj, void  *sbptr, void *fsacl, size_t *fsacl_size);
static int	lstatx_syscall(void *obj, void  *sbptr, void *fsacl, size_t *fsacl_size);

static int	statx1(void *obj,
		       int (* stat_syscall)(void *obj, void *sbptr, void *fsacl, size_t *fsacl_size),
		       void *sbptr, filesec_t fsec);

/*
 * Stat interfaces.
 */
int
statx_np(const char *path, struct stat *sb, filesec_t fsec)
{
	if (fsec == NULL)
		return(stat(path, sb));
	return(statx1((void *)&path, statx_syscall, (void *)sb, fsec));
}

int
fstatx_np(int fd, struct stat *sb, filesec_t fsec)
{
	if (fsec == NULL)
		return(fstat(fd, sb));
	return(statx1((void *)&fd, fstatx_syscall, (void *)sb, fsec));
}

int
lstatx_np(const char *path, struct stat *sb, filesec_t fsec)
{
	if (fsec == NULL)
		return(lstat(path, sb));
	return(statx1((void *)&path, lstatx_syscall, (void *)sb, fsec));
}


#if __DARWIN_64_BIT_INO_T && !__DARWIN_ONLY_64_BIT_INO_T
int
statx64_np(const char *path, struct stat64 *sb, filesec_t fsec)
{
	return(statx_np(path, (struct stat *)sb, fsec));
}

int
fstatx64_np(int fd, struct stat64 *sb, filesec_t fsec)
{
	return(fstatx_np(fd, (struct stat *)sb, fsec));
}

int
lstatx64_np(const char *path, struct stat64 *sb, filesec_t fsec)
{
	return(lstatx_np(path, (struct stat *)sb, fsec));
}
#endif /* __DARWIN_64_BIT_INO_T && !__DARWIN_ONLY_64_BIT_INO_T */

/*
 * Stat syscalls
 */
#if __DARWIN_64_BIT_INO_T
extern int __fstat64_extended(int, struct stat *, void *, size_t *);
extern int __lstat64_extended(const char *, struct stat *, void *, size_t *);
extern int __stat64_extended(const char *, struct stat *, void *, size_t *);
#else /* !__DARWIN_64_BIT_INO_T */
extern int __fstat_extended(int, struct stat *, void *, size_t *);
extern int __lstat_extended(const char *, struct stat *, void *, size_t *);
extern int __stat_extended(const char *, struct stat *, void *, size_t *);
#endif /* __DARWIN_64_BIT_INO_T */

static int
statx_syscall(void *obj, void  *sb, void *fsacl, size_t *fsacl_size)
{
	const char *path = *(const char **)obj;
#if __DARWIN_64_BIT_INO_T
	return(__stat64_extended(path, (struct stat *)sb, fsacl, fsacl_size));
#else /* !__DARWIN_64_BIT_INO_T */
	return(__stat_extended(path, (struct stat *)sb, fsacl, fsacl_size));
#endif /* __DARWIN_64_BIT_INO_T */
}

static int
fstatx_syscall(void *obj, void  *sb, void *fsacl, size_t *fsacl_size)
{
	int fd = *(int *)obj;
#if __DARWIN_64_BIT_INO_T
	return(__fstat64_extended(fd, (struct stat *)sb, fsacl, fsacl_size));
#else /* !__DARWIN_64_BIT_INO_T */
	return(__fstat_extended(fd, (struct stat *)sb, fsacl, fsacl_size));
#endif /* __DARWIN_64_BIT_INO_T */
}

static int
lstatx_syscall(void *obj, void  *sb, void *fsacl, size_t *fsacl_size)
{
	const char *path = *(const char **)obj;
#if __DARWIN_64_BIT_INO_T
	return(__lstat64_extended(path, (struct stat *)sb, fsacl, fsacl_size));
#else /* !__DARWIN_64_BIT_INO_T */
	return(__lstat_extended(path, (struct stat *)sb, fsacl, fsacl_size));
#endif /* __DARWIN_64_BIT_INO_T */
}

/*
 * Stat internals
 */
static int
statx1(void *obj,
    int (* stat_syscall)(void *obj, void *sbptr, void *fsacl, size_t *fsacl_size),
    void  *sbptr, filesec_t fsec)
{
	kauth_filesec_t fsacl, ofsacl;
	size_t fsacl_size, buffer_size;
	int error;
	struct stat * sb = (struct stat *)0;

	fsacl = NULL;
	error = 0;
	
	sb = (struct stat *)sbptr;
		
	/*
	 * Allocate an initial buffer.
	 */
	if ((fsacl = malloc(ACL_MIN_SIZE_HEURISTIC)) == NULL) {
		error = ENOMEM;
		goto out;
	}		
	buffer_size = ACL_MIN_SIZE_HEURISTIC;

	/*
	 * Loop until we have the ACL.
	 */
	for (;;) {
		fsacl_size = buffer_size;

		if ((error = stat_syscall(obj, sbptr, fsacl, &fsacl_size)) != 0)
			goto out;

		/*
		 * No error, did we get the ACL?
		 */
		if (fsacl_size <= buffer_size)
			break;

		/* no, use supplied buffer size plus some padding */
		ofsacl = fsacl;
		fsacl = realloc(fsacl, fsacl_size + sizeof(struct kauth_ace) * 2);
		if (fsacl == NULL) {
			fsacl = ofsacl;
			errno = ENOMEM;
			goto out;
		}
		buffer_size = fsacl_size;
	}
	
	/* populate filesec with values from stat */
	filesec_set_property(fsec, FILESEC_OWNER, &(sb->st_uid));
	filesec_set_property(fsec, FILESEC_GROUP, &(sb->st_gid));
	filesec_set_property(fsec, FILESEC_MODE, &(sb->st_mode));

	/* if we got a kauth_filesec, take values from there too */
	if (fsacl_size >= sizeof(struct kauth_filesec)) {
		filesec_set_property(fsec, FILESEC_UUID, &fsacl->fsec_owner);
		filesec_set_property(fsec, FILESEC_GRPUUID, &fsacl->fsec_group);
		
		/* check to see whether there's actually an ACL here */
		if (fsacl->fsec_acl.acl_entrycount != KAUTH_FILESEC_NOACL) {
		    filesec_set_property(fsec, FILESEC_ACL_ALLOCSIZE, &fsacl_size);
		    filesec_set_property(fsec, FILESEC_ACL_RAW, &fsacl);
		    fsacl = NULL;	/* avoid freeing it below */
		} else {
		    filesec_set_property(fsec, FILESEC_ACL_ALLOCSIZE, NULL);
		    filesec_set_property(fsec, FILESEC_ACL_RAW, NULL);
		}
	} else {
		filesec_set_property(fsec, FILESEC_UUID, NULL);
		filesec_set_property(fsec, FILESEC_GRPUUID, NULL);
		filesec_set_property(fsec, FILESEC_ACL_ALLOCSIZE, NULL);
		filesec_set_property(fsec, FILESEC_ACL_RAW, NULL);
	}
out:
	if (fsacl != NULL)
		free(fsacl);
	return(error);
}
