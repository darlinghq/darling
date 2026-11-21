// Copyright (C) 2026 Lubos Dolezel
// File name lookup is normalization-insensitive, as on APFS: a name in its decomposed form (what Foundation's
// fileSystemRepresentation gives on macOS and in Darling) finds a file stored with the precomposed form (what Linux
// names usually are), and the other way round, without changing which names are distinct when both exist.
// See tests/path-normalization.md. Plain C: builds for the guest and on macOS, where it also passes.
// ARGS: "$(mktemp -d)/t"
// Usage: path_normalization <empty or new directory>. Prints PASS/FAIL lines; exits 1 on a failure.
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int failures;

static void check(int ok, const char *what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

static int make(const char *path, const char *contents)
{
	int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd < 0)
		return -1;
	ssize_t n = write(fd, contents, strlen(contents));
	close(fd);
	return n == (ssize_t) strlen(contents) ? 0 : -1;
}

static int contents_are(const char *path, const char *want)
{
	char buf[64] = { 0 };
	int fd = open(path, O_RDONLY);
	if (fd < 0)
		return 0;
	ssize_t n = read(fd, buf, sizeof(buf) - 1);
	close(fd);
	return n >= 0 && strcmp(buf, want) == 0;
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "usage: %s <directory>\n", argv[0]);
		return 2;
	}
	char dir[1024], a[1200], b[1200], c[1200];
	snprintf(dir, sizeof(dir), "%s", argv[1]);
	mkdir(dir, 0755);

	// u-umlaut precomposed (U+00FC, c3 bc) and decomposed (u, U+0308 cc 88)
	snprintf(a, sizeof(a), "%s/\xc3\xbc.txt", dir);
	snprintf(b, sizeof(b), "%s/u\xcc\x88.txt", dir);
	check(make(a, "one") == 0, "a file with a precomposed name is created");

	struct stat sa, sb;
	check(access(b, F_OK) == 0, "access() with the decomposed name finds it");
	check(stat(a, &sa) == 0 && stat(b, &sb) == 0 && sa.st_ino == sb.st_ino,
		"stat() with either name gives the same file");
	check(contents_are(b, "one"), "open() with the decomposed name reads it");
	errno = 0;
	int fd = open(b, O_WRONLY | O_CREAT | O_EXCL, 0644);
	check(fd < 0 && errno == EEXIST, "creating the decomposed name exclusively fails with EEXIST");
	if (fd >= 0)
		close(fd);

	// A folder with a precomposed name (D, i-diaeresis U+00EF, r) holding e-acute (U+00E9), looked up decomposed
	snprintf(c, sizeof(c), "%s/D\xc3\xafr", dir);
	mkdir(c, 0755);
	snprintf(c, sizeof(c), "%s/D\xc3\xafr/\xc3\xa9.txt", dir);
	make(c, "nested");
	snprintf(c, sizeof(c), "%s/Di\xcc\x88r/e\xcc\x81.txt", dir);
	check(contents_are(c, "nested"), "a decomposed path through a precomposed folder opens the file");

	// Not equivalent: v.txt is not u-umlaut
	snprintf(c, sizeof(c), "%s/v.txt", dir);
	errno = 0;
	check(open(c, O_RDONLY) < 0 && errno == ENOENT, "a different name is not found");
	// o-umlaut decomposed is not u-umlaut
	snprintf(c, sizeof(c), "%s/o\xcc\x88.txt", dir);
	errno = 0;
	check(open(c, O_RDONLY) < 0 && errno == ENOENT, "a different accented name is not found");

	// Invalid UTF-8 is matched by its exact bytes
	snprintf(c, sizeof(c), "%s/\xff.txt", dir);
	int made = make(c, "raw");
	if (made == 0)
		check(contents_are(c, "raw"), "a name with invalid UTF-8 is found by its bytes");
	else
		printf("SKIP: the file system does not take invalid UTF-8 names (errno %d)\n", errno);

	return failures != 0;
}
