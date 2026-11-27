/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * sendfile(2) (sys/socket.h): sends a file's bytes to a stream socket, between an optional header and trailer;
 * *len is the number of bytes sent, 0 as the length means up to the end of the file, and the offset selects
 * where in the file to start.
 */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s (errno %d)\n", __LINE__, #cond, errno); failures++; } } while (0)

static size_t drain(int fd, char* out, size_t cap)
{
	size_t got = 0;
	ssize_t n;
	while (got < cap && (n = recv(fd, out + got, cap - got, MSG_DONTWAIT)) > 0)
		got += n;
	return got;
}

int main(void)
{
	char path[] = "/tmp/sendfile-hdtr.XXXXXX";
	int fd = mkstemp(path), sv[2];
	char out[256];
	off_t len;
	struct iovec hdr[2] = { { "H1-", 3 }, { "H2-", 3 } };
	struct iovec trl[1] = { { "-T", 2 } };
	struct sf_hdtr hdtr = { hdr, 2, trl, 1 };

	CHECK(fd >= 0);
	CHECK(write(fd, "0123456789abcdef", 16) == 16);
	CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0);

	/* a part of the file, from an offset */
	len = 5;
	errno = 0;
	CHECK(sendfile(fd, sv[0], 4, &len, NULL, 0) == 0);
	CHECK(len == 5);
	memset(out, 0, sizeof(out));
	CHECK(drain(sv[1], out, sizeof(out)) == 5 && memcmp(out, "45678", 5) == 0);

	/* length 0: to the end of the file */
	len = 0;
	errno = 0;
	CHECK(sendfile(fd, sv[0], 10, &len, NULL, 0) == 0);
	CHECK(len == 6);
	memset(out, 0, sizeof(out));
	CHECK(drain(sv[1], out, sizeof(out)) == 6 && memcmp(out, "abcdef", 6) == 0);

	/* header and trailer around the file bytes; *len counts all of them */
	len = 0;
	errno = 0;
	CHECK(sendfile(fd, sv[0], 10, &len, &hdtr, 0) == 0);
	CHECK(len == 3 + 3 + 6 + 2);
	memset(out, 0, sizeof(out));
	CHECK(drain(sv[1], out, sizeof(out)) == 14 && memcmp(out, "H1-H2-abcdef-T", 14) == 0);

	/* a file larger than a socket buffer, with a non-blocking sender, reports what it sent */
	errno = 0;
	{
		off_t big = 1 << 22;
		char* block = calloc(1, 4096);
		for (int i = 0; i < (int) (big / 4096); i++)
			CHECK(write(fd, block, 4096) == 4096);
		free(block);
	}
	CHECK(fcntl(sv[0], F_SETFL, O_NONBLOCK) == 0);
	len = 0;
	errno = 0;
	int rc = sendfile(fd, sv[0], 0, &len, NULL, 0);
	CHECK(rc == -1 && errno == EAGAIN);
	CHECK(len > 0);

	/* errors */
	len = 1;
	errno = 0;
	CHECK(sendfile(-1, sv[0], 0, &len, NULL, 0) == -1 && errno == EBADF);
	errno = 0;
	CHECK(sendfile(fd, sv[0], 0, &len, NULL, 1) == -1 && errno == EINVAL);
	errno = 0;
	CHECK(sendfile(fd, -1, 0, &len, NULL, 0) == -1 && errno == EBADF);
	errno = 0;
	CHECK(sendfile(fd, sv[0], -1, &len, NULL, 0) == -1 && errno == EINVAL);

	close(fd);
	unlink(path);
	if (failures == 0)
		puts("PASS");
	return failures != 0;
}
