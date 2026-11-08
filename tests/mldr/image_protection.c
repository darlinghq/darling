// Host-side test of mldr's image snapshots: the copy has the file's contents,
// is sealed against every change and does not follow later changes of the
// file; the source descriptor's offset is untouched; files that cannot be
// copied are refused with the documented errors.
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "image_protection.h"

static int failures;

static void check(int ok, const char* what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

// a new temporary file with `length` bytes of a known pattern
static int temp_file(size_t length)
{
	char path[] = "/tmp/mldr-image-protection.XXXXXX";
	int fd = mkstemp(path);
	if (fd < 0) {
		perror("mkstemp");
		exit(1);
	}
	unlink(path);
	for (size_t i = 0; i < length; i++) {
		unsigned char byte = (unsigned char)(i * 7 + 3);
		if (write(fd, &byte, 1) != 1) {
			perror("write");
			exit(1);
		}
	}
	return fd;
}

static int has_pattern(const unsigned char* data, size_t length)
{
	for (size_t i = 0; i < length; i++)
		if (data[i] != (unsigned char)(i * 7 + 3))
			return 0;
	return 1;
}

static void test_snapshot(void)
{
	// larger than the copy buffer, and not a multiple of it or of a page
	const size_t length = 100000;
	unsigned char* data = malloc(length);
	int fd = temp_file(length);

	lseek(fd, 1234, SEEK_SET);
	int snapshot = mldr_snapshot_image(fd);
	check(snapshot >= 0, "snapshot of a regular file");
	if (snapshot < 0) {
		printf("  error %d (%s)\n", -snapshot, strerror(-snapshot));
		return;
	}

	check(lseek(fd, 0, SEEK_CUR) == 1234, "the source offset is unchanged");
	check(lseek(snapshot, 0, SEEK_CUR) == 0, "the snapshot's offset is 0");
	check((fcntl(snapshot, F_GETFD) & FD_CLOEXEC) != 0, "the snapshot is close-on-exec");

	struct stat st;
	check(fstat(snapshot, &st) == 0 && st.st_size == (off_t)length, "the snapshot has the file's size");
	check(pread(snapshot, data, length, 0) == (ssize_t)length && has_pattern(data, length),
		"the snapshot has the file's contents");

	int seals = fcntl(snapshot, F_GET_SEALS);
	int all = F_SEAL_SEAL | F_SEAL_SHRINK | F_SEAL_GROW | F_SEAL_WRITE;
	check(seals >= 0 && (seals & all) == all, "the snapshot is sealed (seal, shrink, grow, write)");

	errno = 0;
	check(pwrite(snapshot, "x", 1, 0) < 0 && errno == EPERM, "writing to the snapshot fails with EPERM");
	errno = 0;
	check(ftruncate(snapshot, 10) < 0 && errno == EPERM, "shrinking the snapshot fails with EPERM");
	errno = 0;
	check(ftruncate(snapshot, (off_t)length * 2) < 0 && errno == EPERM, "growing the snapshot fails with EPERM");
	errno = 0;
	check(fcntl(snapshot, F_ADD_SEALS, 0) < 0 && errno == EPERM, "the seals cannot be changed");
	errno = 0;
	check(mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, snapshot, 0) == MAP_FAILED && errno == EPERM,
		"the snapshot cannot be mapped shared and writable");

	// what the loader does: a private mapping at the image's offset
	long page = sysconf(_SC_PAGESIZE);
	unsigned char* mapped = mmap(NULL, length - (size_t)page, PROT_READ, MAP_PRIVATE, snapshot, page);
	check(mapped != MAP_FAILED, "the snapshot can be mapped privately");

	// code and data segments: executable, and private writable mappings
	void* text = mmap(NULL, (size_t)page, PROT_READ | PROT_EXEC, MAP_PRIVATE, snapshot, 0);
	check(text != MAP_FAILED, "the snapshot can be mapped executable");
	if (text != MAP_FAILED)
		munmap(text, (size_t)page);
	unsigned char* data_map = mmap(NULL, (size_t)page, PROT_READ | PROT_WRITE, MAP_PRIVATE, snapshot, 0);
	check(data_map != MAP_FAILED, "the snapshot can be mapped private and writable");
	if (data_map != MAP_FAILED) {
		data_map[0] ^= 0xff;
		munmap(data_map, (size_t)page);
		check(pread(snapshot, data, 1, 0) == 1 && data[0] == 3, "writes to a private mapping do not reach the snapshot");
	}

	// change and shorten the file: neither the snapshot nor its mapping follows
	unsigned char zeroes[4096] = { 0 };
	pwrite(fd, zeroes, sizeof(zeroes), page);
	ftruncate(fd, page * 2);
	check(pread(snapshot, data, length, 0) == (ssize_t)length && has_pattern(data, length),
		"the snapshot does not follow changes of the file");
	if (mapped != MAP_FAILED) {
		int same = 1;
		for (size_t i = 0; i < length - (size_t)page; i++)
			if (mapped[i] != (unsigned char)((i + (size_t)page) * 7 + 3))
				same = 0;
		check(same, "the mapping of the snapshot does not follow changes of the file");
		munmap(mapped, length - (size_t)page);
	}

	close(snapshot);
	close(fd);
	free(data);
}

static void test_errors(void)
{
	int fd = temp_file(0);
	check(mldr_snapshot_image(fd) == -EINVAL, "an empty file: EINVAL");

	// sparse, nothing is copied (a 32-bit off_t cannot describe such a file)
	if (sizeof(off_t) >= 8) {
		check(ftruncate(fd, (off_t)MLDR_SNAPSHOT_IMAGE_MAX + 1) == 0, "a sparse file above the limit");
		check(mldr_snapshot_image(fd) == -EFBIG, "a file above the limit: EFBIG");
	}
	close(fd);

	int dir = open("/tmp", O_RDONLY | O_DIRECTORY);
	check(mldr_snapshot_image(dir) == -EINVAL, "a directory: EINVAL");
	close(dir);

	int fds[2];
	if (pipe(fds) == 0) {
		write(fds[1], "abc", 3);
		check(mldr_snapshot_image(fds[0]) == -EINVAL, "a pipe: EINVAL");
		close(fds[0]);
		close(fds[1]);
	}

	check(mldr_snapshot_image(-1) == -EBADF, "a bad descriptor: EBADF");
}

int main(void)
{
	setvbuf(stdout, NULL, _IOLBF, 0);
	test_snapshot();
	test_errors();
	printf("%s\n", failures ? "FAILED" : "ALL PASSED");
	return failures ? 1 : 0;
}
