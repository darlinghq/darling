/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * kevent(2): for EVFILT_WRITE on a socket (or pipe), the returned data is
 * the amount of space available in the write buffer. A writable descriptor
 * must therefore report a positive value, bounded by the buffer size, and
 * the value must not grow when data is queued. Exact values differ between
 * systems, so only those relations are checked, for a connected idle TCP
 * socket, TCP after writes, connected and unconnected UDP sockets, stream
 * and datagram unix sockets, and a pipe.
 *
 * Runs on macOS and in Darling; prints PASS and exits 0.
 */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

static int failures;

#define CHECK(cond, ...) do { \
	if (!(cond)) { \
		failures++; \
		printf("FAIL %s:%d: ", __FILE__, __LINE__); \
		printf(__VA_ARGS__); \
		printf("\n"); \
	} \
} while (0)

/* Returns the EVFILT_WRITE data for fd, or -1 if no event was delivered. */
static long write_space(int fd, unsigned short flags, unsigned short *flags_out)
{
	int kq = kqueue();
	struct kevent ch, out;
	struct timespec ts = {2, 0};
	long data = -1;

	CHECK(kq >= 0, "kqueue: %s", strerror(errno));
	EV_SET(&ch, fd, EVFILT_WRITE, flags, 0, 0, NULL);
	memset(&out, 0, sizeof(out));
	int n = kevent(kq, &ch, 1, &out, 1, &ts);
	if (n == 1 && out.filter == EVFILT_WRITE && (int)out.ident == fd) {
		data = (long)out.data;
		if (flags_out)
			*flags_out = out.flags;
	}
	close(kq);
	return data;
}

static int sndbuf_of(int fd)
{
	int sb = 0;
	socklen_t sl = sizeof(sb);

	if (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &sb, &sl) < 0)
		return -1;
	return sb;
}

/* An idle writable descriptor: positive space, at most the buffer size. */
static void check_idle(const char *what, int fd, long max)
{
	unsigned short fl = 0;
	long d = write_space(fd, EV_ADD | EV_ENABLE | EV_CLEAR, &fl);

	CHECK(d > 0, "%s idle: data %ld, expected free buffer space > 0", what, d);
	CHECK(max < 0 || d <= max, "%s idle: data %ld exceeds buffer size %ld", what, d, max);
	CHECK(d < 0 || (fl & EV_CLEAR), "%s idle: EV_CLEAR not echoed (flags 0x%x)", what, fl);
}

static void inet_pair(int type, int *a, int *b)
{
	struct sockaddr_in sa;
	socklen_t len = sizeof(sa);
	int l = socket(AF_INET, type, 0);

	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	if (l < 0 || bind(l, (struct sockaddr *)&sa, sizeof(sa)) < 0 ||
	    getsockname(l, (struct sockaddr *)&sa, &len) < 0) {
		printf("FAIL setup inet socket: %s\n", strerror(errno));
		exit(1);
	}
	*a = socket(AF_INET, type, 0);
	if (type == SOCK_STREAM) {
		if (listen(l, 1) < 0 || connect(*a, (struct sockaddr *)&sa, sizeof(sa)) < 0 ||
		    (*b = accept(l, NULL, NULL)) < 0) {
			printf("FAIL setup tcp: %s\n", strerror(errno));
			exit(1);
		}
		close(l);
	} else {
		if (connect(*a, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
			printf("FAIL setup udp: %s\n", strerror(errno));
			exit(1);
		}
		*b = l;
	}
}

static void test_tcp(void)
{
	int a, b;
	char buf[65536];
	unsigned short fl;

	memset(buf, 'x', sizeof(buf));
	inet_pair(SOCK_STREAM, &a, &b);
	int sb = sndbuf_of(a);
	check_idle("tcp", a, sb);

	/* Level-triggered re-registration after a few writes. */
	CHECK(write(a, buf, 20000) == 20000, "tcp write: %s", strerror(errno));
	usleep(50000);
	long d = write_space(a, EV_ADD | EV_ENABLE, &fl);
	CHECK(d > 0 && d <= sb, "tcp after writes: data %ld (sndbuf %d)", d, sb);

	/* Fill the connection until it would block: no space is left. */
	fcntl(a, F_SETFL, O_NONBLOCK);
	while (write(a, buf, sizeof(buf)) > 0)
		;
	CHECK(errno == EAGAIN, "tcp fill ended with %s", strerror(errno));
	usleep(50000);
	d = write_space(a, EV_ADD | EV_ENABLE, NULL);
	CHECK(d <= 0 || d < 4096, "tcp full: data %ld, expected (almost) no space or no event", d);
	close(a);
	close(b);
}

static void test_udp(void)
{
	int a, b;

	inet_pair(SOCK_DGRAM, &a, &b);
	check_idle("udp connected", a, sndbuf_of(a));
	close(a);
	close(b);
	a = socket(AF_INET, SOCK_DGRAM, 0);
	check_idle("udp unconnected", a, sndbuf_of(a));
	close(a);
}

static void test_unix(void)
{
	int sv[2];
	char buf[2000];
	long before, after;

	memset(buf, 'x', sizeof(buf));
	CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0, "socketpair: %s", strerror(errno));
	check_idle("unix stream", sv[0], sndbuf_of(sv[0]));
	before = write_space(sv[0], EV_ADD | EV_ENABLE, NULL);
	CHECK(write(sv[0], buf, sizeof(buf)) == (ssize_t)sizeof(buf), "unix write: %s", strerror(errno));
	after = write_space(sv[0], EV_ADD | EV_ENABLE, NULL);
	CHECK(after > 0 && after < before, "unix stream after 2000 bytes: data %ld, before %ld", after, before);
	close(sv[0]);
	close(sv[1]);

	CHECK(socketpair(AF_UNIX, SOCK_DGRAM, 0, sv) == 0, "socketpair dgram: %s", strerror(errno));
	check_idle("unix dgram", sv[0], sndbuf_of(sv[0]));
	close(sv[0]);
	close(sv[1]);
}

static void test_pipe(void)
{
	int p[2];
	char buf[1000];
	long before, after;

	memset(buf, 'x', sizeof(buf));
	CHECK(pipe(p) == 0, "pipe: %s", strerror(errno));
	check_idle("pipe", p[1], -1);
	before = write_space(p[1], EV_ADD | EV_ENABLE, NULL);
	CHECK(write(p[1], buf, sizeof(buf)) == (ssize_t)sizeof(buf), "pipe write: %s", strerror(errno));
	after = write_space(p[1], EV_ADD | EV_ENABLE, NULL);
	CHECK(after > 0 && after < before, "pipe after 1000 bytes: data %ld, before %ld", after, before);
	close(p[0]);
	close(p[1]);
}

int main(void)
{
	test_tcp();
	test_udp();
	test_unix();
	test_pipe();
	if (failures) {
		printf("%d failure(s)\n", failures);
		return 1;
	}
	printf("PASS\n");
	return 0;
}
