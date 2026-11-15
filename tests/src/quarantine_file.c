// Copyright (C) 2026 Lubos Dolezel
//
// libquarantine's file API: the quarantine record kept in the com.apple.quarantine
// extended attribute ("flags;timestamp;agent;metadata", flags as 4 and timestamp as 8
// hex digits) is stored, read back, serialized and carried by copyfile(3). Nothing is
// enforced; this is only the attribute round trip.
//
// Expected results were observed with the same calls on macOS 26.7.3 and 10.14.4. The
// functions are exported by libSystem; the prototypes are in Darling's quarantine.h, not
// in an SDK header (build with -I src/quarantine/include).
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <copyfile.h>
#include <quarantine.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include <unistd.h>

static int failures;

static void check(const char *name, int ok)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", name);
	failures += !ok;
}

static char dir[256];

static char *path(const char *name)
{
	static char buf[8][300];
	static int i;

	i = (i + 1) % 8;
	snprintf(buf[i], sizeof(buf[i]), "%s/%s", dir, name);
	return buf[i];
}

static void touch(const char *name)
{
	int fd = open(path(name), O_CREAT | O_WRONLY | O_TRUNC, 0644);

	if (fd >= 0)
		close(fd);
}

// Returns the attribute text of a path (not following a link) or "" if there is none.
static const char *attr(const char *p)
{
	static char buf[8][512];
	static int i;
	ssize_t n;

	i = (i + 1) % 8;
	n = getxattr(p, "com.apple.quarantine", buf[i], sizeof(buf[i]) - 1, 0, XATTR_NOFOLLOW);
	buf[i][n < 0 ? 0 : n] = 0;
	return n < 0 ? "(none)" : buf[i];
}

static void set_attr(const char *p, const char *text)
{
	setxattr(p, "com.apple.quarantine", text, strlen(text), 0, 0);
}

