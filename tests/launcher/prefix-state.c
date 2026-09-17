#define _GNU_SOURCE
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../../src/startup/prefix-state.h"

static void makeDirectory(const char* path)
{
	assert(mkdir(path, 0755) == 0);
}

static void makeFile(const char* path)
{
	int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0644);
	assert(fd >= 0);
	assert(close(fd) == 0);
}

int main(void)
{
	char root[] = "/tmp/darling-prefix-state-XXXXXX";
	assert(mkdtemp(root));

	char path[4096];
	assert(darlingClassifyPrefix(root) == DARLING_PREFIX_EMPTY);
	snprintf(path, sizeof(path), "%s/user-data", root);
	makeFile(path);
	assert(darlingClassifyPrefix(root) == DARLING_PREFIX_UNINITIALIZED);

	const char* dirs[] = {"Volumes", "usr", "var", "var/run", "private", "private/etc"};
	for (size_t i = 0; i < sizeof(dirs) / sizeof(*dirs); ++i) {
		snprintf(path, sizeof(path), "%s/%s", root, dirs[i]);
		makeDirectory(path);
	}
	const char* files[] = {"private/etc/passwd", "private/etc/master.passwd", "private/etc/group"};
	for (size_t i = 0; i < sizeof(files) / sizeof(*files); ++i) {
		snprintf(path, sizeof(path), "%s/%s", root, files[i]);
		makeFile(path);
	}
	assert(darlingClassifyPrefix(root) == DARLING_PREFIX_INITIALIZED);

	puts("PASS: empty prefixes initialize, partial non-empty prefixes fail closed, initialized prefixes remain usable");
	return 0;
}
