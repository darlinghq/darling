// Copyright (C) 2026 Lubos Dolezel
//
// getattrlist(), fgetattrlist() and getattrlistbulk() over a fixed tree,
// compared with what macOS 27 (arm64, and x86_64 under Rosetta) returns for
// the same calls: ATTR_CMN_RETURNED_ATTRS, names, object types, FinderInfo
// (an all-zero one is not returned by getattrlist but is by
// getattrlistbulk), resource fork length, file attributes only for
// non-directories and directory attributes only for directories, symlink
// following, short buffers (ERANGE only under 4 bytes, otherwise truncated
// with the full length) and getattrlistbulk's ERANGE/EINVAL cases.
// The expected values come from vd-upstream/mac-probes/mc8attrlist.c.
//
// Usage: getattrlist-returned [empty scratch dir]

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/attr.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include <unistd.h>

static int failed;

static struct attrlist withfile = { .bitmapcount = ATTR_BIT_MAP_COUNT,
	.commonattr = ATTR_CMN_RETURNED_ATTRS | ATTR_CMN_NAME | ATTR_CMN_OBJTYPE | ATTR_CMN_FNDRINFO,
	.fileattr = ATTR_FILE_DATALENGTH | ATTR_FILE_RSRCLENGTH };
static struct attrlist withdir = { .bitmapcount = ATTR_BIT_MAP_COUNT,
	.commonattr = ATTR_CMN_RETURNED_ATTRS | ATTR_CMN_NAME | ATTR_CMN_OBJTYPE | ATTR_CMN_FNDRINFO,
	.dirattr = ATTR_DIR_ENTRYCOUNT };
static struct attrlist noret = { .bitmapcount = ATTR_BIT_MAP_COUNT,
	.commonattr = ATTR_CMN_NAME | ATTR_CMN_OBJTYPE, .fileattr = ATTR_FILE_RSRCLENGTH };
static struct attrlist bulkattrs = { .bitmapcount = ATTR_BIT_MAP_COUNT,
	.commonattr = ATTR_CMN_RETURNED_ATTRS | ATTR_CMN_NAME | ATTR_CMN_ERROR | ATTR_CMN_OBJTYPE | ATTR_CMN_FNDRINFO,
	.dirattr = ATTR_DIR_ENTRYCOUNT, .fileattr = ATTR_FILE_DATALENGTH | ATTR_FILE_RSRCLENGTH };

