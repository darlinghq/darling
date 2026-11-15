/*
This file is part of Darling.

Copyright (C) 2026 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * ditto: copy directory hierarchies, create and extract archives.
 *
 * Implements the interface documented in ditto(1). Metadata (extended
 * attributes, resource forks, ACLs, AppleDouble packing) goes through
 * copyfile(3), archives through libarchive, BOM listings are read with
 * lsbom(8).
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/xattr.h>
#include <mach-o/fat.h>
#include <mach-o/loader.h>
#include <mach/machine.h>
#include <arpa/inet.h>
#include <archive.h>
#include <archive_entry.h>
#include <copyfile.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <readpassphrase.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifndef FAT_MAGIC_64
#define FAT_MAGIC_64 0xcafebabf
#define FAT_CIGAM_64 0xbfbafeca
#endif

#define USAGE_LINE "Usage: ditto [ <options> ] src [ ... src ] dst\n"
#define APPLEDOUBLE_MAGIC 0x00051607u
#define SEQUESTER_DIR "__MACOSX"

static struct {
	int verbose;		/* 1: -v, 2: -V */
	int xdev;
	int create, extract, gzip, bzip2, zip;
	int keep_parent, sequester;
	int rsrc, extattr, acl, qtn;
	int rsrc_set, extattr_set, acl_set, qtn_set;
	int atomic;
	int level;		/* zlib level, -1: library default */
	int password;
	const char *bom;
	const char *arch[16];
	int narch;
} O = { .rsrc = 1, .extattr = 1, .acl = 1, .qtn = 1, .atomic = 1, .level = -1 };

static int failed;

static void
verr(const char *fmt, va_list ap)
{
	fputs("ditto: ", stderr);
	vfprintf(stderr, fmt, ap);
	fputc('\n', stderr);
}

static void
err(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	verr(fmt, ap);
	va_end(ap);
	failed = 1;
}

static void
err_errno(const char *path, int e)
{
	err("%s: %s", path, strerror(e));
}

static void
bad_usage(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	verr(fmt, ap);
	va_end(ap);
	fputs(USAGE_LINE, stderr);
	exit(1);
}

static void *
xmalloc(size_t n)
{
	void *p = malloc(n ? n : 1);

	if (p == NULL) {
		fputs("ditto: out of memory\n", stderr);
		exit(1);
	}
	return p;
}

static void *
xrealloc(void *old, size_t n)
{
	void *p = realloc(old, n ? n : 1);

	if (p == NULL) {
		fputs("ditto: out of memory\n", stderr);
		exit(1);
	}
	return p;
}

static char *
xstrdup(const char *s)
{
	char *p = xmalloc(strlen(s) + 1);

	strcpy(p, s);
	return p;
}

/* a + "/" + b, without doubling an existing trailing slash of a */
static char *
join(const char *a, const char *b)
{
	size_t la = strlen(a);
	char *p = xmalloc(la + strlen(b) + 2);

	memcpy(p, a, la);
	if (la == 0 || a[la - 1] != '/')
		p[la++] = '/';
	strcpy(p + la, b);
	return p;
}

/* Final path component of path, ignoring trailing slashes. */
static char *
base_name(const char *path)
{
	size_t len = strlen(path), start;
	char *p;

	while (len > 1 && path[len - 1] == '/')
		len--;
	start = len;
	while (start > 0 && path[start - 1] != '/')
		start--;
	p = xmalloc(len - start + 1);
	memcpy(p, path + start, len - start);
	p[len - start] = 0;
	return p;
}

