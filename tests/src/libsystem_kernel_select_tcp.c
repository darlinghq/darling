/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * select(2) on TCP sockets that never connected, and on a descriptor another thread closes. Expected results as
 * observed on macOS 26.7.3:
 *  - a TCP socket that is not connected (fresh, or bound but not listening) is neither readable nor writable;
 *  - a select() that waits on a descriptor which another thread closes fails with EBADF;
 *  - connected and listening sockets keep working: a listener is readable when a connection waits, a connected
 *    socket is writable and becomes readable when data arrives.
 * Before the change Linux's answer (readable and writable at once) was passed on.
 */
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s (errno %d)\n", __LINE__, #cond, errno); failures++; } } while (0)

static void* closer(void* arg)
{
	usleep(200000);
	close((int) (long) arg);
	return NULL;
}

static long elapsed_ms(struct timespec a, struct timespec b)
{
	return (b.tv_sec - a.tv_sec) * 1000 + (b.tv_nsec - a.tv_nsec) / 1000000;
}

int main(void)
{
	struct timeval tv;
	fd_set rf, wf;

	// fresh TCP socket: neither readable nor writable
	int s = socket(AF_INET, SOCK_STREAM, 0);
	CHECK(s >= 0);
	FD_ZERO(&rf);
	FD_ZERO(&wf);
	FD_SET(s, &rf);
	FD_SET(s, &wf);
	tv.tv_sec = 0;
	tv.tv_usec = 100000;
	CHECK(select(s + 1, &rf, &wf, NULL, &tv) == 0);
	CHECK(!FD_ISSET(s, &rf) && !FD_ISSET(s, &wf));

	// a closed-by-another-thread descriptor ends the wait with EBADF, well before the timeout
	FD_ZERO(&rf);
	FD_SET(s, &rf);
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	pthread_t t;
	CHECK(pthread_create(&t, NULL, closer, (void*) (long) s) == 0);
	struct timespec a, b;
	clock_gettime(CLOCK_MONOTONIC, &a);
	int r = select(s + 1, &rf, NULL, NULL, &tv);
	int e = errno;
	clock_gettime(CLOCK_MONOTONIC, &b);
	CHECK(r == -1 && e == EBADF);
	CHECK(elapsed_ms(a, b) < 3000);
	pthread_join(t, NULL);

	// bound but not listening: not readable
	s = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in sa;
	memset(&sa, 0, sizeof sa);
	sa.sin_len = sizeof sa;
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	CHECK(bind(s, (struct sockaddr*) &sa, sizeof sa) == 0);
	FD_ZERO(&rf);
	FD_SET(s, &rf);
	tv.tv_sec = 0;
	tv.tv_usec = 100000;
	CHECK(select(s + 1, &rf, NULL, NULL, &tv) == 0);
	CHECK(!FD_ISSET(s, &rf));

	// listening and connected sockets
	CHECK(listen(s, 4) == 0);
	socklen_t len = sizeof sa;
	CHECK(getsockname(s, (struct sockaddr*) &sa, &len) == 0);
	FD_ZERO(&rf);
	FD_SET(s, &rf);
	tv.tv_sec = 0;
	tv.tv_usec = 50000;
	CHECK(select(s + 1, &rf, NULL, NULL, &tv) == 0); // nothing waiting yet
	int c = socket(AF_INET, SOCK_STREAM, 0);
	CHECK(connect(c, (struct sockaddr*) &sa, sizeof sa) == 0);
	FD_ZERO(&rf);
	FD_SET(s, &rf);
	tv.tv_sec = 2;
	tv.tv_usec = 0;
	CHECK(select(s + 1, &rf, NULL, NULL, &tv) == 1 && FD_ISSET(s, &rf));
	int d = accept(s, NULL, NULL);
	CHECK(d >= 0);
	FD_ZERO(&rf);
	FD_ZERO(&wf);
	FD_SET(c, &rf);
	FD_SET(c, &wf);
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	CHECK(select(c + 1, &rf, &wf, NULL, &tv) == 1 && FD_ISSET(c, &wf) && !FD_ISSET(c, &rf));
	CHECK(write(d, "x", 1) == 1);
	FD_ZERO(&rf);
	FD_SET(c, &rf);
	tv.tv_sec = 2;
	tv.tv_usec = 0;
	CHECK(select(c + 1, &rf, NULL, NULL, &tv) == 1 && FD_ISSET(c, &rf));

	// a never-connected socket in the same set as a ready descriptor: only the ready one is reported
	int u = socket(AF_INET, SOCK_STREAM, 0);
	int hi = c > u ? c : u;
	FD_ZERO(&rf);
	FD_SET(c, &rf);
	FD_SET(u, &rf);
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	CHECK(select(hi + 1, &rf, NULL, NULL, &tv) == 1 && FD_ISSET(c, &rf) && !FD_ISSET(u, &rf));

	close(u);
	close(c);
	close(d);
	close(s);
	printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
	return failures != 0;
}
