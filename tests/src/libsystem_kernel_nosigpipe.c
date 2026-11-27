/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * fcntl(2): F_SETNOSIGPIPE and F_GETNOSIGPIPE keep a per-descriptor flag that
 * makes a write to a socket with no reader fail with EPIPE without raising SIGPIPE. A bad descriptor is EBADF.
 * Expected results observed on macOS 10.14.4 (i386, x86_64) and macOS 26.7.3 (x86_64).
 */
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static int failures;
static volatile int sigpipes;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s (errno %d)\n", __LINE__, #cond, errno); failures++; } } while (0)

static void on_sigpipe(int sig) { (void) sig; sigpipes++; }

int main(void)
{
	int fds[2], sv[2];
	signal(SIGPIPE, on_sigpipe);

	CHECK(pipe(fds) == 0);
	errno = 0;
	CHECK(fcntl(fds[1], F_GETNOSIGPIPE) == 0);
	errno = 0;
	CHECK(fcntl(fds[1], F_SETNOSIGPIPE, 1) == 0);
	CHECK(fcntl(fds[1], F_GETNOSIGPIPE) == 1);

	/* the flag belongs to the descriptor, not to the other end */
	CHECK(fcntl(fds[0], F_GETNOSIGPIPE) == 0);
	CHECK(fcntl(fds[1], F_SETNOSIGPIPE, 0) == 0);
	CHECK(fcntl(fds[1], F_GETNOSIGPIPE) == 0);
	close(fds[0]);
	close(fds[1]);

	/* a socket with the flag set: writing to it after the peer closed is EPIPE without SIGPIPE */
	CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0);
	CHECK(fcntl(sv[0], F_SETNOSIGPIPE, 1) == 0);
	CHECK(fcntl(sv[0], F_GETNOSIGPIPE) == 1);
	close(sv[1]);
	errno = 0;
	CHECK(write(sv[0], "x", 1) == -1 && errno == EPIPE);
	errno = 0;
	CHECK(send(sv[0], "x", 1, 0) == -1 && errno == EPIPE);
	CHECK(sigpipes == 0);
	close(sv[0]);

	/* without it SIGPIPE is raised as well */
	CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0);
	CHECK(fcntl(sv[0], F_SETNOSIGPIPE, 1) == 0);
	CHECK(fcntl(sv[0], F_SETNOSIGPIPE, 0) == 0);
	CHECK(fcntl(sv[0], F_GETNOSIGPIPE) == 0);
	close(sv[1]);
	errno = 0;
	CHECK(send(sv[0], "x", 1, 0) == -1 && errno == EPIPE);
	CHECK(sigpipes == 1);
	close(sv[0]);

	errno = 0;
	CHECK(fcntl(-1, F_SETNOSIGPIPE, 1) == -1 && errno == EBADF);
	errno = 0;
	CHECK(fcntl(-1, F_GETNOSIGPIPE) == -1 && errno == EBADF);

	if (failures == 0)
		puts("PASS");
	return failures != 0;
}