static void
vlog(int level, const char *fmt, ...)
{
	va_list ap;

	if (O.verbose < level)
		return;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

static void
usage(FILE *f)
{
	fputs(USAGE_LINE
	    "\n"
	    "    <options> are any of:\n"
	    "    -h, --help                     print full usage\n"
	    "    -v                             print a line for each source directory copied\n"
	    "    -V                             print a line for every file copied\n"
	    "    -X                             do not descend into directories on another device\n"
	    "\n"
	    "    -c                             create an archive at dst (CPIO unless -k)\n"
	    "    -x                             the sources are archives\n"
	    "    -z                             gzip-compress a CPIO archive\n"
	    "    -j                             bzip2-compress a CPIO archive\n"
	    "    -k                             use the PKZip format\n"
	    "    --keepParent                   embed the name of the source directory in the archive\n"
	    "    --arch archVal                 thin fat Mach-O files to archVal\n"
	    "                                   (may be given more than once)\n"
	    "    --bom bomFile                  copy only the paths present in bomFile\n"
	    "\n"
	    "    --rsrc, --norsrc               preserve / drop resource forks and HFS meta-data\n"
	    "    --extattr, --noextattr         preserve / drop extended attributes\n"
	    "    --qtn, --noqtn                 preserve / drop quarantine information\n"
	    "    --acl, --noacl                 preserve / drop ACLs\n"
	    "\n"
	    "    --sequesterRsrc                store resources of a PKZip archive in __MACOSX\n"
	    "    --zlibCompressionLevel <num>   PKZip compression level, 0 to 9\n"
	    "    --password                     prompt for the password of an encrypted PKZip archive\n"
	    "    --nonAtomicCopies              replace existing files in place\n"
	    "\n"
	    "    Accepted without effect: --hfsCompression, --nohfsCompression,\n"
	    "    --preserveHFSCompression, --nopreserveHFSCompression, --nocache,\n"
	    "    --persistRootless, --nopersistRootless, --segmentLargeFiles, --clone,\n"
	    "    --noclone, --keepBinaries, --keepBinariesList <path>,\n"
	    "    --keepBinariesPattern <regex>, --lang <lang>, --outBom <bom>,\n"
	    "    --option <key=value>\n"
	    "\n"
	    "Environment Variables:\n"
	    "    DITTONORSRC                    same as --norsrc --noextattr --noacl --noqtn\n"
	    "                                   for the options not given explicitly\n",
	    f);
}

/* ---- metadata helpers ------------------------------------------------ */

static int
is_rsrc_xattr(const char *name)
{
	return strcmp(name, "com.apple.ResourceFork") == 0 ||
	    strcmp(name, "com.apple.FinderInfo") == 0;
}

/*
 * Drop from path the extended attributes the options say not to keep:
 * resource fork and Finder info (--norsrc), the other attributes
 * (--noextattr) and the quarantine attribute (--noqtn).
 */
static void
filter_xattrs(const char *path)
{
	ssize_t len;
	char *list, *p;

	if (O.rsrc && O.extattr && O.qtn)
		return;
	len = listxattr(path, NULL, 0, XATTR_NOFOLLOW);
	if (len <= 0)
		return;
	list = xmalloc((size_t)len);
	len = listxattr(path, list, (size_t)len, XATTR_NOFOLLOW);
	for (p = list; len > 0 && p < list + len; p += strlen(p) + 1) {
		int keep;

		if (strcmp(p, "com.apple.quarantine") == 0)
			/*
			 * ditto(1) says --noqtn drops quarantine information, but
			 * macOS (26.7.3 and 10.14.4, observed) keeps an explicitly
			 * set com.apple.quarantine attribute with --noqtn alone;
			 * it goes away only with the other attributes (--norsrc,
			 * --noextattr). Behave like macOS.
			 */
			keep = O.extattr;
		else if (is_rsrc_xattr(p))
			keep = O.rsrc;
		else
			keep = O.extattr;
		if (!keep)
			removexattr(path, p, XATTR_NOFOLLOW);
	}
	free(list);
}

static copyfile_flags_t
meta_flags(void)
{
	copyfile_flags_t f = COPYFILE_STAT | COPYFILE_NOFOLLOW;

	if (O.rsrc || O.extattr || O.qtn)
		f |= COPYFILE_XATTR;
	if (O.acl)
		f |= COPYFILE_ACL;
	return f;
}

/* A filesystem without attribute or ACL support is not an error. */
static int
copyfile_failed(void)
{
	return errno != ENOTSUP && errno != EOPNOTSUPP;
}

/* Copy the metadata of src (mode, owner, times, attributes, ACL) to dst. */
static int
copy_metadata(const char *src, const char *dst)
{
	if (copyfile(src, dst, NULL, meta_flags()) < 0 && copyfile_failed())
		return -1;
	filter_xattrs(dst);
	return 0;
}

/* ---- Mach-O thinning --------------------------------------------------- */

static const struct {
	const char *name;
	cpu_type_t type;
	cpu_subtype_t subtype;
} arch_names[] = {
	{ "i386", CPU_TYPE_X86, CPU_SUBTYPE_X86_ALL },
	{ "x86_64", CPU_TYPE_X86_64, CPU_SUBTYPE_X86_64_ALL },
	{ "x86_64h", CPU_TYPE_X86_64, CPU_SUBTYPE_X86_64_H },
	{ "arm64", CPU_TYPE_ARM64, CPU_SUBTYPE_ARM64_ALL },
	{ "arm64e", CPU_TYPE_ARM64, CPU_SUBTYPE_ARM64E },
	{ "arm64_32", CPU_TYPE_ARM64_32, CPU_SUBTYPE_ARM64_32_ALL },
	{ "arm", CPU_TYPE_ARM, CPU_SUBTYPE_ARM_ALL },
	{ "armv6", CPU_TYPE_ARM, CPU_SUBTYPE_ARM_V6 },
	{ "armv7", CPU_TYPE_ARM, CPU_SUBTYPE_ARM_V7 },
	{ "armv7f", CPU_TYPE_ARM, CPU_SUBTYPE_ARM_V7F },
	{ "armv7k", CPU_TYPE_ARM, CPU_SUBTYPE_ARM_V7K },
	{ "armv7s", CPU_TYPE_ARM, CPU_SUBTYPE_ARM_V7S },
	{ "ppc", CPU_TYPE_POWERPC, CPU_SUBTYPE_POWERPC_ALL },
	{ "ppc7400", CPU_TYPE_POWERPC, CPU_SUBTYPE_POWERPC_7400 },
	{ "ppc970", CPU_TYPE_POWERPC, CPU_SUBTYPE_POWERPC_970 },
	{ "ppc64", CPU_TYPE_POWERPC64, CPU_SUBTYPE_POWERPC_ALL },
};
#define NARCH_NAMES (sizeof(arch_names) / sizeof(arch_names[0]))

static int
arch_wanted(uint32_t type, uint32_t subtype)
{
	size_t i;
	int n;

	subtype &= ~(uint32_t)CPU_SUBTYPE_MASK;
	for (n = 0; n < O.narch; n++) {
		for (i = 0; i < NARCH_NAMES; i++) {
			if (strcmp(arch_names[i].name, O.arch[n]) != 0)
				continue;
			if ((uint32_t)arch_names[i].type == type &&
			    ((uint32_t)arch_names[i].subtype & ~(uint32_t)CPU_SUBTYPE_MASK) == subtype)
				return 1;
		}
	}
	return 0;
}

struct slice {
	uint32_t type, subtype, align;
	uint64_t offset, size;
};

#define MAX_SLICES 64

/*
 * Read the slice table of a fat file. Returns the slice count, or 0 if the
 * file is not a (32 or 64 bit) fat file.
 */
static int
read_fat(int fd, struct slice *s)
{
	uint32_t hdr[2];
	uint32_t n, i;
	int is64;

	if (pread(fd, hdr, sizeof(hdr), 0) != (ssize_t)sizeof(hdr))
		return 0;
	if (hdr[0] == FAT_CIGAM)
		is64 = 0;
	else if (hdr[0] == FAT_CIGAM_64)
		is64 = 1;
	else
		return 0;
	n = ntohl(hdr[1]);
	if (n == 0 || n > MAX_SLICES)
		return 0;
	for (i = 0; i < n; i++) {
		uint32_t e[8];
		size_t esz = is64 ? 32 : 20;

		if (pread(fd, e, esz, 8 + (off_t)esz * i) != (ssize_t)esz)
			return 0;
		s[i].type = ntohl(e[0]);
		s[i].subtype = ntohl(e[1]);
		if (is64) {
			s[i].offset = ((uint64_t)ntohl(e[2]) << 32) | ntohl(e[3]);
			s[i].size = ((uint64_t)ntohl(e[4]) << 32) | ntohl(e[5]);
			s[i].align = ntohl(e[6]);
		} else {
			s[i].offset = ntohl(e[2]);
			s[i].size = ntohl(e[3]);
			s[i].align = ntohl(e[4]);
		}
	}
	return (int)n;
}

static int
copy_range(int in, int out, uint64_t off, uint64_t size)
{
	char buf[65536];

	while (size > 0) {
		size_t want = size < sizeof(buf) ? (size_t)size : sizeof(buf);
		ssize_t r = pread(in, buf, want, (off_t)off);

		if (r <= 0)
			return -1;
		if (write(out, buf, (size_t)r) != r)
			return -1;
		off += (uint64_t)r;
		size -= (uint64_t)r;
	}
	return 0;
}

/*
 * If src is a fat file containing some but not all of the wanted
 * architectures' slices, write the thinned file (a plain Mach-O file for one
 * slice, a smaller fat file for several) to dst and return 1. Returns 0 if
 * src is to be copied unchanged, 2 if it is a Mach-O file containing none of
 * the wanted architectures (such a file is not copied), -1 on error.
 */
static int
thin_file(const char *src, const char *dst)
{
	struct slice s[MAX_SLICES];
	int sel[MAX_SLICES], nsel = 0, n, i, in, out, ret = -1, e;

	in = open(src, O_RDONLY);
	if (in < 0)
		return -1;
	n = read_fat(in, s);
	for (i = 0; i < n; i++)
		if (arch_wanted(s[i].type, s[i].subtype))
			sel[nsel++] = i;
	if (n == 0) {
		/* a thin Mach-O file is kept if it is one of the wanted architectures */
		uint32_t h[3];
		int skip = 0;

		if (pread(in, h, sizeof(h), 0) == (ssize_t)sizeof(h)) {
			if (h[0] == MH_MAGIC || h[0] == MH_MAGIC_64)
				skip = !arch_wanted(h[1], h[2]);
			else if (h[0] == MH_CIGAM || h[0] == MH_CIGAM_64)
				skip = !arch_wanted(ntohl(h[1]), ntohl(h[2]));
		}
		close(in);
		return skip ? 2 : 0;
	}
	if (nsel == n) {
		close(in);
		return 0;
	}
	if (nsel == 0) {
		close(in);
		return 2;
	}
	out = open(dst, O_WRONLY | O_CREAT | O_EXCL, 0600);
	if (out < 0) {
		e = errno;
		close(in);
		errno = e;
		return -1;
	}
	if (nsel == 1) {
		ret = copy_range(in, out, s[sel[0]].offset, s[sel[0]].size);
	} else {
		uint32_t hdr[2] = { htonl(FAT_MAGIC), htonl((uint32_t)nsel) };
		uint32_t ent[MAX_SLICES][5];
		uint64_t offs[MAX_SLICES];
		uint64_t pos = 8 + 20 * (uint64_t)nsel, al;
		uint32_t maxalign = 0;

		for (i = 0; i < nsel; i++)
			if (s[sel[i]].align > maxalign)
				maxalign = s[sel[i]].align;
		for (i = 0; i < nsel; i++) {
			struct slice *sl = &s[sel[i]];

			al = (uint64_t)1 << (sl->align < 31 ? sl->align : 31);
			pos = (pos + al - 1) & ~(al - 1);
			offs[i] = pos;
			ent[i][0] = htonl(sl->type);
			ent[i][1] = htonl(sl->subtype);
			ent[i][2] = htonl((uint32_t)pos);
			ent[i][3] = htonl((uint32_t)sl->size);
			ent[i][4] = htonl(sl->align);
			pos += sl->size;
		}
		ret = 0;
		if (pos > 0xffffffffu || write(out, hdr, 8) != 8 ||
		    write(out, ent, 20 * (size_t)nsel) != (ssize_t)(20 * (size_t)nsel))
			ret = -1;
		for (i = 0; ret == 0 && i < nsel; i++) {
			if (lseek(out, (off_t)offs[i], SEEK_SET) < 0 ||
			    copy_range(in, out, s[sel[i]].offset, s[sel[i]].size) < 0)
				ret = -1;
		}
	}
	e = errno;
	if (close(out) < 0 && ret == 0) {
		e = errno;
		ret = -1;
	}
	close(in);
	if (ret < 0)
		unlink(dst);
	errno = e;
	return ret < 0 ? -1 : 1;
}

/* ---- tree copy --------------------------------------------------------- */

struct hl {
	dev_t dev;
	ino_t ino;
	char *dst;
	struct hl *next;
};
static struct hl *hardlinks;
static unsigned tmp_counter;
static dev_t top_dev;
static dev_t dst_top_dev;
static ino_t dst_top_ino;
static int dst_top_valid;

static int
ignored_name(const char *name)
{
	return strncmp(name, ".nfs", 4) == 0 || strncmp(name, ".afpDeleted", 11) == 0;
}

/* A fresh name next to dst, for the atomic replacement of dst. */
static char *
temp_name_for(const char *dst)
{
	char tmp[64];
	char *dir = xstrdup(dst), *slash, *name;

	snprintf(tmp, sizeof(tmp), ".ditto.%d.%u", (int)getpid(), tmp_counter++);
	slash = strrchr(dir, '/');
	if (slash == NULL) {
		free(dir);
		return xstrdup(tmp);
	}
	slash[1] = 0;
	name = join(dir, tmp);
	free(dir);
	return name;
}

/*
 * Create the item at "to" (which does not exist) as a copy of src.
 * Returns 0 on success, 1 if the item is not to be copied, -1 on error.
 */
static int
produce(const char *src, const char *to, const struct stat *sst, const char *link_to)
{
	if (link_to != NULL)
		return link(link_to, to);
	if (S_ISCHR(sst->st_mode) || S_ISBLK(sst->st_mode)) {
		if (mknod(to, sst->st_mode, sst->st_rdev) < 0)
			return -1;
		return copy_metadata(src, to);
	}
	if (O.narch > 0 && S_ISREG(sst->st_mode)) {
		int r = thin_file(src, to);

		if (r < 0)
			return -1;
		if (r == 2)
			return 1;
		if (r == 1)
			return copy_metadata(src, to);
	}
	if (copyfile(src, to, NULL, COPYFILE_DATA | meta_flags()) < 0 && copyfile_failed())
		return -1;
	filter_xattrs(to);
	return 0;
}

/* Copy a file, symlink or device to dst, replacing what is there. */
static int
copy_leaf(const char *src, const char *dst, const struct stat *sst, const char *rel)
{
	struct stat dst_st;
	int exists = lstat(dst, &dst_st) == 0;
	const char *link_to = NULL;
	struct hl *h;
	char *to;
	int r;

	if (exists && S_ISDIR(dst_st.st_mode)) {
		err_errno(dst, EISDIR);
		return -1;
	}
	if (exists && dst_st.st_dev == sst->st_dev && dst_st.st_ino == sst->st_ino) {
		err("%s and %s are identical (not copied).", src, dst);
		return -1;
	}
	vlog(2, "copying %s %s ... \n", S_ISLNK(sst->st_mode) ? "symlink" : "file", rel);
	if (S_ISREG(sst->st_mode) && sst->st_nlink > 1) {
		for (h = hardlinks; h != NULL; h = h->next)
			if (h->dev == sst->st_dev && h->ino == sst->st_ino) {
				link_to = h->dst;
				break;
			}
	}
	if (exists && !O.atomic) {
		if (unlink(dst) < 0) {
			err_errno(dst, errno);
			return -1;
		}
		exists = 0;
	}
	to = exists ? temp_name_for(dst) : xstrdup(dst);
	r = produce(src, to, sst, link_to);
	if (r < 0) {
		err_errno(dst, errno);
		if (exists)
			unlink(to);
		free(to);
		return -1;
	}
	if (r == 1) {
		free(to);
		return 0;
	}
	if (exists && rename(to, dst) < 0) {
		err_errno(dst, errno);
		unlink(to);
		free(to);
		return -1;
	}
	free(to);
	if (link_to != NULL) {
		vlog(2, "hardlinked %s\n", rel);
	} else if (S_ISLNK(sst->st_mode)) {
		vlog(2, "linked %s\n", rel);
	} else {
		if (S_ISREG(sst->st_mode) && sst->st_nlink > 1) {
			h = xmalloc(sizeof(*h));
			h->dev = sst->st_dev;
			h->ino = sst->st_ino;
			h->dst = xstrdup(dst);
			h->next = hardlinks;
			hardlinks = h;
		}
		vlog(2, "%lld bytes for %s\n", (long long)sst->st_size, rel);
	}
	return 0;
}

/* mkdir -p for the parents of path (not path itself). */
static int
make_parents(const char *path)
{
	char *p = xstrdup(path), *s;
	int ret = 0;

	for (s = p + 1; *s; s++) {
		if (*s != '/')
			continue;
		*s = 0;
		if (mkdir(p, 0777) < 0 && errno != EEXIST) {
			err_errno(p, errno);
			ret = -1;
			break;
		}
		*s = '/';
	}
	free(p);
	return ret;
}

static int copy_dir(const char *src, const char *dst, const char *rel);

static void
copy_entry(const char *src, const char *dst, const char *rel)
{
	struct stat st;

	if (lstat(src, &st) < 0) {
		err_errno(src, errno);
		return;
	}
	if (S_ISDIR(st.st_mode)) {
		if (O.xdev && st.st_dev != top_dev)
			return;
		if (dst_top_valid && st.st_dev == dst_top_dev && st.st_ino == dst_top_ino)
			return;		/* the destination lives inside the source */
		copy_dir(src, dst, rel);
	} else if (S_ISREG(st.st_mode) || S_ISLNK(st.st_mode) ||
	    S_ISCHR(st.st_mode) || S_ISBLK(st.st_mode)) {
		copy_leaf(src, dst, &st, rel);
	}
	/* Pipes and sockets are ignored. */
}

/*
 * Merge directory src into dst. A directory that already exists keeps its
 * mode, owner and attributes; a new one takes them from src.
 */
static int
copy_dir(const char *src, const char *dst, const char *rel)
{
	struct stat dst_st;
	DIR *d;
	struct dirent *de;
	int created = 0;

	if (lstat(dst, &dst_st) == 0) {
		if (!S_ISDIR(dst_st.st_mode)) {
			err("%s: %s", dst, strerror(ENOTDIR));
			return -1;
		}
	} else if (errno == ENOENT) {
		if (mkdir(dst, 0700) < 0) {
			err_errno(dst, errno);
			return -1;
		}
		created = 1;
	} else {
		err_errno(dst, errno);
		return -1;
	}
	d = opendir(src);
	if (d == NULL) {
		err_errno(src, errno);
		return -1;
	}
	while ((de = readdir(d)) != NULL) {
		char *s, *t, *r;

		if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0 ||
		    ignored_name(de->d_name))
			continue;
		s = join(src, de->d_name);
		t = join(dst, de->d_name);
		r = join(rel, de->d_name);
		copy_entry(s, t, r);
		free(s);
		free(t);
		free(r);
	}
	closedir(d);
	/* Times and mode last: creating the contents changed the times. */
	if (created && copy_metadata(src, dst) < 0) {
		err_errno(dst, errno);
		return -1;
	}
	return 0;
}

