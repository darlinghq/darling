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
 * connect(2) on a non-blocking socket: a TCP connect that cannot complete at
 * once returns -1 with EINPROGRESS and the socket stays non-blocking. A
 * repeated connect() reports EALREADY while the attempt is pending and
 * EISCONN once it has completed. Completion is signalled as write readiness
 * (kqueue EVFILT_WRITE, select, poll) and SO_ERROR then reads 0, or the
 * pending error (ECONNREFUSED) for a refused attempt. Unix-domain sockets
 * and blocking sockets connect synchronously and return 0.
 *
 * Runs on macOS and in Darling; prints PASS and exits 0.
 */
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/event.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>

static int failures;

#define CHECK(cond, ...) do { \
	if (!(cond)) { \
		failures++; \
		printf("FAIL %s:%d: ", __FILE__, __LINE__); \
		printf(__VA_ARGS__); \
		printf("\n"); \
	} \
} while (0)

static int listener(struct sockaddr_in *addr)
{
	int ls = socket(AF_INET, SOCK_STREAM, 0);
	socklen_t len = sizeof(*addr);

	memset(addr, 0, sizeof(*addr));
	addr->sin_len = sizeof(*addr);
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	CHECK(ls >= 0, "socket: %s", strerror(errno));
	CHECK(bind(ls, (struct sockaddr *)addr, sizeof(*addr)) == 0, "bind: %s", strerror(errno));
	CHECK(listen(ls, 16) == 0, "listen: %s", strerror(errno));
	CHECK(getsockname(ls, (struct sockaddr *)addr, &len) == 0, "getsockname: %s", strerror(errno));
	return ls;
}

static int nb_socket(void)
{
	int s = socket(AF_INET, SOCK_STREAM, 0);

	CHECK(s >= 0, "socket: %s", strerror(errno));
	CHECK(fcntl(s, F_SETFL, fcntl(s, F_GETFL) | O_NONBLOCK) == 0, "F_SETFL: %s", strerror(errno));
	return s;
}

static int so_error(int s)
{
	int err = -1;
	socklen_t len = sizeof(err);

	CHECK(getsockopt(s, SOL_SOCKET, SO_ERROR, &err, &len) == 0, "SO_ERROR: %s", strerror(errno));
	return err;
}

static void check_still_nonblocking(int s, const char *what)
{
	CHECK(fcntl(s, F_GETFL) & O_NONBLOCK, "%s: O_NONBLOCK was lost", what);
}

/* Starts the non-blocking connect and checks the immediate result. */
static void start_connect(int s, const struct sockaddr_in *addr, const char *what)
{
	int r = connect(s, (const struct sockaddr *)addr, sizeof(*addr));
	int e = errno;

	CHECK(r == -1 && e == EINPROGRESS, "%s: connect returned %d errno=%d, expected -1/EINPROGRESS", what, r, r ? e : 0);
	check_still_nonblocking(s, what);
}

/* A second connect() is EALREADY while pending, EISCONN once completed. */
static void check_repeat_pending(int s, const struct sockaddr_in *addr, const char *what)
{
	int r = connect(s, (const struct sockaddr *)addr, sizeof(*addr));
	int e = errno;

	CHECK(r == -1 && (e == EALREADY || e == EISCONN), "%s: repeated connect returned %d errno=%d, expected EALREADY or EISCONN", what, r, r ? e : 0);
}

static void check_repeat_done(int s, const struct sockaddr_in *addr, const char *what)
{
	int r = connect(s, (const struct sockaddr *)addr, sizeof(*addr));
	int e = errno;

	CHECK(r == -1 && e == EISCONN, "%s: connect after completion returned %d errno=%d, expected EISCONN", what, r, r ? e : 0);
}

