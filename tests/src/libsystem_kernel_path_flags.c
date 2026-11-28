/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Path and open flags, expected results as observed on macOS 26.7.3 (black-box):
 *  - "/.nofollow/<path>" names <path> (the path-prefix form of O_NOFOLLOW_ANY): stat/open of a plain file and
 *    directory work through it, lstat of a symlink reports the symlink;
 *  - open(O_SYMLINK) of a regular file or a directory is an ordinary open (read, write and fstat work); of a symlink
 *    it opens the link itself (fstat works, read fails);
 *  - renamex_np accepts RENAME_SECLUDE (1), RENAME_NOFOLLOW_ANY (0x10) and RENAME_RESOLVE_BENEATH (0x20, relative
 *    paths; an absolute path is ENOTCAPABLE on macOS and is not refused here), and a flag bit outside these and RENAME_SWAP / RENAME_EXCL is EINVAL.
 * Before the change: the /.nofollow/ paths were ENOENT, a file opened with O_SYMLINK could not be read, the newer
 * rename flags were ENOTSUP.
 */
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

extern int renamex_np(const char*, const char*, unsigned int);

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s (errno %d)\n", __LINE__, #cond, errno); failures++; } } while (0)

static void touch(const char* path, const char* text)
{
	int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd >= 0)
	{
		write(fd, text, strlen(text));
		close(fd);
	}
}

int main(void)
{
	char tmpl[] = "/tmp/libsystem_kernel_path_flags.XXXXXX";
	char* made = mkdtemp(tmpl);
	CHECK(made != NULL);
	char dir[PATH_MAX];
	CHECK(realpath(made, dir) != NULL); // no symlink anywhere in the path
	char file[PATH_MAX + 16], link[PATH_MAX + 16], sub[PATH_MAX + 16], np[PATH_MAX + 32];
	snprintf(file, sizeof file, "%s/file", dir);
	snprintf(link, sizeof link, "%s/link", dir);
	snprintf(sub, sizeof sub, "%s/sub", dir);
	touch(file, "hello");
	CHECK(symlink("file", link) == 0);
	CHECK(mkdir(sub, 0755) == 0);

	// /.nofollow/
	struct stat st;
	snprintf(np, sizeof np, "/.nofollow%s", file);
	CHECK(stat(np, &st) == 0 && S_ISREG(st.st_mode) && st.st_size == 5);
	int fd = open(np, O_RDONLY);
	CHECK(fd >= 0);
	char buf[8] = { 0 };
	CHECK(fd >= 0 && read(fd, buf, 5) == 5 && memcmp(buf, "hello", 5) == 0);
	if (fd >= 0)
		close(fd);
	snprintf(np, sizeof np, "/.nofollow%s", sub);
	CHECK(stat(np, &st) == 0 && S_ISDIR(st.st_mode));
	snprintf(np, sizeof np, "/.nofollow%s", link);
	CHECK(lstat(np, &st) == 0 && S_ISLNK(st.st_mode));

	// O_SYMLINK
	fd = open(file, O_SYMLINK | O_RDWR);
	CHECK(fd >= 0);
	if (fd >= 0)
	{
		memset(buf, 0, sizeof buf);
		CHECK(pread(fd, buf, 5, 0) == 5 && memcmp(buf, "hello", 5) == 0);
		CHECK(pwrite(fd, "J", 1, 0) == 1);
		CHECK(fstat(fd, &st) == 0 && S_ISREG(st.st_mode));
		close(fd);
	}
	fd = open(sub, O_SYMLINK | O_RDONLY);
	CHECK(fd >= 0);
	if (fd >= 0)
	{
		CHECK(fstat(fd, &st) == 0 && S_ISDIR(st.st_mode));
		close(fd);
	}
	fd = open(link, O_SYMLINK | O_RDONLY);
	CHECK(fd >= 0);
	if (fd >= 0)
	{
		CHECK(fstat(fd, &st) == 0 && S_ISLNK(st.st_mode));
		CHECK(read(fd, buf, 5) == -1); // EPERM on macOS 26.7.3, EBADF here (an O_PATH descriptor)
		close(fd);
	}

	// renamex_np
	// relative names: RENAME_RESOLVE_BENEATH refuses an absolute path (ENOTCAPABLE) on macOS
	const char* from = "rfrom";
	const char* to = "rto";
	CHECK(chdir(dir) == 0);
	unsigned int ok_flags[] = { 0x1, 0x10, 0x20, 0x30, 0x11 };
	for (unsigned i = 0; i < sizeof ok_flags / sizeof *ok_flags; i++)
	{
		touch(from, "x");
		if (renamex_np(from, to, ok_flags[i]) != 0)
			printf("renamex_np flags 0x%x: errno %d\n", ok_flags[i], errno);
		CHECK(access(to, F_OK) == 0 && access(from, F_OK) == -1);
		unlink(from);
		unlink(to);
	}
	unsigned int bad_flags[] = { 0x8, 0x40, 0x100 };
	for (unsigned i = 0; i < sizeof bad_flags / sizeof *bad_flags; i++)
	{
		touch(from, "x");
		CHECK(renamex_np(from, to, bad_flags[i]) == -1 && errno == EINVAL);
		unlink(from);
	}

	CHECK(chdir("/") == 0);
	unlink(file);
	unlink(link);
	rmdir(sub);
	rmdir(dir);
	printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
	return failures != 0;
}