/* Copy one source operand; into_dir: dst names a directory to put files in. */
static void
copy_operand(const char *arg, const char *dst, int into_dir)
{
	char real[PATH_MAX], realdst[PATH_MAX];
	struct stat st, dst_st;
	int dst_exists;

	if (realpath(arg, real) == NULL || stat(real, &st) < 0) {
		err("Cannot get the real path for source '%s'", arg);
		return;
	}
	dst_exists = stat(dst, &dst_st) == 0;
	if (dst_exists && dst_st.st_dev == st.st_dev && dst_st.st_ino == st.st_ino) {
		err("%s and %s are identical (not copied).", real, dst);
		return;
	}
	if (O.verbose)
		fprintf(stderr, O.verbose > 1 ? ">>> Copying %s \n" : "Copying %s \n", arg);
	top_dev = st.st_dev;
	if (S_ISDIR(st.st_mode)) {
		if (dst_exists && !S_ISDIR(dst_st.st_mode)) {
			err("Can't copy directory %s into a file %s.", real, dst);
			return;
		}
		if (make_parents(dst) < 0)
			return;
		if (dst_exists && realpath(dst, realdst) != NULL) {
			/* an existing destination given as a symlink is followed */
			dst = realdst;
			dst_top_dev = dst_st.st_dev;
			dst_top_ino = dst_st.st_ino;
			dst_top_valid = 1;
		} else {
			dst_top_valid = 0;
		}
		copy_dir(real, dst, ".");
	} else {
		char *joined = NULL, *name;
		const char *target = dst;
		struct stat lst;

		if (into_dir) {
			name = base_name(arg);
			joined = join(dst, name);
			free(name);
			target = joined;
		}
		if (make_parents(target) == 0 && lstat(real, &lst) == 0)
			copy_leaf(real, target, &lst, ".");
		free(joined);
	}
}

