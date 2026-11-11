/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * A successful opendir()/fdopendir() leaves errno as the caller set it.
 */
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

static int failures;

static void check(const char *what, int cond)
{
	printf("%s: %s\n", cond ? "PASS" : "FAIL", what);
	if (!cond)
		failures++;
}

int main(void)
{
	DIR *d;
	int fd, n = 0;

	errno = 0;
	d = opendir("/");
	check("opendir(\"/\") succeeds", d != NULL);
	check("errno still 0 after opendir", errno == 0);
	if (d) {
		while (readdir(d) != NULL)
			n++;
		check("readdir returns entries", n > 0);
		closedir(d);
	}

	errno = EINTR;
	d = opendir("/tmp");
	check("errno still EINTR after opendir", d != NULL && errno == EINTR);
	if (d)
		closedir(d);

	fd = open("/", O_RDONLY | O_DIRECTORY);
	errno = 0;
	d = fd >= 0 ? fdopendir(fd) : NULL;
	check("errno still 0 after fdopendir", d != NULL && errno == 0);
	if (d)
		closedir(d);

	errno = 0;
	d = opendir("/nonexistent-opendir-errno-test");
	check("failing opendir sets ENOENT", d == NULL && errno == ENOENT);

	return failures != 0;
}
