/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * fsctl() and ffsctl() with the APFS requests that tools of a volume's files issue:
 *  - APFSIOC_GET_PARENT_SYNCROOT (_IOR('J', 74, uint64_t)): 0 and the value 0 for a file, a directory and a symbolic
 *    link (followed, or with FSOPT_NOFOLLOW) and for a descriptor of each; EINVAL for the root of a volume;
 *  - APFSIOC_MAINTAIN_DIR_STATS (_IOW('J', 2, uint64_t)): 0 for a directory, EINVAL for any other file;
 *  - for both, ENOENT for a missing path and EBADF for a bad descriptor come first, EFAULT for a NULL data pointer next
 *    (also when the object is a file for MAINTAIN_DIR_STATS), then the kind of object;
 *  - any other APFS request is ENOTTY (the other file system commands are in fsctl-unsupported.c), FSIOC_FD_ONLY_OPEN_ONCE
 *    is still EINVAL for fsctl, and FSIOC_CAS_BSDFLAGS still works.
 * Every expected value was observed on macOS 26.7.3 (APFS); the program also passes there.
 */
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

extern int fsctl(const char* path, unsigned long request, void* data, unsigned int options);
extern int ffsctl(int fd, unsigned long request, void* data, unsigned int options);

#define APFSIOC_GET_PARENT_SYNCROOT 0x40084a4aUL
#define APFSIOC_MAINTAIN_DIR_STATS 0x80084a02UL
#define FSIOC_FD_ONLY_OPEN_ONCE 0xC0044115UL
#define FSIOC_CAS_BSDFLAGS 0xC00C4114UL
#define HFS_COMMAND 0xC004682FUL
#define FSOPT_NOFOLLOW 1

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s (errno %d)\n", __LINE__, #cond, errno); failures++; } } while (0)

#define UNSET 0xAAAAAAAAAAAAAAAAULL

static int get_sync_root(const char* path, unsigned options, uint64_t* value)
{
	*value = UNSET;
	errno = 0;
	return fsctl(path, APFSIOC_GET_PARENT_SYNCROOT, value, options);
}

static int maintain(const char* path, unsigned options)
{
	uint64_t value = 0;
	errno = 0;
	return fsctl(path, APFSIOC_MAINTAIN_DIR_STATS, &value, options);
}