int main(void)
{
	char tmpl[] = "/tmp/quarantine_file.XXXXXX";
	char data[QTN_SERIALIZED_DATA_MAX];
	size_t len;
	int r, fd;
	qtn_file_t q, q2;

	if (!mkdtemp(tmpl)) {
		perror("mkdtemp");
		return 2;
	}
	strcpy(dir, tmpl);
	touch("a"); touch("b"); touch("c"); touch("d"); touch("e"); touch("f");
	mkdir(path("dir"), 0755);

	check("attribute name", strcmp(qtn_xattr_name, "com.apple.quarantine") == 0);

	q = qtn_file_alloc();
	check("alloc: empty record", qtn_file_get_flags(q) == 0 && qtn_file_get_timestamp(q) == 0
		&& qtn_file_get_identifier(q)[0] == 0 && qtn_file_get_metadata_size(q) == 0);

	// Not quarantined, missing file.
	r = qtn_file_init_with_path(q, path("a"));
	check("init_with_path on an item without the attribute: QTN_NOT_QUARANTINED", r == QTN_NOT_QUARANTINED);
	check("error text for QTN_NOT_QUARANTINED", strcmp(qtn_error(r), "Item not quarantined") == 0);
	check("init_with_path on a missing path: ENOENT", qtn_file_init_with_path(q, path("missing")) == ENOENT);
	check("apply_to_path on a missing path: ENOENT", qtn_file_apply_to_path(q, path("missing")) == ENOENT);

	// Apply and read the attribute text.
	check("set_flags", qtn_file_set_flags(q, 0x41) == 0);
	check("set_identifier", qtn_file_set_identifier(q, "TestAgent") == 0);
	qtn_file_set_timestamp(q, 0x5f000000);
	check("set_metadata", qtn_file_set_metadata(q, "ABCD-1234", 9) == 0);
	check("getters", qtn_file_get_flags(q) == 0x41 && qtn_file_get_timestamp(q) == 0x5f000000
		&& strcmp(qtn_file_get_identifier(q), "TestAgent") == 0 && qtn_file_get_metadata_size(q) == 9
		&& memcmp(qtn_file_get_metadata(q), "ABCD-1234", 9) == 0);
	check("apply_to_path", qtn_file_apply_to_path(q, path("a")) == 0);
	check("attribute text", strcmp(attr(path("a")), "0041;5f000000;TestAgent;ABCD-1234") == 0);

	// Round trip through path and fd.
	q2 = qtn_file_alloc();
	check("init_with_path", qtn_file_init_with_path(q2, path("a")) == 0);
	check("path round trip", qtn_file_get_flags(q2) == 0x41 && qtn_file_get_timestamp(q2) == 0x5f000000
		&& strcmp(qtn_file_get_identifier(q2), "TestAgent") == 0 && qtn_file_get_metadata_size(q2) == 9);
	fd = open(path("a"), O_RDONLY);
	qtn_file_free(q2);
	q2 = qtn_file_alloc();
	check("init_with_fd", qtn_file_init_with_fd(q2, fd) == 0 && qtn_file_get_flags(q2) == 0x41);
	close(fd);

	// Apply to an fd and to a directory.
	fd = open(path("b"), O_RDWR);
	check("apply_to_fd", qtn_file_apply_to_fd(q, fd) == 0);
	close(fd);
	check("apply_to_fd attribute", strcmp(attr(path("b")), "0041;5f000000;TestAgent;ABCD-1234") == 0);
	check("apply_to_path on a directory", qtn_file_apply_to_path(q, path("dir")) == 0
		&& strcmp(attr(path("dir")), "0041;5f000000;TestAgent;ABCD-1234") == 0);

	// Serialized form.
	len = sizeof(data);
	check("to_data", qtn_file_to_data(q, data, &len) == 0 && len == 2 + 33 + 1
		&& memcmp(data, "q/0041;5f000000;TestAgent;ABCD-1234", 35) == 0 && data[len - 1] == 0);
	len = 5;
	r = qtn_file_to_data(q, data, &len);
	check("to_data with a small buffer: ERANGE and the needed size", r == ERANGE && len == 36);
	len = sizeof(data);
	qtn_file_to_data(q, data, &len);
	qtn_file_free(q2);
	q2 = qtn_file_alloc();
	check("init_with_data from to_data", qtn_file_init_with_data(q2, data, len) == 0
		&& qtn_file_get_flags(q2) == 0x41 && qtn_file_get_timestamp(q2) == 0x5f000000
		&& strcmp(qtn_file_get_identifier(q2), "TestAgent") == 0);
	// quirk_serialized_form: the attribute text without the "q/" prefix is refused.
	check("quirk_serialized_form: raw attribute text: EINVAL",
		qtn_file_init_with_data(q2, "0041;5f000000;TestAgent;", 25) == EINVAL);
	check("init_with_data garbage: EINVAL", qtn_file_init_with_data(q2, "garbage", 8) == EINVAL);
	check("init_with_data empty: EINVAL", qtn_file_init_with_data(q2, "", 1) == EINVAL);

	// Clone is independent.
	{
		qtn_file_t cl = qtn_file_clone(q);

		qtn_file_set_identifier(cl, "Other");
		check("clone copies the record", qtn_file_get_flags(cl) == 0x41 && qtn_file_get_timestamp(cl) == 0x5f000000
			&& qtn_file_get_metadata_size(cl) == 9);
		check("clone is independent", strcmp(qtn_file_get_identifier(q), "TestAgent") == 0);
		qtn_file_free(cl);
	}

	// Limits.
	{
		char big[600];

		memset(big, 'x', sizeof(big) - 1);
		big[sizeof(big) - 1] = 0;
		big[255] = 0;
		check("identifier of 255 characters", qtn_file_set_identifier(q2, big) == 0);
		big[255] = 'x';
		big[256] = 0;
		check("identifier of 256 characters: ERANGE", qtn_file_set_identifier(q2, big) == ERANGE);
		check("metadata of 64 bytes", qtn_file_set_metadata(q2, big, 64) == 0);
		check("metadata of 65 bytes: ERANGE", qtn_file_set_metadata(q2, big, 65) == ERANGE);
	}
	// quirk_plain_flags_get_0x80: flags 0, 1 and 2 are written with 0x80 (0 as 0x81).
	{
		static const struct { uint32_t in; const char *out; } t[] = {
			{ 0, "0081" }, { 1, "0081" }, { 2, "0082" }, { 4, "0004" }, { 0x40, "0040" },
			{ 0x41, "0041" }, { 0x80, "0080" }, { 0x83, "0083" }, { 0x100, "0100" },
		};
		int ok = 1;

		for (unsigned i = 0; i < sizeof(t) / sizeof(t[0]); i++) {
			qtn_file_t f = qtn_file_alloc();

			unlink(path("e"));
			touch("e");
			qtn_file_set_flags(f, t[i].in);
			ok &= qtn_file_apply_to_path(f, path("e")) == 0 && strncmp(attr(path("e")), t[i].out, 4) == 0;
			qtn_file_free(f);
		}
		check("quirk_plain_flags_get_0x80: flags as written", ok);
		unlink(path("e"));
		touch("e");
		q2 = qtn_file_alloc();
		qtn_file_apply_to_path(q2, path("e"));
		check("a fresh record is written as 0081;00000000;;", strcmp(attr(path("e")), "0081;00000000;;") == 0);
	}

	// Hand-written attributes.
	set_attr(path("f"), "0083;5f000000;Safari;12345678-1234-1234-1234-123456789ABC");
	qtn_file_free(q2);
	q2 = qtn_file_alloc();
	check("init_with_path: full record", qtn_file_init_with_path(q2, path("f")) == 0
		&& qtn_file_get_flags(q2) == 0x83 && strcmp(qtn_file_get_identifier(q2), "Safari") == 0
		&& qtn_file_get_metadata_size(q2) == 36);
	touch("c");
	check("apply carries identifier and metadata", qtn_file_apply_to_path(q2, path("c")) == 0
		&& strcmp(attr(path("c")), attr(path("f"))) == 0);
	qtn_file_free(q2);
	q2 = qtn_file_alloc();
	set_attr(path("f"), "zzzz;5f000000;A;");
	check("init_with_path: bad flags: EINVAL", qtn_file_init_with_path(q2, path("f")) == EINVAL);
	set_attr(path("f"), "0041");
	check("init_with_path: flags only: EINVAL", qtn_file_init_with_path(q2, path("f")) == EINVAL);

	// Symbolic links: the path functions act on the link itself.
	touch("target");
	unlink(path("lnk"));
	symlink("target", path("lnk"));
	r = qtn_file_apply_to_path(q, path("lnk"));
	if (r == 0) {
		check("apply_to_path on a symlink marks the link, not the target",
			strcmp(attr(path("lnk")), "(none)") != 0 && strcmp(attr(path("target")), "(none)") == 0);
		q2 = qtn_file_alloc();
		check("init_with_path on a symlink reads the link", qtn_file_init_with_path(q2, path("lnk")) == 0);
	} else {
		printf("SKIP: symlink attributes (apply_to_path returned %d)\n", r);
	}

	// copyfile(3) carries the record, also with COPYFILE_DATA alone, as observed on macOS.
	set_attr(path("f"), "0083;5f000000;Safari;12345678-1234-1234-1234-123456789ABC");
	r = copyfile(path("f"), path("copy_all"), NULL, COPYFILE_ALL);
	check("copyfile COPYFILE_ALL succeeds", r == 0);
	check("copyfile COPYFILE_ALL carries the record", strcmp(attr(path("copy_all")), attr(path("f"))) == 0);
	r = copyfile(path("f"), path("copy_data"), NULL, COPYFILE_DATA);
	check("copyfile COPYFILE_DATA carries the record too (applied whenever the source has one)", r == 0
		&& strcmp(attr(path("copy_data")), attr(path("f"))) == 0);
	r = copyfile(path("f"), path("copy_xattr"), NULL, COPYFILE_DATA | COPYFILE_XATTR);
	check("copyfile COPYFILE_XATTR carries the record", r == 0
		&& strcmp(attr(path("copy_xattr")), attr(path("f"))) == 0);

	qtn_file_free(q);
	qtn_file_free(q2);
	// Remove the scratch directory (it holds only the plain files and links made above).
	{
		DIR *d = opendir(dir);
		struct dirent *e;

		while (d && (e = readdir(d)))
			if (strcmp(e->d_name, ".") && strcmp(e->d_name, ".."))
				if (unlink(path(e->d_name)) != 0)
					rmdir(path(e->d_name));
		if (d)
			closedir(d);
		rmdir(dir);
	}
	printf("%s\n", failures ? "SOME FAILED" : "ALL PASSED");
	return failures ? 1 : 0;
}
