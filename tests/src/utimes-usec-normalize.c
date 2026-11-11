// Copyright (C) 2026 Lubos Dolezel
// SPDX-License-Identifier: GPL-3.0-or-later
//
// utimes() and futimes() set microsecond timestamps, and carry a tv_usec
// outside 0..999999 into the seconds instead of failing.
// QUIRK test: macOS 27.0.1 (arm64 and x86_64 under Rosetta) gives exactly
// these results; Linux would return EINVAL for the out-of-range cases.

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>

static int failures;

static void test(const char* path, int use_fd, long sec, int usec, long exp_sec, long exp_nsec)
{
	struct timeval tv[2] = { { 1000, 0 }, { sec, usec } };
	struct stat st;
	int ret;

	if (use_fd) {
		int fd = open(path, O_RDONLY);
		ret = futimes(fd, tv);
		close(fd);
	} else {
		ret = utimes(path, tv);
	}

	if (ret != 0) {
		printf("FAIL %s({%ld, %d}): ", use_fd ? "futimes" : "utimes", sec, usec);
		fflush(stdout);
		perror(NULL);
		failures++;
		return;
	}

	stat(path, &st);
	int ok = st.st_mtimespec.tv_sec == exp_sec && st.st_mtimespec.tv_nsec == exp_nsec
		&& st.st_atimespec.tv_sec == 1000 && st.st_atimespec.tv_nsec == 0;
	printf("%s %s({%ld, %d}): mtime %ld.%09ld, expected %ld.%09ld\n",
		ok ? "PASS" : "FAIL", use_fd ? "futimes" : "utimes", sec, usec,
		(long) st.st_mtimespec.tv_sec, st.st_mtimespec.tv_nsec, exp_sec, exp_nsec);
	if (!ok)
		failures++;
}

int main(void)
{
	char path[] = "/tmp/utimes-test.XXXXXX";
	int fd = mkstemp(path);

	if (fd < 0) {
		perror("mkstemp");
		return 1;
	}
	close(fd);

	for (int use_fd = 0; use_fd < 2; use_fd++) {
		test(path, use_fd, 2000, 500000, 2000, 500000000);
		test(path, use_fd, 2000, 999999, 2000, 999999000);
		test(path, use_fd, 2000, 1000000, 2001, 0);
		test(path, use_fd, 2000, 2500000, 2002, 500000000);
		test(path, use_fd, 2000, -1, 1999, 999999000);
		test(path, use_fd, 2000, -1500000, 1998, 500000000);
	}

	unlink(path);
	printf("%s\n", failures ? "FAILED" : "ALL PASSED");
	return failures != 0;
}