int main(void)
{
	char dir[] = "/tmp/fsctl-apfs-XXXXXX";
	char file[256], sub[256], link[256], missing[256];
	uint64_t value;

	CHECK(mkdtemp(dir) != NULL);
	snprintf(file, sizeof(file), "%s/file", dir);
	snprintf(sub, sizeof(sub), "%s/sub", dir);
	snprintf(link, sizeof(link), "%s/link", dir);
	snprintf(missing, sizeof(missing), "%s/missing", dir);
	close(open(file, O_CREAT | O_WRONLY, 0644));
	CHECK(mkdir(sub, 0755) == 0);
	CHECK(symlink("file", link) == 0);

	// GET_PARENT_SYNCROOT: no sync roots, value 0, for any object that is not the root of a volume
	CHECK(get_sync_root(dir, 0, &value) == 0 && value == 0);
	CHECK(get_sync_root(sub, 0, &value) == 0 && value == 0);
	CHECK(get_sync_root(file, 0, &value) == 0 && value == 0);
	CHECK(get_sync_root(link, 0, &value) == 0 && value == 0);
	CHECK(get_sync_root(link, FSOPT_NOFOLLOW, &value) == 0 && value == 0);
	CHECK(get_sync_root("/", 0, &value) == -1 && errno == EINVAL && value == UNSET);
	CHECK(get_sync_root(missing, 0, &value) == -1 && errno == ENOENT && value == UNSET);
	errno = 0;
	CHECK(fsctl(dir, APFSIOC_GET_PARENT_SYNCROOT, NULL, 0) == -1 && errno == EFAULT);
	errno = 0;
	CHECK(fsctl(file, APFSIOC_GET_PARENT_SYNCROOT, NULL, 0) == -1 && errno == EFAULT);
	errno = 0;
	CHECK(fsctl(missing, APFSIOC_GET_PARENT_SYNCROOT, NULL, 0) == -1 && errno == ENOENT);
	errno = 0;
	CHECK(fsctl(NULL, APFSIOC_GET_PARENT_SYNCROOT, &value, 0) == -1 && errno == EFAULT);

	// MAINTAIN_DIR_STATS: a directory is accepted, a file is not
	CHECK(maintain(dir, 0) == 0);
	CHECK(maintain(sub, 0) == 0);
	CHECK(maintain(file, 0) == -1 && errno == EINVAL);
	CHECK(maintain(missing, 0) == -1 && errno == ENOENT);
	{
		uint64_t zero = 0;
		errno = 0;
		CHECK(fsctl(dir, APFSIOC_MAINTAIN_DIR_STATS, NULL, 0) == -1 && errno == EFAULT);
		errno = 0;
		CHECK(fsctl(file, APFSIOC_MAINTAIN_DIR_STATS, NULL, 0) == -1 && errno == EFAULT);
		errno = 0;
		CHECK(fsctl(missing, APFSIOC_MAINTAIN_DIR_STATS, NULL, 0) == -1 && errno == ENOENT);
		CHECK(zero == 0);
	}

	// the same through descriptors
	{
		int dfd = open(dir, O_RDONLY | O_DIRECTORY);
		int ffd = open(file, O_RDONLY);
		CHECK(dfd >= 0 && ffd >= 0);
		value = UNSET;
		CHECK(ffsctl(dfd, APFSIOC_GET_PARENT_SYNCROOT, &value, 0) == 0 && value == 0);
		value = UNSET;
		CHECK(ffsctl(ffd, APFSIOC_GET_PARENT_SYNCROOT, &value, 0) == 0 && value == 0);
		errno = 0;
		CHECK(ffsctl(dfd, APFSIOC_GET_PARENT_SYNCROOT, NULL, 0) == -1 && errno == EFAULT);
		value = 0;
		CHECK(ffsctl(dfd, APFSIOC_MAINTAIN_DIR_STATS, &value, 0) == 0);
		errno = 0;
		CHECK(ffsctl(ffd, APFSIOC_MAINTAIN_DIR_STATS, &value, 0) == -1 && errno == EINVAL);
		errno = 0;
		CHECK(ffsctl(ffd, APFSIOC_MAINTAIN_DIR_STATS, NULL, 0) == -1 && errno == EFAULT);
		errno = 0;
		CHECK(ffsctl(dfd, APFSIOC_MAINTAIN_DIR_STATS, NULL, 0) == -1 && errno == EFAULT);
		int rootfd = open("/", O_RDONLY);
		CHECK(rootfd >= 0);
		value = UNSET;
		errno = 0;
		CHECK(ffsctl(rootfd, APFSIOC_GET_PARENT_SYNCROOT, &value, 0) == -1 && errno == EINVAL && value == UNSET);
		errno = 0;
		CHECK(ffsctl(-1, APFSIOC_GET_PARENT_SYNCROOT, &value, 0) == -1 && errno == EBADF);
		errno = 0;
		CHECK(ffsctl(-1, APFSIOC_MAINTAIN_DIR_STATS, &value, 0) == -1 && errno == EBADF);
		close(rootfd);
		close(dfd);
		close(ffd);
	}

	// everything else is as it was
	errno = 0;
	CHECK(fsctl(dir, APFSIOC_GET_PARENT_SYNCROOT + 1, &value, 0) == -1 && errno == ENOTTY);
	errno = 0;
	CHECK(fsctl(dir, FSIOC_FD_ONLY_OPEN_ONCE, &value, 0) == -1 && errno == EINVAL);
	errno = 0;
	CHECK(fsctl(missing, HFS_COMMAND, &value, 0) == -1 && errno == ENOENT);
	{
		int ffd = open(file, O_RDONLY);
		errno = 0;
		CHECK(ffsctl(ffd, APFSIOC_GET_PARENT_SYNCROOT + 1, &value, 0) == -1 && errno == ENOTTY);
		close(ffd);
	}
	{
		// FSIOC_CAS_BSDFLAGS with flags that are already what is expected changes nothing and reports them
		unsigned int cas[3] = { 0, 0, 0xFFFFFFFFu };
		CHECK(fsctl(file, FSIOC_CAS_BSDFLAGS, cas, 0) == 0 && cas[2] == 0);
	}

	unlink(link);
	unlink(file);
	rmdir(sub);
	rmdir(dir);
	printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
	return failures != 0;
}
