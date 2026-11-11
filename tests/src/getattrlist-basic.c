// Copyright (C) 2026 Lubos Dolezel
//
// getattrlist() without ATTR_CMN_RETURNED_ATTRS: names, object types,
// FinderInfo, resource fork length, symlink following and short buffers.
// The expected bytes are what macOS 27 (arm64, and x86_64 under Rosetta)
// writes for the same tree. Usage: getattrlist-basic [empty scratch dir]

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/attr.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include <unistd.h>

static int failed;

#define Z8 "00000000 00000000 "
#define Z32 Z8 Z8 Z8 Z8

static void check(const char *label, const char *path, struct attrlist *al, size_t bufsize,
	unsigned long opts, int expect_errno, const char *expect)
{
	unsigned char buf[512];
	char hex[2048] = "";
	size_t touched = 0, n = 0;

	memset(buf, 0xa5, sizeof(buf));
	errno = 0;
	int r = getattrlist(path, al, buf, bufsize, opts);
	int err = r ? errno : 0;
	for (size_t i = 0; i < sizeof(buf); i++)
		if (buf[i] != 0xa5)
			touched = i + 1;
	for (size_t i = 0; i < touched; i++)
		n += snprintf(hex + n, sizeof(hex) - n, "%02x%s", buf[i], (i % 4 == 3) ? " " : "");
	if (err != expect_errno || strcmp(hex, expect) != 0) {
		printf("FAIL: %s: result %d errno %d, expected errno %d\n  got      %s\n  expected %s\n",
			label, r, err, expect_errno, hex, expect);
		failed = 1;
	}
}

int main(int argc, char **argv)
{
	char tmpl[] = "/tmp/getattrlist-basic.XXXXXX";
	const char *dir = argc > 1 ? argv[1] : mkdtemp(tmpl);
	if (!dir || chdir(dir) != 0) {
		perror("scratch dir");
		return 1;
	}

	mkdir("T", 0755);
	mkdir("T/sub", 0755);
	int fd = open("T/file1", O_CREAT | O_WRONLY | O_TRUNC, 0644);
	write(fd, "hello", 5);
	close(fd);
	fd = open("T/rsrc", O_CREAT | O_WRONLY | O_TRUNC, 0644);
	write(fd, "abc", 3);
	close(fd);
	unsigned char fi[32] = { 'T', 'E', 'X', 'T', 't', 't', 'x', 't' };
	if (setxattr("T/rsrc", XATTR_RESOURCEFORK_NAME, "RSRC123", 7, 0, 0) != 0
		|| setxattr("T/rsrc", XATTR_FINDERINFO_NAME, fi, 32, 0, 0) != 0) {
		perror("setxattr (the file system needs extended attributes)");
		return 1;
	}
	symlink("file1", "T/link");

	struct attrlist all = { .bitmapcount = ATTR_BIT_MAP_COUNT,
		.commonattr = ATTR_CMN_NAME | ATTR_CMN_OBJTYPE | ATTR_CMN_FNDRINFO,
		.fileattr = ATTR_FILE_RSRCLENGTH };
	struct attrlist dirs = { .bitmapcount = ATTR_BIT_MAP_COUNT,
		.commonattr = ATTR_CMN_NAME | ATTR_CMN_OBJTYPE, .dirattr = ATTR_DIR_ENTRYCOUNT };
	struct attrlist small = { .bitmapcount = ATTR_BIT_MAP_COUNT,
		.commonattr = ATTR_CMN_NAME | ATTR_CMN_OBJTYPE, .fileattr = ATTR_FILE_RSRCLENGTH };

	// length, name reference (offset, length), object type, FinderInfo,
	// resource fork length (an 8-byte off_t, 4-byte aligned), name
	check("file1", "T/file1", &all, 512, 0, 0,
		"40000000 34000000 06000000 01000000 " Z32 Z8 "66696c65 31000000 ");
	check("rsrc", "T/rsrc", &all, 512, 0, 0,
		"40000000 34000000 05000000 01000000 54455854 74747874 00000000 00000000 "
		Z8 Z8 "07000000 00000000 72737263 00000000 ");
	// a directory gets no file attributes
	check("sub", "T/sub", &all, 512, 0, 0,
		"34000000 2c000000 04000000 02000000 " Z32 "73756200 ");
	// a file gets no directory attributes
	check("file1 dirattr", "T/file1", &dirs, 512, 0, 0,
		"18000000 0c000000 06000000 01000000 66696c65 31000000 ");
	// symlinks are followed unless FSOPT_NOFOLLOW is given
	check("link", "T/link", &all, 512, 0, 0,
		"40000000 34000000 06000000 01000000 " Z32 Z8 "66696c65 31000000 ");
	check("link nofollow", "T/link", &all, 512, FSOPT_NOFOLLOW, 0,
		"40000000 34000000 05000000 05000000 " Z32 Z8 "6c696e6b 00000000 ");
	check("rsrc small", "T/rsrc", &small, 512, 0, 0,
		"20000000 14000000 05000000 01000000 07000000 00000000 72737263 00000000 ");

	// Short buffers: under 4 bytes is ERANGE; otherwise the result is
	// truncated and the length field holds the full size.
	check("short 0", "T/rsrc", &all, 0, 0, ERANGE, "");
	check("short 2", "T/rsrc", &all, 2, 0, ERANGE, "");
	check("short 2 fullsize", "T/rsrc", &all, 2, FSOPT_REPORT_FULLSIZE, ERANGE, "");
	check("short 4", "T/rsrc", &all, 4, 0, 0, "40000000 ");
	check("short 8", "T/rsrc", &all, 8, 0, 0, "40000000 34000000 ");
	check("short 8 fullsize", "T/rsrc", &all, 8, FSOPT_REPORT_FULLSIZE, 0, "40000000 34000000 ");
	check("short 24", "T/rsrc", &all, 24, 0, 0,
		"40000000 34000000 05000000 01000000 54455854 74747874 ");
	check("small short 16", "T/rsrc", &small, 16, 0, 0,
		"20000000 14000000 05000000 01000000 ");

	check("missing", "T/missing", &all, 512, 0, ENOENT, "");

	if (!failed)
		printf("PASS\n");
	return failed;
}