static const struct {
	const char *label, *path;
	struct attrlist *al;
	size_t bufsize;
	unsigned long opts;
	int err;
	const char *hex;
} cases[] = {
	{ "T/file1 common+file", "T/file1", &withfile, 1024, 0x0, 0,
	  "5c000000 09000080 00000000 00000000 00120000 00000000 3c000000 06000000 01000000 05000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 66696c65 31000000 " },
	{ "T/file1 common+dir", "T/file1", &withdir, 1024, 0x0, 0,
	  "4c000000 09000080 00000000 00000000 00000000 00000000 2c000000 06000000 01000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 66696c65 31000000 " },
	{ "T/rsrc common+file", "T/rsrc", &withfile, 1024, 0x0, 0,
	  "5c000000 09400080 00000000 00000000 00120000 00000000 3c000000 05000000 01000000 54455854 74747874 00000000 00000000 00000000 00000000 00000000 00000000 03000000 00000000 07000000 00000000 72737263 00000000 " },
	{ "T/rsrc common+dir", "T/rsrc", &withdir, 1024, 0x0, 0,
	  "4c000000 09400080 00000000 00000000 00000000 00000000 2c000000 05000000 01000000 54455854 74747874 00000000 00000000 00000000 00000000 00000000 00000000 72737263 00000000 " },
	{ "T/sub common+file", "T/sub", &withfile, 1024, 0x0, 0,
	  "48000000 09000080 00000000 00000000 00000000 00000000 2c000000 04000000 02000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 73756200 " },
	{ "T/sub common+dir", "T/sub", &withdir, 1024, 0x0, 0,
	  "4c000000 09000080 00000000 02000000 00000000 00000000 30000000 04000000 02000000 02000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 73756200 " },
	{ "T/link common+file", "T/link", &withfile, 1024, 0x0, 0,
	  "5c000000 09000080 00000000 00000000 00120000 00000000 3c000000 06000000 01000000 05000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 66696c65 31000000 " },
	{ "T/link common+dir", "T/link", &withdir, 1024, 0x0, 0,
	  "4c000000 09000080 00000000 00000000 00000000 00000000 2c000000 06000000 01000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 66696c65 31000000 " },
	{ "T/link common+file NOFOLLOW", "T/link", &withfile, 1024, 0x1, 0,
	  "5c000000 09000080 00000000 00000000 00120000 00000000 3c000000 05000000 05000000 05000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 6c696e6b 00000000 " },
	{ "T/rsrc name+objtype+rsrclength (no RETURNED_ATTRS)", "T/rsrc", &noret, 1024, 0x0, 0,
	  "20000000 14000000 05000000 01000000 07000000 00000000 72737263 00000000 " },
	{ "short 0", "T/rsrc", &withfile, 0, 0x0, ERANGE,
	  "" },
	{ "short FULLSIZE 0", "T/rsrc", &withfile, 0, 0x4, ERANGE,
	  "" },
	{ "short 2", "T/rsrc", &withfile, 2, 0x0, ERANGE,
	  "" },
	{ "short FULLSIZE 2", "T/rsrc", &withfile, 2, 0x4, ERANGE,
	  "" },
	{ "short 4", "T/rsrc", &withfile, 4, 0x0, 0,
	  "5c000000 " },
	{ "short FULLSIZE 4", "T/rsrc", &withfile, 4, 0x4, 0,
	  "5c000000 " },
	{ "short 8", "T/rsrc", &withfile, 8, 0x0, 0,
	  "5c000000 09400080 " },
	{ "short FULLSIZE 8", "T/rsrc", &withfile, 8, 0x4, 0,
	  "5c000000 09400080 " },
	{ "short 20", "T/rsrc", &withfile, 20, 0x0, 0,
	  "5c000000 09400080 00000000 00000000 00120000 " },
	{ "short FULLSIZE 20", "T/rsrc", &withfile, 20, 0x4, 0,
	  "5c000000 09400080 00000000 00000000 00120000 " },
	{ "short 40", "T/rsrc", &withfile, 40, 0x0, 0,
	  "5c000000 09400080 00000000 00000000 00120000 00000000 3c000000 05000000 01000000 54455854 " },
	{ "short FULLSIZE 40", "T/rsrc", &withfile, 40, 0x4, 0,
	  "5c000000 09400080 00000000 00000000 00120000 00000000 3c000000 05000000 01000000 54455854 " },
	{ "short 4", "T/rsrc", &noret, 4, 0x0, 0,
	  "20000000 " },
	{ "short FULLSIZE 4", "T/rsrc", &noret, 4, 0x4, 0,
	  "20000000 " },
	{ "short 8", "T/rsrc", &noret, 8, 0x0, 0,
	  "20000000 14000000 " },
	{ "short FULLSIZE 8", "T/rsrc", &noret, 8, 0x4, 0,
	  "20000000 14000000 " },
	{ "short 16", "T/rsrc", &noret, 16, 0x0, 0,
	  "20000000 14000000 05000000 01000000 " },
	{ "short FULLSIZE 16", "T/rsrc", &noret, 16, 0x4, 0,
	  "20000000 14000000 05000000 01000000 " },
};

static const struct {
	const char *name, *text;
} bulk_entries[] = {
	{ "sub",
	  "length 80; returned: cmn 0x80004009 vol 0x00000000 dir 0x00000002 file 0x00000000 fork 0x00000000; name \"sub\" (offset 52, length 4); objtype 2 VDIR; finderinfo 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000; entrycount 2" },
	{ "link",
	  "length 96; returned: cmn 0x80004009 vol 0x00000000 dir 0x00000000 file 0x00001200 fork 0x00000000; name \"link\" (offset 64, length 5); objtype 5 VLNK; finderinfo 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000; datalength 5; rsrclength 0" },
	{ "rsrc",
	  "length 96; returned: cmn 0x80004009 vol 0x00000000 dir 0x00000000 file 0x00001200 fork 0x00000000; name \"rsrc\" (offset 64, length 5); objtype 1 VREG; finderinfo 54455854 74747874 00000000 00000000 00000000 00000000 00000000 00000000; datalength 3; rsrclength 7" },
	{ "file1",
	  "length 96; returned: cmn 0x80004009 vol 0x00000000 dir 0x00000000 file 0x00001200 fork 0x00000000; name \"file1\" (offset 64, length 6); objtype 1 VREG; finderinfo 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000; datalength 5; rsrclength 0" },
};