static void
do_copy(char **srcs, int nsrc, const char *dst)
{
	struct stat st;
	size_t dl = strlen(dst);
	int into_dir = (stat(dst, &st) == 0 && S_ISDIR(st.st_mode)) ||
	    nsrc > 1 || (dl > 0 && dst[dl - 1] == '/');
	int i;

	for (i = 0; i < nsrc; i++)
		copy_operand(srcs[i], dst, into_dir);
}

/* ---- BOM mode --------------------------------------------------------- */

/* The BOM mode accepts only clean relative paths. */
static int
bom_path_valid(const char *rel)
{
	size_t n = strlen(rel);

	if (n == 0 || rel[0] == '/')
		return 0;
	if (strcmp(rel, "..") == 0 || strncmp(rel, "../", 3) == 0)
		return 0;
	if (n >= 3 && strcmp(rel + n - 3, "/..") == 0)
		return 0;
	if (n >= 2 && strcmp(rel + n - 2, "/.") == 0)
		return 0;
	if (strstr(rel, "/../") || strstr(rel, "/./") || strstr(rel, "//"))
		return 0;
	return 1;
}

struct dirmeta {
	char *src, *dst;
	struct dirmeta *next;
};

/* Read the paths of a BOM with lsbom -s. */
static int
bom_read(const char *bom, char ***lines, size_t *nlines)
{
	int fds[2], status;
	pid_t pid;
	FILE *f;
	char *line = NULL;
	size_t cap = 0, n = 0, alloc = 64;
	ssize_t len;

	*lines = xmalloc(alloc * sizeof(char *));
	if (pipe(fds) < 0)
		return -1;
	pid = fork();
	if (pid < 0)
		return -1;
	if (pid == 0) {
		close(fds[0]);
		dup2(fds[1], 1);
		close(fds[1]);
		execlp("lsbom", "lsbom", "-s", bom, (char *)NULL);
		_exit(127);
	}
	close(fds[1]);
	f = fdopen(fds[0], "r");
	while ((len = getline(&line, &cap, f)) >= 0) {
		if (len > 0 && line[len - 1] == '\n')
			line[len - 1] = 0;
		if (n == alloc) {
			alloc *= 2;
			*lines = xrealloc(*lines, alloc * sizeof(char *));
		}
		(*lines)[n++] = xstrdup(line);
	}
	free(line);
	fclose(f);
	if (waitpid(pid, &status, 0) < 0 || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
		return -1;
	*nlines = n;
	return 0;
}

/* True if some leading directory of rel is a symlink in either tree. */
static int
bom_symlinked_parent(const char *src_dir, const char *dst_dir, const char *rel, char **which)
{
	char *parent = xstrdup(rel), *slash = strrchr(parent, '/'), *p;
	int found = 0;

	if (slash == NULL) {
		free(parent);
		return 0;
	}
	*slash = 0;
	for (p = parent; !found; p++) {
		if (*p == '/' || *p == 0) {
			char saved = *p, *ps, *pd;
			struct stat ls;

			*p = 0;
			ps = join(src_dir, parent);
			pd = join(dst_dir, parent);
			if ((lstat(ps, &ls) == 0 && S_ISLNK(ls.st_mode)) ||
			    (lstat(pd, &ls) == 0 && S_ISLNK(ls.st_mode))) {
				found = 1;
				*which = xstrdup(parent);
			}
			free(ps);
			free(pd);
			*p = saved;
			if (saved == 0)
				break;
		}
	}
	free(parent);
	return found;
}

static int
bom_copy(const char *bom, const char *src_arg, const char *dst_arg)
{
	char **lines, *src_dir, *dst_dir;
	size_t n, i, l;
	struct stat st;
	struct dirmeta *dirs = NULL, *dm;

	src_dir = xstrdup(src_arg);
	dst_dir = xstrdup(dst_arg);
	for (l = strlen(src_dir); l > 1 && src_dir[l - 1] == '/'; l--)
		src_dir[l - 1] = 0;
	for (l = strlen(dst_dir); l > 1 && dst_dir[l - 1] == '/'; l--)
		dst_dir[l - 1] = 0;
	if (stat(src_dir, &st) < 0 || !S_ISDIR(st.st_mode)) {
		fprintf(stderr, "ditto: source and destination must be directories\n");
		return 1;
	}
	if (stat(dst_dir, &st) < 0) {
		if (make_parents(dst_dir) < 0 || (mkdir(dst_dir, 0777) < 0 && errno != EEXIST)) {
			err_errno(dst_dir, errno);
			return 1;
		}
	} else if (!S_ISDIR(st.st_mode)) {
		fprintf(stderr, "ditto: source and destination must be directories\n");
		return 1;
	}
	if (access(bom, R_OK) < 0 || bom_read(bom, &lines, &n) < 0) {
		fprintf(stderr, "ditto: cannot read BOM: %s\n", bom);
		return 1;
	}
	for (i = 0; i < n; i++) {
		const char *rel = lines[i];
		char *s, *d, *bad = NULL, *slash, *pd;
		struct stat sst, ds;

		if (strcmp(rel, ".") == 0)
			continue;
		while (strncmp(rel, "./", 2) == 0)
			rel += 2;
		if (!bom_path_valid(rel)) {
			fprintf(stderr, "ditto: invalid BOM path: %s\n", rel);
			return 1;
		}
		s = join(src_dir, rel);
		d = join(dst_dir, rel);
		if (lstat(s, &sst) < 0) {
			fprintf(stderr, "ditto: missing BOM source: %s\n", rel);
			return 1;
		}
		/*
		 * Never traverse a symlink in either tree. A symlink may still
		 * be copied as a leaf, without following its target.
		 */
		if (bom_symlinked_parent(src_dir, dst_dir, rel, &bad)) {
			fprintf(stderr, "ditto: symlinked BOM parent: %s\n", bad);
			return 1;
		}
		pd = xstrdup(d);
		slash = strrchr(pd, '/');
		if (slash != NULL) {
			*slash = 0;
			if (make_parents(pd) < 0 || (mkdir(pd, 0777) < 0 && errno != EEXIST)) {
				if (errno != EEXIST)
					err_errno(pd, errno);
				return 1;
			}
		}
		free(pd);
		if (S_ISDIR(sst.st_mode)) {
			if (lstat(d, &ds) == 0) {
				if (!S_ISDIR(ds.st_mode)) {
					err("%s: %s", d, strerror(ENOTDIR));
					return 1;
				}
			} else {
				if (mkdir(d, 0700) < 0) {
					err_errno(d, errno);
					return 1;
				}
				dm = xmalloc(sizeof(*dm));
				dm->src = s;
				dm->dst = d;
				dm->next = dirs;
				dirs = dm;
				continue;
			}
		} else if (copy_leaf(s, d, &sst, rel) < 0) {
			return 1;
		}
		free(s);
		free(d);
	}
	/* Directory times are set last: creating their contents changed them. */
	for (dm = dirs; dm != NULL; dm = dm->next)
		if (copy_metadata(dm->src, dm->dst) < 0) {
			err_errno(dm->dst, errno);
			return 1;
		}
	return failed;
}

/* ---- archive creation ---------------------------------------------------- */

static struct archive *wa;
static struct archive_entry_linkresolver *resolver;
static int nentries;
static char **seq_dirs;
static size_t nseq_dirs;

static int
write_header_checked(struct archive_entry *e)
{
	if (archive_write_header(wa, e) < ARCHIVE_WARN) {
		err("%s: %s", archive_entry_pathname(e), archive_error_string(wa));
		return -1;
	}
	return 0;
}

/* Emit the directory entry dir (with trailing slash) once. */
static void
emit_sequester_dir(const char *dir)
{
	size_t i;
	struct archive_entry *e;

	for (i = 0; i < nseq_dirs; i++)
		if (strcmp(seq_dirs[i], dir) == 0)
			return;
	seq_dirs = xrealloc(seq_dirs, (nseq_dirs + 1) * sizeof(char *));
	seq_dirs[nseq_dirs++] = xstrdup(dir);
	e = archive_entry_new();
	archive_entry_copy_pathname(e, dir);
	archive_entry_set_filetype(e, AE_IFDIR);
	archive_entry_set_perm(e, 0775);
	archive_entry_set_mtime(e, time(NULL), 0);
	if (write_header_checked(e) == 0)
		nentries++;
	archive_entry_free(e);
}

/*
 * Path of the AppleDouble entry for the archive path name: "._base" next
 * to the item, or under __MACOSX when the resources are sequestered
 * (creating the __MACOSX directory entries as needed).
 */
static char *
appledouble_name(const char *name)
{
	char *n = xstrdup(name), *slash, *base, *dir = NULL, *res;
	size_t l = strlen(n);

	while (l > 1 && n[l - 1] == '/')
		n[--l] = 0;
	slash = strrchr(n, '/');
	if (slash != NULL) {
		*slash = 0;
		dir = n;
		base = slash + 1;
	} else {
		base = n;
	}
	res = xmalloc(strlen(name) + 32);
	if (O.zip && O.sequester) {
		char *acc = xstrdup(SEQUESTER_DIR "/"), *p;

		emit_sequester_dir(acc);
		if (dir != NULL) {
			for (p = dir; *p; ) {
				char *end = strchr(p, '/'), *next;
				size_t cl = end ? (size_t)(end - p) : strlen(p);

				next = xmalloc(strlen(acc) + cl + 2);
				memcpy(next, acc, strlen(acc));
				memcpy(next + strlen(acc), p, cl);
				strcpy(next + strlen(acc) + cl, "/");
				free(acc);
				acc = next;
				emit_sequester_dir(acc);
				p += cl;
				if (*p == '/')
					p++;
			}
		}
		sprintf(res, "%s._%s", acc, base);
		/* the sprintf above needs room for the accumulated path */
		free(acc);
	} else if (dir != NULL) {
		sprintf(res, "%s/._%s", dir, base);
	} else {
		sprintf(res, "._%s", base);
	}
	free(n);
	return res;
}

/* Returns the AppleDouble data for path in *buf, or *len == 0 if none. */
static int
pack_appledouble(const char *path, unsigned char **buf, size_t *len)
{
	copyfile_flags_t f = COPYFILE_NOFOLLOW | COPYFILE_XATTR;
	const char *tdir;
	char *tmp;
	int fd;
	struct stat st;
	ssize_t r;

	*buf = NULL;
	*len = 0;
	if (!O.rsrc)
		return 0;
	if (O.acl)
		f |= COPYFILE_ACL;
	if (copyfile(path, NULL, NULL, f | COPYFILE_CHECK) <= 0)
		return 0;
	tdir = getenv("TMPDIR");
	if (tdir == NULL || *tdir == 0)
		tdir = "/tmp";
	tmp = join(tdir, "ditto.ad.XXXXXX");
	fd = mkstemp(tmp);
	if (fd < 0) {
		free(tmp);
		return -1;
	}
	if (copyfile(path, tmp, NULL, f | COPYFILE_PACK) < 0 ||
	    fstat(fd, &st) < 0 || st.st_size <= 0) {
		close(fd);
		unlink(tmp);
		free(tmp);
		return 0;
	}
	*buf = xmalloc((size_t)st.st_size);
	r = pread(fd, *buf, (size_t)st.st_size, 0);
	close(fd);
	unlink(tmp);
	free(tmp);
	if (r != st.st_size) {
		free(*buf);
		*buf = NULL;
		return -1;
	}
	*len = (size_t)st.st_size;
	return 0;
}

static void
write_appledouble(const char *srcpath, const char *arcname, const struct stat *st)
{
	unsigned char *buf;
	size_t len;
	struct archive_entry *e;
	char *name;

	if (strcmp(arcname, ".") == 0)
		return;
	if (pack_appledouble(srcpath, &buf, &len) < 0) {
		err("%s: cannot pack extended attributes", srcpath);
		return;
	}
	if (len == 0)
		return;
	name = appledouble_name(arcname);
	e = archive_entry_new();
	archive_entry_copy_pathname(e, name);
	archive_entry_set_filetype(e, AE_IFREG);
	archive_entry_set_perm(e, st->st_mode & 0777);
	archive_entry_set_mtime(e, st->st_mtime, 0);
	archive_entry_set_size(e, (la_int64_t)len);
	if (write_header_checked(e) == 0) {
		if (archive_write_data(wa, buf, len) < 0)
			err("%s: %s", name, archive_error_string(wa));
		nentries++;
	}
	archive_entry_free(e);
	free(name);
	free(buf);
}

static void
write_entry(struct archive_entry *e)
{
	const char *src = archive_entry_sourcepath(e);

	if (write_header_checked(e) < 0)
		return;
	nentries++;
	if (archive_entry_filetype(e) == AE_IFREG && archive_entry_size(e) > 0 && src != NULL) {
		char buf[65536];
		int fd = open(src, O_RDONLY);
		ssize_t r;
		la_int64_t left = archive_entry_size(e);

		if (fd < 0) {
			err_errno(src, errno);
			return;
		}
		while (left > 0 && (r = read(fd, buf, sizeof(buf))) > 0) {
			if (r > left)
				r = (ssize_t)left;
			if (archive_write_data(wa, buf, (size_t)r) < 0) {
				err("%s: %s", archive_entry_pathname(e), archive_error_string(wa));
				break;
			}
			left -= r;
		}
		if (left > 0)
			err("%s: file changed size while archiving", src);
		close(fd);
	}
}

static void
add_path(const char *srcpath, const char *arcname)
{
	struct stat st;
	struct archive_entry *e, *spare = NULL;
	char *name;
	char link[PATH_MAX];
	int isdir;

	if (lstat(srcpath, &st) < 0) {
		err_errno(srcpath, errno);
		return;
	}
	if (S_ISFIFO(st.st_mode) || S_ISSOCK(st.st_mode))
		return;
	isdir = S_ISDIR(st.st_mode);
	if (isdir && O.xdev && st.st_dev != top_dev)
		return;
	/* PKZip directory names end in a slash, CPIO ones do not */
	name = isdir && O.zip ? join(arcname, "") : xstrdup(arcname);
	vlog(2, "copying %s %s ... \n", S_ISLNK(st.st_mode) ? "symlink" : "file", arcname);

	e = archive_entry_new();
	archive_entry_copy_pathname(e, isdir && O.zip ? name : arcname);
	free(name);
	archive_entry_copy_sourcepath(e, srcpath);
	archive_entry_copy_stat(e, &st);
	if (S_ISLNK(st.st_mode)) {
		ssize_t n = readlink(srcpath, link, sizeof(link) - 1);

		if (n < 0) {
			err_errno(srcpath, errno);
			archive_entry_free(e);
			return;
		}
		link[n] = 0;
		archive_entry_copy_symlink(e, link);
		archive_entry_set_size(e, 0);
	}
	if (isdir)
		archive_entry_set_size(e, 0);

	if (O.zip) {
		write_entry(e);
		archive_entry_free(e);
		write_appledouble(srcpath, arcname, &st);
	} else {
		/* AppleDouble data precedes the item it belongs to */
		write_appledouble(srcpath, arcname, &st);
		if (isdir) {
			write_entry(e);
			archive_entry_free(e);
		} else {
			archive_entry_linkify(resolver, &e, &spare);
			if (e != NULL) {
				write_entry(e);
				archive_entry_free(e);
			}
			if (spare != NULL) {
				write_entry(spare);
				archive_entry_free(spare);
			}
		}
	}
	if (isdir) {
		DIR *d = opendir(srcpath);
		struct dirent *de;

		if (d == NULL) {
			err_errno(srcpath, errno);
			return;
		}
		while ((de = readdir(d)) != NULL) {
			char *s, *a;

			if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0 ||
			    ignored_name(de->d_name))
				continue;
			s = join(srcpath, de->d_name);
			a = join(arcname, de->d_name);
			add_path(s, a);
			free(s);
			free(a);
		}
		closedir(d);
	}
}

