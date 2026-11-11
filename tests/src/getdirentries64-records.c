// Copyright (C) 2026 Lubos Dolezel
//
// Walks the records the getdirentries64 syscall returns (the call readdir()
// is built on) for a directory with names of every length from 1 to 60,
// using several buffer sizes. Checks that every record starts on an 8-byte
// boundary, that d_reclen is a multiple of 8 and holds the NUL-terminated
// name, and that every entry is returned exactly once.

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#define NFILES 60
#define NAME_OFFSET 21 // offsetof(struct dirent, d_name) with 64-bit inodes

struct rec {
	uint64_t d_ino;
	uint64_t d_seekoff;
	uint16_t d_reclen;
	uint16_t d_namlen;
	uint8_t d_type;
	char d_name[];
};

static int failed;

static int walk(const char *dir, size_t bufsize)
{
	int seen[NFILES + 1] = { 0 };
	int dots = 0;
	char *buf = malloc(bufsize);
	int fd = open(dir, O_RDONLY | O_DIRECTORY);
	if (fd < 0 || !buf) {
		perror("open");
		return 1;
	}

	for (;;) {
		long base = 0;
		long n = syscall(SYS_getdirentries64, fd, buf, bufsize, &base);
		if (n < 0) {
			printf("FAIL: bufsize %zu: getdirentries64: %s\n", bufsize, strerror(errno));
			failed = 1;
			break;
		}
		if (n == 0)
			break;
		long pos = 0;
		while (pos < n) {
			struct rec *r = (struct rec *)(buf + pos);
			if (pos % 8 != 0) {
				printf("FAIL: bufsize %zu: record at offset %ld is not 8-byte aligned\n", bufsize, pos);
				failed = 1;
			}
			if (r->d_reclen == 0 || r->d_reclen % 8 != 0 || pos + r->d_reclen > n
				|| r->d_reclen < NAME_OFFSET + r->d_namlen + 1) {
				printf("FAIL: bufsize %zu: \"%.*s\": bad d_reclen %u (namlen %u)\n",
					bufsize, (int)r->d_namlen, r->d_name, r->d_reclen, r->d_namlen);
				failed = 1;
				break;
			}
			if (r->d_name[r->d_namlen] != '\0' || strlen(r->d_name) != r->d_namlen) {
				printf("FAIL: bufsize %zu: name not terminated at d_namlen %u\n", bufsize, r->d_namlen);
				failed = 1;
			}
			if (!strcmp(r->d_name, ".") || !strcmp(r->d_name, ".."))
				dots++;
			else if (r->d_namlen >= 1 && r->d_namlen <= NFILES)
				seen[r->d_namlen]++;
			else {
				printf("FAIL: bufsize %zu: unexpected entry \"%s\"\n", bufsize, r->d_name);
				failed = 1;
			}
			pos += r->d_reclen;
		}
		if (pos != n) {
			failed = 1;
			break;
		}
	}

	for (int i = 1; i <= NFILES; i++) {
		if (seen[i] != 1) {
			printf("FAIL: bufsize %zu: name of length %d seen %d times\n", bufsize, i, seen[i]);
			failed = 1;
		}
	}
	if (dots != 2) {
		printf("FAIL: bufsize %zu: . and .. seen %d times in total\n", bufsize, dots);
		failed = 1;
	}
	close(fd);
	free(buf);
	return 0;
}

int main(void)
{
	char dir[] = "/tmp/getdirentries64-test.XXXXXX";
	char path[256];
	if (!mkdtemp(dir)) {
		perror("mkdtemp");
		return 1;
	}
	for (int i = 1; i <= NFILES; i++) {
		char name[NFILES + 1];
		memset(name, 'a' + i % 26, i);
		name[i] = '\0';
		snprintf(path, sizeof(path), "%s/%s", dir, name);
		int fd = open(path, O_CREAT | O_WRONLY, 0644);
		if (fd < 0) {
			perror(path);
			return 1;
		}
		close(fd);
	}

	size_t sizes[] = { 128, 200, 256, 512, 1048, 4096, 65536 };
	for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++)
		walk(dir, sizes[i]);

	for (int i = 1; i <= NFILES; i++) {
		char name[NFILES + 1];
		memset(name, 'a' + i % 26, i);
		name[i] = '\0';
		snprintf(path, sizeof(path), "%s/%s", dir, name);
		unlink(path);
	}
	rmdir(dir);

	if (!failed)
		printf("PASS\n");
	return failed;
}