static void test_kqueue(void)
{
	struct sockaddr_in addr;
	int ls = listener(&addr);
	int s = nb_socket();
	int kq = kqueue();
	struct kevent ch, out;
	struct timespec ts = {5, 0};

	start_connect(s, &addr, "kqueue");
	check_repeat_pending(s, &addr, "kqueue");

	EV_SET(&ch, s, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
	memset(&out, 0, sizeof(out));
	int n = kevent(kq, &ch, 1, &out, 1, &ts);
	CHECK(n == 1 && out.filter == EVFILT_WRITE && (int)out.ident == s, "kqueue: EVFILT_WRITE not delivered (n=%d)", n);
	CHECK(so_error(s) == 0, "kqueue: SO_ERROR after completion is not 0");
	check_repeat_done(s, &addr, "kqueue");
	check_still_nonblocking(s, "kqueue");

	int c = accept(ls, NULL, NULL);
	CHECK(c >= 0, "accept: %s", strerror(errno));
	CHECK(write(s, "x", 1) == 1, "write after connect: %s", strerror(errno));
	close(c);
	close(kq);
	close(s);
	close(ls);
}

static void test_select_poll(void)
{
	struct sockaddr_in addr;
	int ls = listener(&addr);
	int s = nb_socket();
	fd_set wfds;
	struct timeval tv = {5, 0};

	start_connect(s, &addr, "select");
	FD_ZERO(&wfds);
	FD_SET(s, &wfds);
	int n = select(s + 1, NULL, &wfds, NULL, &tv);
	CHECK(n == 1 && FD_ISSET(s, &wfds), "select: not writable after connect (n=%d)", n);
	CHECK(so_error(s) == 0, "select: SO_ERROR after completion is not 0");
	close(s);

	s = nb_socket();
	start_connect(s, &addr, "poll");
	struct pollfd pfd = { s, POLLOUT, 0 };
	n = poll(&pfd, 1, 5000);
	CHECK(n == 1 && (pfd.revents & POLLOUT), "poll: not writable after connect (n=%d revents=%x)", n, pfd.revents);
	CHECK(so_error(s) == 0, "poll: SO_ERROR after completion is not 0");
	close(s);
	close(ls);
}

static void test_refused(void)
{
	struct sockaddr_in addr;
	int ls = listener(&addr);
	int s = nb_socket();
	struct pollfd pfd = { s, POLLOUT, 0 };

	close(ls);	/* nothing listens on the port any more */
	start_connect(s, &addr, "refused");
	int n = poll(&pfd, 1, 5000);
	CHECK(n == 1, "refused: poll did not report the failed attempt (n=%d)", n);
	CHECK(so_error(s) == ECONNREFUSED, "refused: SO_ERROR is %d, expected ECONNREFUSED", so_error(s));
	close(s);
}

static void test_unix(void)
{
	struct sockaddr_un un;
	char path[64];

	snprintf(path, sizeof(path), "/tmp/nbconnect-%d.sock", (int)getpid());
	memset(&un, 0, sizeof(un));
	un.sun_len = sizeof(un);
	un.sun_family = AF_UNIX;
	strcpy(un.sun_path, path);
	unlink(path);

	int ls = socket(AF_UNIX, SOCK_STREAM, 0);
	CHECK(bind(ls, (struct sockaddr *)&un, sizeof(un)) == 0, "unix bind: %s", strerror(errno));
	CHECK(listen(ls, 4) == 0, "unix listen: %s", strerror(errno));

	int s = socket(AF_UNIX, SOCK_STREAM, 0);
	fcntl(s, F_SETFL, fcntl(s, F_GETFL) | O_NONBLOCK);
	int r = connect(s, (struct sockaddr *)&un, sizeof(un));
	CHECK(r == 0, "unix: non-blocking connect returned %d errno=%d, expected 0", r, errno);
	check_still_nonblocking(s, "unix");
	CHECK(so_error(s) == 0, "unix: SO_ERROR is not 0");
	close(s);
	close(ls);
	unlink(path);
}

static void test_blocking(void)
{
	struct sockaddr_in addr;
	int ls = listener(&addr);
	int s = socket(AF_INET, SOCK_STREAM, 0);

	int r = connect(s, (struct sockaddr *)&addr, sizeof(addr));
	CHECK(r == 0, "blocking: connect returned %d errno=%d", r, errno);
	CHECK(!(fcntl(s, F_GETFL) & O_NONBLOCK), "blocking: socket became non-blocking");
	CHECK(so_error(s) == 0, "blocking: SO_ERROR is not 0");
	r = connect(s, (struct sockaddr *)&addr, sizeof(addr));
	CHECK(r == -1 && errno == EISCONN, "blocking: second connect returned %d errno=%d, expected EISCONN", r, errno);
	close(s);
	close(ls);
}

int main(void)
{
	test_kqueue();
	test_select_poll();
	test_refused();
	test_unix();
	test_blocking();
	if (failures) {
		printf("%d failure(s)\n", failures);
		return 1;
	}
	printf("PASS\n");
	return 0;
}
