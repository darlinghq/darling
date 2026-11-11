// Copyright (C) 2026 Lubos Dolezel
//
// symlinkat() creates the link relative to its directory descriptor, not to
// the current directory.

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main(void)
{
	char dir[] = "/tmp/symlinkat-test.XXXXXX";
	char other[] = "/tmp/symlinkat-cwd.XXXXXX";
	char target[64];
	int failed = 0;

	if (!mkdtemp(dir) || !mkdtemp(other)) {
		perror("mkdtemp");
		return 1;
	}
	int dirfd = open(dir, O_RDONLY | O_DIRECTORY);
	if (dirfd < 0) {
		perror("open");
		return 1;
	}
	chdir(other);

	if (symlinkat("some-target", dirfd, "link") != 0) {
		printf("FAIL: symlinkat: %s\n", strerror(errno));
		failed = 1;
	} else {
		struct stat st;
		ssize_t n = readlinkat(dirfd, "link", target, sizeof(target) - 1);
		if (n < 0) {
			printf("FAIL: link not found in the directory: %s\n", strerror(errno));
			failed = 1;
		} else {
			target[n] = '\0';
			if (strcmp(target, "some-target") != 0) {
				printf("FAIL: link points to \"%s\"\n", target);
				failed = 1;
			}
		}
		if (lstat("link", &st) == 0) {
			printf("FAIL: link was created in the current directory\n");
			failed = 1;
			unlink("link");
		}
	}

	// AT_FDCWD still means the current directory.
	if (symlinkat("cwd-target", AT_FDCWD, "cwdlink") != 0) {
		printf("FAIL: symlinkat(AT_FDCWD): %s\n", strerror(errno));
		failed = 1;
	} else {
		ssize_t n = readlink("cwdlink", target, sizeof(target) - 1);
		if (n < 0) {
			printf("FAIL: AT_FDCWD link not in the current directory\n");
			failed = 1;
		}
		unlink("cwdlink");
	}

	unlinkat(dirfd, "link", 0);
	close(dirfd);
	chdir("/");
	rmdir(dir);
	rmdir(other);

	if (!failed)
		printf("PASS\n");
	return failed;
}
