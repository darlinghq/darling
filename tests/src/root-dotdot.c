// Copyright (C) 2026 Lubos Dolezel
//
// ".." in the root directory refers to the root directory itself (POSIX
// pathname resolution). chdir("/..") and stat() of "/..", "/../.." and
// "/../tmp" must behave as if the ".." were not there.

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>

static int failed;

static void same_as(const char *path, const struct stat *ref, const char *refname)
{
	struct stat st;
	if (stat(path, &st) != 0) {
		printf("FAIL: stat(\"%s\"): %s\n", path, strerror(errno));
		failed = 1;
	} else if (st.st_dev != ref->st_dev || st.st_ino != ref->st_ino) {
		printf("FAIL: \"%s\" is not the same file as \"%s\"\n", path, refname);
		failed = 1;
	}
}

int main(void)
{
	struct stat root, tmp;
	char cwd[MAXPATHLEN];

	if (stat("/", &root) != 0 || stat("/tmp", &tmp) != 0) {
		perror("stat");
		return 1;
	}

	same_as("/..", &root, "/");
	same_as("/../..", &root, "/");
	same_as("/../tmp", &tmp, "/tmp");
	same_as("/tmp/../..", &root, "/");

	if (chdir("/..") != 0) {
		printf("FAIL: chdir(\"/..\"): %s\n", strerror(errno));
		failed = 1;
	} else if (!getcwd(cwd, sizeof(cwd)) || strcmp(cwd, "/") != 0) {
		printf("FAIL: getcwd after chdir(\"/..\") is \"%s\"\n", cwd);
		failed = 1;
	}

	chdir("/");
	if (chdir("..") != 0) {
		printf("FAIL: chdir(\"..\") in /: %s\n", strerror(errno));
		failed = 1;
	} else
		same_as(".", &root, "/");

	// F_GETPATH of the root directory is "/".
	int fd = open("/", O_RDONLY | O_DIRECTORY);
	if (fd < 0 || fcntl(fd, F_GETPATH, cwd) != 0 || strcmp(cwd, "/") != 0) {
		printf("FAIL: F_GETPATH of / is \"%s\"\n", fd < 0 ? "" : cwd);
		failed = 1;
	}
	if (fd >= 0)
		close(fd);

	if (!failed)
		printf("PASS\n");
	return failed;
}