static void
do_create(char **srcs, int nsrc, const char *dst)
{
	char real[PATH_MAX], *name;
	struct stat st;

	if (nsrc > 1) {
		fputs("ditto: Can't archive multiple sources\n" USAGE_LINE, stderr);
		failed = 1;
		return;
	}
	if (realpath(srcs[0], real) == NULL || stat(real, &st) < 0) {
		err("Cannot get the real path for source '%s'", srcs[0]);
		return;
	}
	top_dev = st.st_dev;
	wa = archive_write_new();
	if (O.zip) {
		archive_write_set_format_zip(wa);
		if (O.level >= 0) {
			char opt[8];

			snprintf(opt, sizeof(opt), "%d", O.level);
			archive_write_set_format_option(wa, "zip", "compression-level", opt);
		}
	} else {
		archive_write_set_format_cpio(wa);
		if (O.gzip)
			archive_write_add_filter_gzip(wa);
		else if (O.bzip2)
			archive_write_add_filter_bzip2(wa);
		resolver = archive_entry_linkresolver_new();
		archive_entry_linkresolver_set_strategy(resolver, ARCHIVE_FORMAT_CPIO_POSIX);
	}
	if (archive_write_open_filename(wa, strcmp(dst, "-") == 0 ? NULL : dst) != ARCHIVE_OK) {
		err("%s: %s", dst, archive_error_string(wa));
		archive_write_free(wa);
		return;
	}
	if (O.verbose)
		fprintf(stderr, O.verbose > 1 ? ">>> Copying %s \n" : "Copying %s \n", srcs[0]);
	name = base_name(srcs[0]);
	if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
		free(name);
		name = base_name(real);
	}
	if (S_ISDIR(st.st_mode)) {
		if (O.keep_parent) {
			char *arc = O.zip ? xstrdup(name) : join(".", name);

			add_path(real, arc);
			free(arc);
		} else if (O.zip) {
			/* the contents, without an entry for the directory itself */
			DIR *d = opendir(real);
			struct dirent *de;

			if (d == NULL) {
				err_errno(real, errno);
			} else {
				while ((de = readdir(d)) != NULL) {
					char *s;

					if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0 ||
					    ignored_name(de->d_name))
						continue;
					s = join(real, de->d_name);
					add_path(s, de->d_name);
					free(s);
				}
				closedir(d);
			}
		} else {
			add_path(real, ".");
		}
	} else {
		char *arc, *rel = name;

		if (O.keep_parent) {
			/* the file's parent directory name is embedded */
			char *parent = xstrdup(real), *slash = strrchr(parent, '/'), *pname;

			if (slash != NULL && slash != parent)
				*slash = 0;
			else if (slash != NULL)
				slash[1] = 0;
			pname = base_name(parent);
			rel = join(pname, name);
			free(parent);
			free(pname);
		}
		arc = O.zip ? xstrdup(rel) : join(".", rel);
		add_path(real, arc);
		free(arc);
		if (rel != name)
			free(rel);
	}
	free(name);
	if (resolver != NULL) {
		struct archive_entry *e = NULL, *spare = NULL;

		/* links held back until their last occurrence */
		for (;;) {
			archive_entry_linkify(resolver, &e, &spare);
			if (e == NULL)
				break;
			write_entry(e);
			archive_entry_free(e);
			e = NULL;
		}
		archive_entry_linkresolver_free(resolver);
	}
	if (O.zip && nentries == 0) {
		err("No files for PKZip archive");
		archive_write_free(wa);
		unlink(dst);
		return;
	}
	if (archive_write_close(wa) != ARCHIVE_OK)
		err("%s: %s", dst, archive_error_string(wa));
	archive_write_free(wa);
}