static void tohex(const unsigned char *b, size_t n, char *out, size_t size)
{
	size_t used = 0;
	out[0] = 0;
	for (size_t i = 0; i < n; i++)
		used += snprintf(out + used, size - used, "%02x%s", b[i], (i % 4 == 3) ? " " : "");
}

static void single(const char *label, int r, int err, int expect_err, const unsigned char *buf,
	size_t bufsize, const char *expect)
{
	char hex[2048];
	size_t touched = 0;
	for (size_t i = 0; i < bufsize; i++)
		if (buf[i] != 0xa5)
			touched = i + 1;
	tohex(buf, touched, hex, sizeof(hex));
	if ((r ? err : 0) != expect_err || strcmp(hex, expect) != 0) {
		printf("FAIL: %s: result %d errno %d, expected errno %d\n  got      %s\n  expected %s\n",
			label, r, r ? err : 0, expect_err, hex, expect);
		failed = 1;
	}
}

// One getattrlistbulk record, in the format of the Mac probe.
static void describe(const unsigned char *rec, char *out, size_t size)
{
	const unsigned char *p = rec;
	size_t used = 0;
	uint32_t len;
	attribute_set_t ret;
	char fi[80];

	memcpy(&len, p, 4); p += 4;
	memcpy(&ret, p, sizeof(ret)); p += sizeof(ret);
	used += snprintf(out + used, size - used, "length %u; returned: cmn 0x%08x vol 0x%08x dir 0x%08x file 0x%08x fork 0x%08x",
		len, ret.commonattr, ret.volattr, ret.dirattr, ret.fileattr, ret.forkattr);
	if (ret.commonattr & ATTR_CMN_ERROR) {
		uint32_t e; memcpy(&e, p, 4); p += 4;
		used += snprintf(out + used, size - used, "; error %u", e);
	}
	if (ret.commonattr & ATTR_CMN_NAME) {
		attrreference_t r; memcpy(&r, p, sizeof(r));
		used += snprintf(out + used, size - used, "; name \"%.*s\" (offset %d, length %u)",
			(int)(r.attr_length ? r.attr_length - 1 : 0), (const char *)p + r.attr_dataoffset,
			r.attr_dataoffset, r.attr_length);
		p += sizeof(r);
	}
	if (ret.commonattr & ATTR_CMN_OBJTYPE) {
		static const char *n[] = { "VNON", "VREG", "VDIR", "VBLK", "VCHR", "VLNK", "VSOCK", "VFIFO" };
		uint32_t t; memcpy(&t, p, 4); p += 4;
		used += snprintf(out + used, size - used, "; objtype %u %s", t, t < 8 ? n[t] : "?");
	}
	if (ret.commonattr & ATTR_CMN_FNDRINFO) {
		tohex(p, 32, fi, sizeof(fi));
		fi[strlen(fi) - 1] = 0;
		used += snprintf(out + used, size - used, "; finderinfo %s", fi);
		p += 32;
	}
	if (ret.dirattr & ATTR_DIR_ENTRYCOUNT) {
		uint32_t c; memcpy(&c, p, 4); p += 4;
		used += snprintf(out + used, size - used, "; entrycount %u", c);
	}
	if (ret.fileattr & ATTR_FILE_DATALENGTH) {
		int64_t v; memcpy(&v, p, 8); p += 8;
		used += snprintf(out + used, size - used, "; datalength %lld", (long long)v);
	}
	if (ret.fileattr & ATTR_FILE_RSRCLENGTH) {
		int64_t v; memcpy(&v, p, 8); p += 8;
		used += snprintf(out + used, size - used, "; rsrclength %lld", (long long)v);
	}
}

static void check_bulk_entry(const unsigned char *rec, unsigned *seen)
{
	char text[1024];
	describe(rec, text, sizeof(text));
	for (unsigned i = 0; i < sizeof(bulk_entries) / sizeof(bulk_entries[0]); i++) {
		char want[64];
		snprintf(want, sizeof(want), "name \"%s\" ", bulk_entries[i].name);
		if (!strstr(text, want))
			continue;
		if (*seen & (1u << i)) {
			printf("FAIL: bulk: entry \"%s\" returned twice\n", bulk_entries[i].name);
			failed = 1;
		}
		*seen |= 1u << i;
		if (strcmp(text, bulk_entries[i].text) != 0) {
			printf("FAIL: bulk entry \"%s\"\n  got      %s\n  expected %s\n", bulk_entries[i].name, text,
				bulk_entries[i].text);
			failed = 1;
		}
		return;
	}
	printf("FAIL: bulk: unexpected entry: %s\n", text);
	failed = 1;
}

