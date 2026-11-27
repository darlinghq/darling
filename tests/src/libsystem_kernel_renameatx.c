/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * renamex_np(2) / renameatx_np(2): RENAME_SWAP exchanges two existing names atomically, RENAME_EXCL fails with
 * EEXIST rather than replacing the target, both together are EINVAL.
 */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s (errno %d)\n", __LINE__, #cond, errno); failures++; } } while (0)

static void put(const char* path, const char* text)
{
	int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	write(fd, text, strlen(text));
	close(fd);
}

static int has(const char* path, const char* text)
{
	char buf[32] = { 0 };
	int fd = open(path, O_RDONLY);
	if (fd < 0)
		return 0;
	read(fd, buf, sizeof(buf) - 1);
	close(fd);
	return strcmp(buf, text) == 0;
}

int main(void)
{
	char dir[] = "/tmp/renameatx.XXXXXX";
	char a[64], b[64], c[64];
	int dfd;

	CHECK(mkdtemp(dir) != NULL);
	snprintf(a, sizeof(a), "%s/a", dir);
	snprintf(b, sizeof(b), "%s/b", dir);
	snprintf(c, sizeof(c), "%s/c", dir);
	put(a, "A");
	put(b, "B");

	errno = 0;
	CHECK(renamex_np(a, b, RENAME_SWAP) == 0);
	CHECK(has(a, "B") && has(b, "A"));

	errno = 0;
	CHECK(renamex_np(a, b, RENAME_EXCL) == -1 && errno == EEXIST);
	CHECK(has(a, "B") && has(b, "A"));

	errno = 0;
	CHECK(renamex_np(a, c, RENAME_EXCL) == 0);
	CHECK(access(a, F_OK) == -1 && has(c, "B"));

	errno = 0;
	CHECK(renamex_np(c, b, RENAME_SWAP | RENAME_EXCL) == -1 && errno == EINVAL);

	errno = 0;
	CHECK(renamex_np(a, b, RENAME_SWAP) == -1 && errno == ENOENT);

	/* the *at form, relative to a directory descriptor */
	dfd = open(dir, O_RDONLY | O_DIRECTORY);
	CHECK(dfd >= 0);
	errno = 0;
	CHECK(renameatx_np(dfd, "b", dfd, "c", RENAME_SWAP) == 0);
	CHECK(has(b, "B") && has(c, "A"));
	errno = 0;
	CHECK(renameatx_np(dfd, "b", dfd, "c", RENAME_EXCL) == -1 && errno == EEXIST);
	close(dfd);

	unlink(b);
	unlink(c);
	rmdir(dir);
	if (failures == 0)
		puts("PASS");
	return failures != 0;
}
