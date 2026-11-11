// Copyright (C) 2026 Lubos Dolezel
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Device numbers of character devices are reported in the Darwin dev_t
// encoding (major in bits 31..24), through every stat variant.
// Darling-specific expectation: /dev/null and /dev/zero come from the Linux
// host, where they are 1:3 and 1:5.

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

static int failures;

static void check(const char* what, const char* path, dev_t rdev, int maj, int min)
{
	int ok = major(rdev) == maj && minor(rdev) == min;
	printf("%s %s(%s): major %d minor %d, expected %d %d\n",
		ok ? "PASS" : "FAIL", what, path, major(rdev), minor(rdev), maj, min);
	if (!ok)
		failures++;
}

static void test(const char* path, int maj, int min)
{
	struct stat st;
	int fd;

	if (stat(path, &st) == 0) check("stat", path, st.st_rdev, maj, min);
	else { perror("stat"); failures++; }
	if (lstat(path, &st) == 0) check("lstat", path, st.st_rdev, maj, min);
	else { perror("lstat"); failures++; }
	if (fstatat(AT_FDCWD, path, &st, 0) == 0) check("fstatat", path, st.st_rdev, maj, min);
	else { perror("fstatat"); failures++; }

	fd = open(path, O_RDONLY);
	if (fd >= 0 && fstat(fd, &st) == 0) check("fstat", path, st.st_rdev, maj, min);
	else { perror("fstat"); failures++; }
	if (fd >= 0)
		close(fd);

	if (!S_ISCHR(st.st_mode)) {
		printf("FAIL %s is not a character device\n", path);
		failures++;
	}
}

int main(void)
{
	struct stat st;

	test("/dev/null", 1, 3);
	test("/dev/zero", 1, 5);

	// A directory has no device number.
	if (stat("/", &st) != 0 || st.st_rdev != 0) {
		printf("FAIL st_rdev of / is %#x\n", (unsigned) st.st_rdev);
		failures++;
	}

	printf("%s\n", failures ? "FAILED" : "ALL PASSED");
	return failures != 0;
}