/* ---- archive extraction ----------------------------------------------- */

struct ad_item {
	char *tmp;		/* AppleDouble data file (valid) or archive path */
	char *partner;		/* path of the item it belongs to, relative to the destination */
	int valid, zip;
	struct ad_item *next;
};

static struct ad_item *ad_head, *ad_tail;
static char *ad_tmpdir;
static unsigned ad_counter;

/* True for absolute paths and paths with a ".." component. */
static int
path_escapes(const char *p)
{
	const char *c = p;

	if (*p == '/')
		return 1;
	for (;;) {
		const char *end = strchr(c, '/');
		size_t n = end ? (size_t)(end - c) : strlen(c);

		if (n == 2 && c[0] == '.' && c[1] == '.')
			return 1;
		if (end == NULL)
			return 0;
		c = end + 1;
	}
}

/*
 * Lexically confine an archive path to the destination: drop empty and
 * "." components and let ".." remove the previous component, ignoring it at
 * the root. Returns a new string, empty for the destination itself.
 */
static char *
confine_path(const char *p)
{
	char *res = xmalloc(strlen(p) + 1);
	size_t len = 0;

	res[0] = 0;
	while (*p) {
		const char *end;
		size_t n;

		while (*p == '/')
			p++;
		if (*p == 0)
			break;
		end = strchr(p, '/');
		n = end ? (size_t)(end - p) : strlen(p);
		if (n == 1 && p[0] == '.') {
			/* nothing */
		} else if (n == 2 && p[0] == '.' && p[1] == '.') {
			while (len > 0 && res[len - 1] != '/')
				len--;
			if (len > 0)
				len--;
			res[len] = 0;
		} else {
			if (len > 0)
				res[len++] = '/';
			memcpy(res + len, p, n);
			len += n;
			res[len] = 0;
		}
		p += n;
	}
	return res;
}

static int
is_appledouble_name(const char *path)
{
	const char *b = strrchr(path, '/');

	b = b ? b + 1 : path;
	return b[0] == '.' && b[1] == '_' && b[2] != 0;
}

static int
under_sequester(const char *path)
{
	size_t n = strlen(SEQUESTER_DIR);

	return strncmp(path, SEQUESTER_DIR, n) == 0 && (path[n] == '/' || path[n] == 0);
}

/* Path of the item an AppleDouble entry belongs to. */
static char *
partner_path(const char *path)
{
	const char *p = path, *slash;
	char *res;

	if (under_sequester(p)) {
		p += strlen(SEQUESTER_DIR);
		while (*p == '/')
			p++;
	}
	slash = strrchr(p, '/');
	res = xmalloc(strlen(p) + 1);
	if (slash != NULL) {
		size_t dl = (size_t)(slash - p) + 1;

		memcpy(res, p, dl);
		strcpy(res + dl, slash + 3);
	} else {
		strcpy(res, p + 2);
	}
	return res;
}

/*
 * Remember an AppleDouble entry. The metadata is applied after the whole
 * archive has been extracted, because the item may come after its
 * AppleDouble entry.
 */
