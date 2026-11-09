#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
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
	assert(darlingClassifyPrefix(root) == DARLING_PREFIX_STATE_EMPTY);
	snprintf(path, sizeof(path), "%s/user-data", root);
	makeFile(path);
	assert(darlingClassifyPrefix(root) == DARLING_PREFIX_STATE_UNINITIALIZED);

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
	assert(darlingClassifyPrefix(root) == DARLING_PREFIX_STATE_INITIALIZED);

	// A symlink in place of any marker must not make an unrelated tree look initialized.
	const char* directoryMarkers[] = {"Volumes", "usr", "var/run"};
	for (size_t i = 0; i < sizeof(directoryMarkers) / sizeof(*directoryMarkers); ++i) {
		snprintf(path, sizeof(path), "%s/%s", root, directoryMarkers[i]);
		assert(rmdir(path) == 0);
		char target[4096];
		snprintf(target, sizeof(target), "%s/%s", root,
			strcmp(directoryMarkers[i], "Volumes") == 0 ? "usr" : "Volumes");
		assert(symlink(target, path) == 0);
		assert(darlingClassifyPrefix(root) == DARLING_PREFIX_STATE_UNINITIALIZED);
		assert(unlink(path) == 0);
		makeDirectory(path);
	}

	const char* fileMarkers[] = {
		"private/etc/passwd", "private/etc/master.passwd", "private/etc/group"
	};
	for (size_t i = 0; i < sizeof(fileMarkers) / sizeof(*fileMarkers); ++i) {
		snprintf(path, sizeof(path), "%s/%s", root, fileMarkers[i]);
		assert(unlink(path) == 0);
		char target[4096];
		snprintf(target, sizeof(target), "%s/private/etc/group", root);
		if (strcmp(fileMarkers[i], "private/etc/group") == 0)
			snprintf(target, sizeof(target), "%s/private/etc/passwd", root);
		assert(symlink(target, path) == 0);
		assert(darlingClassifyPrefix(root) == DARLING_PREFIX_STATE_UNINITIALIZED);
		assert(unlink(path) == 0);
		makeFile(path);
	}

	// Parent directories are independently required to be real directories.
	char parentPath[4096], realPath[4096];
	snprintf(parentPath, sizeof(parentPath), "%s/var", root);
	snprintf(realPath, sizeof(realPath), "%s/var-real", root);
	assert(rename(parentPath, realPath) == 0);
	assert(symlink(realPath, parentPath) == 0);
	assert(darlingClassifyPrefix(root) == DARLING_PREFIX_STATE_UNINITIALIZED);
	assert(unlink(parentPath) == 0);
	assert(rename(realPath, parentPath) == 0);
	assert(darlingClassifyPrefix(root) == DARLING_PREFIX_STATE_INITIALIZED);

	snprintf(parentPath, sizeof(parentPath), "%s/private/etc", root);
	snprintf(realPath, sizeof(realPath), "%s/private/etc-real", root);
	assert(rename(parentPath, realPath) == 0);
	assert(symlink(realPath, parentPath) == 0);
	assert(darlingClassifyPrefix(root) == DARLING_PREFIX_STATE_UNINITIALIZED);
	assert(unlink(parentPath) == 0);
	assert(rename(realPath, parentPath) == 0);
	assert(darlingClassifyPrefix(root) == DARLING_PREFIX_STATE_INITIALIZED);

	// lstat of the final marker also follows symlinked parents, so reject those too.
	snprintf(path, sizeof(path), "%s/private-real", root);
	char privatePath[4096];
	snprintf(privatePath, sizeof(privatePath), "%s/private", root);
	assert(rename(privatePath, path) == 0);
	assert(symlink(path, privatePath) == 0);
	assert(darlingClassifyPrefix(root) == DARLING_PREFIX_STATE_UNINITIALIZED);
	assert(unlink(privatePath) == 0);
	assert(rename(path, privatePath) == 0);
	assert(darlingClassifyPrefix(root) == DARLING_PREFIX_STATE_INITIALIZED);

	puts("PASS: empty prefixes initialize; partial and symlinked marker trees fail closed; initialized prefixes remain usable");
	return 0;
}
