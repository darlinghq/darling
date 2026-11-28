/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * chflags(2) / fchflags(2): the user flags (UF_NODUMP, UF_HIDDEN, UF_TRACKED, ...) are stored, reported as st_flags by
 * stat/lstat/fstat/fstatat and as ATTR_CMN_FLAGS by getattrlist, and replaced by FSIOC_CAS_BSDFLAGS only when the
 * expected value matches (<sys/fsctl.h>; a mismatch is not an error and reports the flags found). Expected results
 * as observed on macOS 26.7.3. Before the change chflags failed with ENOTSUP.
 */
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

struct fsioc_cas_bsdflags { uint32_t expected_flags, new_flags, actual_flags; };
#define FSIOC_CAS_BSDFLAGS 0xC00C4114UL
extern int ffsctl(int, unsigned long, void*, unsigned int);
extern int fsctl(const char*, unsigned long, void*, unsigned int);

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s (errno %d)\n", __LINE__, #cond, errno); failures++; } } while (0)

static uint32_t attr_flags(const char* path)
{
	struct { uint32_t len; attribute_set_t returned; uint32_t flags; } __attribute__((packed)) ab;
	memset(&ab, 0, sizeof ab);
	ab.flags = 0xdeadbeef;
	struct attrlist al = { ATTR_BIT_MAP_COUNT, 0, ATTR_CMN_RETURNED_ATTRS | ATTR_CMN_FLAGS, 0, 0, 0, 0 };
	if (getattrlist(path, &al, &ab, sizeof ab, 0) != 0)
		return 0xdeadbeef;
	return ab.flags;
}

int main(void)
{
	char path[] = "/tmp/libsystem_kernel_chflags.XXXXXX";
	int fd = mkstemp(path);
	CHECK(fd >= 0);
	struct stat st;

	CHECK(fstat(fd, &st) == 0 && st.st_flags == 0);
	CHECK(chflags(path, UF_HIDDEN | UF_TRACKED) == 0);
	CHECK(stat(path, &st) == 0 && st.st_flags == (UF_HIDDEN | UF_TRACKED));
	CHECK(lstat(path, &st) == 0 && st.st_flags == (UF_HIDDEN | UF_TRACKED));
	CHECK(fstat(fd, &st) == 0 && st.st_flags == (UF_HIDDEN | UF_TRACKED));
	CHECK(fstatat(AT_FDCWD, path, &st, 0) == 0 && st.st_flags == (UF_HIDDEN | UF_TRACKED));
	CHECK(attr_flags(path) == (UF_HIDDEN | UF_TRACKED));

	CHECK(fchflags(fd, UF_NODUMP) == 0);
	CHECK(fstat(fd, &st) == 0 && st.st_flags == UF_NODUMP);
	CHECK(stat(path, &st) == 0 && st.st_flags == UF_NODUMP);
	CHECK(attr_flags(path) == UF_NODUMP);

	// the flags are not visible as an extended attribute
	char names[256];
	ssize_t n = listxattr(path, names, sizeof names, 0);
	CHECK(n >= 0 && n < (ssize_t) sizeof names);
	CHECK(n == 0 || strstr(names, "bsdflags") == NULL);

	// FSIOC_CAS_BSDFLAGS: replaced if the expected flags are current ...
	struct fsioc_cas_bsdflags cas = { .expected_flags = UF_NODUMP, .new_flags = UF_HIDDEN, .actual_flags = 0 };
	CHECK(ffsctl(fd, FSIOC_CAS_BSDFLAGS, &cas, 0) == 0);
	CHECK(cas.actual_flags == UF_NODUMP);
	CHECK(fstat(fd, &st) == 0 && st.st_flags == UF_HIDDEN);
	// ... and left alone when they are not
	cas.expected_flags = 0x77;
	cas.new_flags = 0;
	CHECK(ffsctl(fd, FSIOC_CAS_BSDFLAGS, &cas, 0) == 0);
	CHECK(cas.actual_flags == UF_HIDDEN);
	CHECK(fstat(fd, &st) == 0 && st.st_flags == UF_HIDDEN);
	// the path variant
	cas.expected_flags = UF_HIDDEN;
	cas.new_flags = UF_TRACKED;
	CHECK(fsctl(path, FSIOC_CAS_BSDFLAGS, &cas, 0) == 0);
	CHECK(cas.actual_flags == UF_HIDDEN);
	CHECK(stat(path, &st) == 0 && st.st_flags == UF_TRACKED);

	// clearing the flags
	CHECK(chflags(path, 0) == 0);
	CHECK(stat(path, &st) == 0 && st.st_flags == 0);
	CHECK(attr_flags(path) == 0);
	// a path that does not exist
	CHECK(chflags("/tmp/libsystem_kernel_chflags.does-not-exist", UF_HIDDEN) == -1 && errno == ENOENT);

	close(fd);
	unlink(path);
	printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
	return failures != 0;
}