int main(int argc, char **argv)
{
	char tmpl[] = "/tmp/getattrlist-returned.XXXXXX";
	const char *dir = argc > 1 ? argv[1] : mkdtemp(tmpl);
	if (!dir || chdir(dir) != 0) {
		perror("scratch dir");
		return 1;
	}

	mkdir("T", 0755);
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
	mkdir("T/sub", 0755);
	close(open("T/sub/a", O_CREAT | O_WRONLY, 0644));
	close(open("T/sub/b", O_CREAT | O_WRONLY, 0644));
	symlink("file1", "T/link");

	unsigned char buf[1024];
	for (unsigned i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
		memset(buf, 0xa5, sizeof(buf));
		int r = getattrlist(cases[i].path, cases[i].al, buf, cases[i].bufsize, cases[i].opts);
		single(cases[i].label, r, errno, cases[i].err, buf, sizeof(buf), cases[i].hex);
	}

	// fgetattrlist returns the name too
	struct attrlist named = { .bitmapcount = ATTR_BIT_MAP_COUNT,
		.commonattr = ATTR_CMN_RETURNED_ATTRS | ATTR_CMN_NAME | ATTR_CMN_OBJTYPE };
	fd = open("T/file1", O_RDONLY);
	memset(buf, 0xa5, sizeof(buf));
	int r = fgetattrlist(fd, &named, buf, sizeof(buf), 0);
	single("fgetattrlist name", r, errno, 0, buf, sizeof(buf),
		"2c000000 09000080 00000000 00000000 00000000 00000000 0c000000 06000000 01000000 66696c65 31000000 ");
	close(fd);

	// getattrlistbulk: every entry once, in any order
	unsigned seen = 0;
	int dfd = open("T", O_RDONLY | O_DIRECTORY);
	static unsigned char big[16384];
	for (int calls = 0; calls < 10; calls++) {
		int n = getattrlistbulk(dfd, &bulkattrs, big, sizeof(big), 0);
		if (n < 0) {
			printf("FAIL: getattrlistbulk: errno %d\n", errno);
			failed = 1;
		}
		if (n <= 0)
			break;
		unsigned char *e = big;
		for (int i = 0; i < n; i++) {
			uint32_t len;
			memcpy(&len, e, 4);
			check_bulk_entry(e, &seen);
			e += len;
		}
	}
	close(dfd);
	if (seen != (1u << (sizeof(bulk_entries) / sizeof(bulk_entries[0]))) - 1) {
		printf("FAIL: getattrlistbulk: entries seen 0x%x\n", seen);
		failed = 1;
	}

	// a buffer smaller than one entry: ERANGE, and no entry is consumed
	dfd = open("T", O_RDONLY | O_DIRECTORY);
	size_t small[] = { 4, 16, 24 };
	for (unsigned i = 0; i < 3; i++) {
		errno = 0;
		int n = getattrlistbulk(dfd, &bulkattrs, big, small[i], 0);
		if (n != -1 || errno != ERANGE) {
			printf("FAIL: getattrlistbulk bufsize %zu: %d errno %d, expected ERANGE\n", small[i], n, errno);
			failed = 1;
		}
	}
	int n = getattrlistbulk(dfd, &bulkattrs, big, sizeof(big), 0);
	if (n != 4) {
		printf("FAIL: getattrlistbulk after ERANGE: %d entries, expected 4\n", n);
		failed = 1;
	}
	close(dfd);

	// ATTR_CMN_RETURNED_ATTRS is required
	dfd = open("T", O_RDONLY | O_DIRECTORY);
	struct attrlist bl2 = bulkattrs;
	bl2.commonattr &= ~ATTR_CMN_RETURNED_ATTRS;
	errno = 0;
	n = getattrlistbulk(dfd, &bl2, big, sizeof(big), 0);
	if (n != -1 || errno != EINVAL) {
		printf("FAIL: getattrlistbulk without RETURNED_ATTRS: %d errno %d, expected EINVAL\n", n, errno);
		failed = 1;
	}
	close(dfd);

	if (!failed)
		printf("PASS\n");
	return failed;
}