static void
queue_ad(const char *path, const unsigned char *data, size_t len)
{
	struct ad_item *it = xmalloc(sizeof(*it));

	it->valid = len >= 26 &&
	    (((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) |
	    ((uint32_t)data[2] << 8) | data[3]) == APPLEDOUBLE_MAGIC;
	it->partner = partner_path(path);
	it->zip = O.zip;
	it->next = NULL;
	if (it->valid) {
		char name[32];
		int fd;

		if (ad_tmpdir == NULL) {
			const char *tdir = getenv("TMPDIR");

			if (tdir == NULL || *tdir == 0)
				tdir = "/tmp";
			ad_tmpdir = join(tdir, "ditto.XXXXXX");
			if (mkdtemp(ad_tmpdir) == NULL) {
				err("cannot create a temporary directory: %s", strerror(errno));
				goto drop;
			}
		}
		snprintf(name, sizeof(name), "ad.%u", ad_counter++);
		it->tmp = join(ad_tmpdir, name);
		fd = open(it->tmp, O_WRONLY | O_CREAT | O_EXCL, 0600);
		if (fd < 0 || write(fd, data, len) != (ssize_t)len) {
			err("cannot store the extended attributes of %s", path);
			if (fd >= 0)
				close(fd);
			free(it->tmp);
			goto drop;
		}
		close(fd);
	} else {
		it->tmp = NULL;
	}
	if (ad_tail)
		ad_tail->next = it;
	else
		ad_head = it;
	ad_tail = it;
	return;
drop:
	free(it->partner);
	free(it);
}

/* True if no directory component of rel (relative to the cwd) is a symlink. */
static int
no_symlink_parents(const char *rel)
{
	char *p = xstrdup(rel), *s;
	int ok = 1;

	for (s = p; ok && *s; s++) {
		if (*s == '/') {
			struct stat st;

			*s = 0;
			if (lstat(p, &st) < 0 || S_ISLNK(st.st_mode))
				ok = 0;
			*s = '/';
		}
	}
	free(p);
	return ok;
}

static void
apply_appledouble(void)
{
	struct ad_item *it, *next;

	for (it = ad_head; it != NULL; it = next) {
		struct stat st;

		next = it->next;
		if (path_escapes(it->partner) || it->partner[0] == 0 ||
		    !no_symlink_parents(it->partner) || lstat(it->partner, &st) < 0)
			goto done;	/* nothing to attach the metadata to */
		if (!it->valid) {
			/* a PKZip archive's unusable AppleDouble entry is an error, a CPIO one is dropped */
			if (it->zip)
				err("%s: %s", it->partner, strerror(EINVAL));
		} else if (O.rsrc) {
			copyfile_flags_t f = COPYFILE_UNPACK | COPYFILE_NOFOLLOW | COPYFILE_XATTR;
			int r;

			if (O.acl)
				f |= COPYFILE_ACL;
			r = copyfile(it->tmp, it->partner, NULL, f);
			if (r < 0 && errno == EACCES && !S_ISLNK(st.st_mode) &&
			    chmod(it->partner, (st.st_mode & 07777) | 0700) == 0) {
				/* a read-only item: allow the write, then restore the mode */
				r = copyfile(it->tmp, it->partner, NULL, f);
				chmod(it->partner, st.st_mode & 07777);
			}
			if (r < 0 && copyfile_failed())
				err_errno(it->partner, errno);
			else
				filter_xattrs(it->partner);
		}
done:
		if (it->tmp != NULL)
			unlink(it->tmp);
		free(it->tmp);
		free(it->partner);
		free(it);
	}
	ad_head = ad_tail = NULL;
}

/* Read the data of the current entry into memory (AppleDouble entries are small). */
static int
read_whole_entry(struct archive *a, unsigned char **buf, size_t *len)
{
	size_t cap = 4096, n = 0;
	unsigned char *b = xmalloc(cap);
	const void *blk;
	size_t sz;
	la_int64_t off;
	int r;

	while ((r = archive_read_data_block(a, &blk, &sz, &off)) == ARCHIVE_OK) {
		if (n + sz > 64u * 1024 * 1024) {
			free(b);
			return -1;
		}
		while (n + sz > cap)
			cap *= 2;
		b = xrealloc(b, cap);
		memcpy(b + n, blk, sz);
		n += sz;
	}
	if (r != ARCHIVE_EOF) {
		free(b);
		return -1;
	}
	*buf = b;
	*len = n;
	return 0;
}

static const char *
passphrase_cb(struct archive *a, void *client)
{
	static char pw[256];

	(void)a;
	(void)client;
	if (readpassphrase("Password: ", pw, sizeof(pw), 0) == NULL)
		return NULL;
	return pw;
}

static void
extract_one(const char *archive_path)
{
	struct archive *a = archive_read_new(), *ext;
	struct archive_entry *e;
	int r, first = 1;
	int flags = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM |
	    ARCHIVE_EXTRACT_SECURE_SYMLINKS | ARCHIVE_EXTRACT_SECURE_NODOTDOT |
	    ARCHIVE_EXTRACT_SECURE_NOABSOLUTEPATHS | ARCHIVE_EXTRACT_UNLINK;

	archive_read_support_filter_all(a);
	if (O.zip) {
		archive_read_support_format_zip(a);
		/*
		 * AppleDouble entries are handled here for every kind of input
		 * (seekable or streamed, sequestered or not); do not let
		 * libarchive hide the __MACOSX entries from this loop.
		 */
		archive_read_set_options(a, "zip:!mac-ext");
	} else {
		archive_read_support_format_cpio(a);
	}
	if (O.password)
		archive_read_set_passphrase_callback(a, NULL, passphrase_cb);
	if (getuid() == 0)
		flags |= ARCHIVE_EXTRACT_OWNER;
	ext = archive_write_disk_new();
	archive_write_disk_set_options(ext, flags);
	archive_write_disk_set_standard_lookup(ext);

	if (archive_read_open_filename(a, strcmp(archive_path, "-") == 0 ? NULL : archive_path,
	    10240) != ARCHIVE_OK) {
		if (archive_errno(a) != 0)
			err("%s: %s", archive_path, strerror(archive_errno(a)));
		else
			err("%s: %s", archive_path, archive_error_string(a));
		goto out;
	}
	if (O.verbose)
		fprintf(stderr, O.verbose > 1 ? ">>> Copying %s \n" : "Copying %s \n", archive_path);
	while ((r = archive_read_next_header(a, &e)) != ARCHIVE_EOF) {
		const char *path, *hl;
		char *norm = NULL, *hlnorm = NULL;
		mode_t ft;

		if (r < ARCHIVE_WARN) {
			if (first && O.zip)
				err("Couldn't read PKZip signature");
			else if (O.zip)
				err("%s", archive_error_string(a));
			else
				err("cpio read error: %s", archive_error_string(a) != NULL ?
				    archive_error_string(a) : "Undefined error: 0");
			break;
		}
		first = 0;
		path = archive_entry_pathname(e);
		if (path == NULL)
			continue;
		hl = archive_entry_hardlink(e);
		/*
		 * macOS confines entry names to the destination instead of
		 * refusing them: leading slashes are dropped and ".." removes
		 * the previous component, at most up to the destination root
		 * (observed for "../x", "a/../b", "a/../../y", "/abs/x").
		 * Entries below a symlink are still never followed
		 * (ARCHIVE_EXTRACT_SECURE_SYMLINKS).
		 */
		path = norm = confine_path(path);
		hl = archive_entry_hardlink(e);
		if (hl != NULL)
			hl = hlnorm = confine_path(hl);
		if (*path == 0) {
			archive_read_data_skip(a);	/* the destination itself */
			free(norm);
			free(hlnorm);
			continue;
		}
		ft = archive_entry_filetype(e);
		if (O.zip && archive_entry_is_encrypted(e) && !O.password) {
			err("no password provided for encrypted PKZip archive");
			archive_read_data_skip(a);
			continue;
		}
		if (ft == AE_IFDIR && under_sequester(path)) {
			archive_read_data_skip(a);
			continue;
		}
		if (ft == AE_IFREG && hl == NULL && is_appledouble_name(path)) {
			unsigned char *buf;
			size_t len;

			if (read_whole_entry(a, &buf, &len) == 0) {
				queue_ad(path, buf, len);
				free(buf);
			} else {
				err("%s: %s", path, archive_error_string(a));
			}
			continue;
		}
		if (ft == AE_IFDIR) {
			struct stat st;

			/* existing directories keep their mode, owner and attributes */
			if (lstat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
				archive_read_data_skip(a);
				continue;
			}
		}
		if (getuid() != 0)
			archive_entry_set_perm(e, archive_entry_perm(e) & ~(mode_t)06000);
		{
			char *p = xstrdup(path);

			archive_entry_copy_pathname(e, p);
			free(p);
			if (hl != NULL && *hl != 0)
				archive_entry_copy_hardlink(e, hl);

		}
		vlog(2, "copying %s ./%s ... \n", ft == AE_IFLNK ? "symlink" : "file", path);
		r = archive_write_header(ext, e);
		if (r < ARCHIVE_OK) {
			err("%s: %s", path, archive_error_string(ext));
			if (r < ARCHIVE_WARN) {
				archive_read_data_skip(a);
				continue;
			}
		}
		if (archive_entry_size(e) > 0 && archive_entry_filetype(e) == AE_IFREG) {
			const void *blk;
			size_t sz;
			la_int64_t off;

			while ((r = archive_read_data_block(a, &blk, &sz, &off)) == ARCHIVE_OK) {
				if (archive_write_data_block(ext, blk, sz, off) < ARCHIVE_WARN) {
					err("%s: %s", path, archive_error_string(ext));
					break;
				}
			}
			if (r != ARCHIVE_EOF && r != ARCHIVE_OK)
				err("%s: %s", path, archive_error_string(a));
		}
		archive_write_finish_entry(ext);
	}
out:
	archive_write_close(ext);
	archive_write_free(ext);
	archive_read_free(a);
	apply_appledouble();
}

static void
do_extract(char **srcs, int nsrc, const char *dst)
{
	int i;

	/* resolve the archive paths before changing into the destination */
	for (i = 0; i < nsrc; i++) {
		char real[PATH_MAX];

		if (strcmp(srcs[i], "-") != 0 && realpath(srcs[i], real) != NULL)
			srcs[i] = xstrdup(real);
	}
	if (make_parents(dst) < 0)
		return;
	if (mkdir(dst, 0777) < 0 && errno != EEXIST) {
		err_errno(dst, errno);
		return;
	}
	if (chdir(dst) < 0) {
		err_errno(dst, errno);
		return;
	}
	for (i = 0; i < nsrc; i++)
		extract_one(srcs[i]);
	if (ad_tmpdir != NULL)
		rmdir(ad_tmpdir);
}

/* ---- option parsing --------------------------------------------------- */

static const struct {
	const char *name;
	int takes_arg;
} long_opts[] = {
	{ "help", 0 }, { "keepParent", 0 }, { "arch", 1 }, { "bom", 1 },
	{ "lang", 1 }, { "outBom", 1 }, { "rsrc", 0 }, { "norsrc", 0 },
	{ "extattr", 0 }, { "noextattr", 0 }, { "qtn", 0 }, { "noqtn", 0 },
	{ "acl", 0 }, { "noacl", 0 }, { "sequesterRsrc", 0 },
	{ "zlibCompressionLevel", 1 }, { "password", 0 }, { "hfsCompression", 0 },
	{ "nohfsCompression", 0 }, { "preserveHFSCompression", 0 },
	{ "nopreserveHFSCompression", 0 }, { "nocache", 0 },
	{ "nonAtomicCopies", 0 }, { "segmentLargeFiles", 0 },
	{ "persistRootless", 0 }, { "nopersistRootless", 0 },
	{ "keepBinaries", 0 }, { "keepBinariesList", 1 },
	{ "keepBinariesPattern", 1 }, { "clone", 0 }, { "noclone", 0 },
	{ "option", 1 },
};

static int
find_long(const char *name)
{
	size_t i;

	for (i = 0; i < sizeof(long_opts) / sizeof(long_opts[0]); i++)
		if (strcmp(long_opts[i].name, name) == 0)
			return (int)i;
	return -1;
}

static void
apply_long(const char *name, const char *arg)
{
	if (strcmp(name, "help") == 0) {
		usage(stdout);
		exit(0);
	} else if (strcmp(name, "keepParent") == 0) {
		O.keep_parent = 1;
	} else if (strcmp(name, "arch") == 0) {
		size_t i;

		for (i = 0; i < NARCH_NAMES; i++)
			if (strcmp(arch_names[i].name, arg) == 0)
				break;
		if (i == NARCH_NAMES)
			bad_usage("Unknown architecture '%s'", arg);
		if (O.narch == (int)(sizeof(O.arch) / sizeof(O.arch[0])))
			bad_usage("Too many --arch options");
		O.arch[O.narch++] = arg;
	} else if (strcmp(name, "bom") == 0) {
		O.bom = arg;
	} else if (strcmp(name, "rsrc") == 0) {
		O.rsrc = O.rsrc_set = 1;
	} else if (strcmp(name, "norsrc") == 0) {
		O.rsrc = 0;
		O.rsrc_set = 1;
	} else if (strcmp(name, "extattr") == 0) {
		O.extattr = O.extattr_set = 1;
	} else if (strcmp(name, "noextattr") == 0) {
		O.extattr = 0;
		O.extattr_set = 1;
	} else if (strcmp(name, "qtn") == 0) {
		O.qtn = O.qtn_set = 1;
	} else if (strcmp(name, "noqtn") == 0) {
		O.qtn = 0;
		O.qtn_set = 1;
	} else if (strcmp(name, "acl") == 0) {
		O.acl = O.acl_set = 1;
	} else if (strcmp(name, "noacl") == 0) {
		O.acl = 0;
		O.acl_set = 1;
	} else if (strcmp(name, "sequesterRsrc") == 0) {
		O.sequester = 1;
	} else if (strcmp(name, "zlibCompressionLevel") == 0) {
		char *end;
		long v = strtol(arg, &end, 10);

		if (*arg == 0 || *end != 0 || v < 0 || v > 9) {
			fprintf(stderr, "ditto: Invalid zlib compression level: %s\n", arg);
			exit(1);
		}
		O.level = (int)v;
	} else if (strcmp(name, "password") == 0) {
		O.password = 1;
	} else if (strcmp(name, "nonAtomicCopies") == 0) {
		O.atomic = 0;
	}
	/* The remaining options are accepted and change nothing here. */
}

/* Parse the options into O; returns the number of operands stored in operands. */
static int
parse(int argc, char **argv, char **operands)
{
	int i, nop = 0, endopts = 0;

	for (i = 1; i < argc; i++) {
		const char *a = argv[i];
		char *nm, *eq;
		int idx;

		if (endopts || a[0] != '-' || a[1] == 0) {
			operands[nop++] = argv[i];
			continue;
		}
		if (strcmp(a, "--") == 0) {
			endopts = 1;
			continue;
		}
		/* long options, with "--" or (as the historic -rsrc) one dash */
		nm = xstrdup(a[1] == '-' ? a + 2 : a + 1);
		eq = a[1] == '-' ? strchr(nm, '=') : NULL;
		if (eq != NULL)
			*eq++ = 0;
		idx = find_long(nm);
		if (idx >= 0 && (a[1] == '-' || strlen(nm) > 1)) {
			const char *arg = NULL;

			if (long_opts[idx].takes_arg) {
				if (eq != NULL)
					arg = eq;
				else if (i + 1 < argc)
					arg = argv[++i];
				else
					bad_usage("option `--%s' requires an argument", nm);
			}
			apply_long(nm, arg);
			continue;
		}
		if (a[1] == '-')
			bad_usage("unrecognized option `%s'", a);
		free(nm);
		for (a = a + 1; *a; a++) {
			switch (*a) {
			case 'h': usage(stdout); exit(0);
			case 'v': if (O.verbose < 1) O.verbose = 1; break;
			case 'V': O.verbose = 2; break;
			case 'X': O.xdev = 1; break;
			case 'c': O.create = 1; break;
			case 'x': O.extract = 1; break;
			case 'z': O.gzip = 1; break;
			case 'j': O.bzip2 = 1; break;
			case 'k': O.zip = 1; break;
			default: bad_usage("invalid option -- %c", *a);
			}
		}
	}
	return nop;
}

int
main(int argc, char **argv)
{
	char **operands = xmalloc(sizeof(char *) * (size_t)(argc + 1));
	int nop = parse(argc, argv, operands);
	const char *dst;

	if (O.create && O.extract)
		bad_usage("-c and -x cannot be combined");
	if ((O.gzip || O.bzip2) && O.zip)
		bad_usage("-%c is only for cpio archives", O.gzip ? 'z' : 'j');
	if (O.keep_parent && !O.create)
		bad_usage("--keepParent only works with -c");
	/* --norsrc implies --noextattr, --noacl and --noqtn unless those were given */
	if (!O.rsrc) {
		if (!O.extattr_set)
			O.extattr = 0;
		if (!O.acl_set)
			O.acl = 0;
		if (!O.qtn_set)
			O.qtn = 0;
	}
	if (getenv("DITTONORSRC") != NULL) {
		if (!O.rsrc_set)
			O.rsrc = 0;
		if (!O.extattr_set)
			O.extattr = 0;
		if (!O.acl_set)
			O.acl = 0;
		if (!O.qtn_set)
			O.qtn = 0;
	}
	if (nop == 0) {
		usage(stderr);
		return 1;
	}
	if (nop < 2)
		bad_usage("No destination");
	dst = operands[nop - 1];
	if (O.create) {
		do_create(operands, nop - 1, dst);
	} else if (O.extract) {
		do_extract(operands, nop - 1, dst);
	} else if (O.bom != NULL) {
		if (nop != 2)
			bad_usage("--bom takes one source and one destination");
		return bom_copy(O.bom, operands[0], dst);
	} else {
		do_copy(operands, nop - 1, dst);
	}
	return failed ? 1 : 0;
}
