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

// Covers a quirk: what select() and pselect() report in exceptfds for a
// pseudo-terminal whose other side was closed. Observed on macOS 27 (arm64
// and x86_64): a master whose slave was closed is readable, writable and
// exceptional; a slave whose master was closed and a pipe whose writer was
// closed are readable and writable only; a packet-mode master is
// exceptional while a status byte is pending.

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
#include <util.h>

static int failures;

#define CHECK(cond) do { \
	if (!(cond)) { \
		printf("FAIL line %d: %s (errno %d)\n", __LINE__, #cond, errno); \
		failures++; \
	} \
} while (0)

// Returns the result; *r, *w, *e say which sets kept fd.
static int wait_fd(int fd, int use_pselect, int with_write, long usec, int* r, int* w, int* e)
{
	fd_set rs, ws, es;
	FD_ZERO(&rs); FD_ZERO(&ws); FD_ZERO(&es);
	FD_SET(fd, &rs); FD_SET(fd, &es);
	if (with_write)
		FD_SET(fd, &ws);
	int n;
	if (use_pselect)
	{
		struct timespec ts = { usec / 1000000, (usec % 1000000) * 1000 };
		n = pselect(fd + 1, &rs, with_write ? &ws : NULL, &es, &ts, NULL);
	}
	else
	{
		struct timeval tv = { usec / 1000000, (int) (usec % 1000000) };
		n = select(fd + 1, &rs, with_write ? &ws : NULL, &es, &tv);
	}
	*r = FD_ISSET(fd, &rs) ? 1 : 0;
	*w = FD_ISSET(fd, &ws) ? 1 : 0;
	*e = FD_ISSET(fd, &es) ? 1 : 0;
	return n;
}

static double now(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + tv.tv_usec / 1e6;
}

static void* close_later(void* arg)
{
	usleep(100000);
	close(*(int*) arg);
	return NULL;
}

static void check(int use_pselect)
{
	int m, s, r, w, e, n;

	CHECK(openpty(&m, &s, NULL, NULL, NULL) == 0);
	n = wait_fd(m, use_pselect, 1, 100000, &r, &w, &e);
	CHECK(n == 1 && r == 0 && w == 1 && e == 0);
	close(s);
	n = wait_fd(m, use_pselect, 1, 1000000, &r, &w, &e);
	CHECK(n == 3 && r == 1 && w == 1 && e == 1);
	close(m);

	// The hangup wakes up a wait that started before it.
	CHECK(openpty(&m, &s, NULL, NULL, NULL) == 0);
	pthread_t thread;
	pthread_create(&thread, NULL, close_later, &s);
	double start = now();
	n = wait_fd(m, use_pselect, 0, 3000000, &r, &w, &e);
	double elapsed = now() - start;
	pthread_join(thread, NULL);
	CHECK(n == 2 && r == 1 && e == 1);
	CHECK(elapsed < 1.5);
	close(m);

	CHECK(openpty(&m, &s, NULL, NULL, NULL) == 0);
	close(m);
	n = wait_fd(s, use_pselect, 1, 1000000, &r, &w, &e);
	CHECK(n == 2 && r == 1 && w == 1 && e == 0);
	close(s);

	int p[2];
	CHECK(pipe(p) == 0);
	close(p[1]);
	n = wait_fd(p[0], use_pselect, 1, 1000000, &r, &w, &e);
	// macOS also reports the read end writable; Linux does not and Darling
	// follows it, so only readable and not exceptional are checked.
	CHECK(n >= 1 && r == 1 && e == 0);
	close(p[0]);

	// Packet mode: a pending status byte is exceptional.
	int one = 1;
	CHECK(openpty(&m, &s, NULL, NULL, NULL) == 0);
	CHECK(ioctl(m, TIOCPKT, &one) == 0);
	n = wait_fd(m, use_pselect, 0, 100000, &r, &w, &e);
	CHECK(n == 0 && e == 0);
	CHECK(tcflush(s, TCIFLUSH) == 0);
	n = wait_fd(m, use_pselect, 0, 1000000, &r, &w, &e);
	CHECK(n == 2 && r == 1 && e == 1);
	close(s);
	close(m);
}

int main(void)
{
	check(1);
	check(0);
	if (failures)
		return 1;
	puts("PASS");
	return 0;
}
