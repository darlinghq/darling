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
 * A kqueue is not inherited by a child created with fork(), so the child
 * must not keep descriptors the kqueue machinery opened on its behalf, and
 * the kqueue descriptor itself is close-on-exec (as on macOS). Registers
 * several kinds of event in the parent, forks, and checks in the child that
 * no descriptor exists beyond those the program opened itself. The standard
 * input descriptor is closed first so that a helper descriptor may take
 * number 0.
 *
 * Runs on macOS and in Darling; prints PASS and exits 0.
 */
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#define CHECK(cond) do { if (!(cond)) { \
	fprintf(stderr, "FAIL: %s (line %d, errno %d)\n", #cond, __LINE__, errno); _exit(1); } } while (0)

#define MAXFD 1024

static char expected[MAXFD];

static void record_open(void)
{
	for (int fd = 0; fd < MAXFD; fd++)
		expected[fd] = fcntl(fd, F_GETFD) != -1;
}

int main(void)
{
	int p[2];
	CHECK(pipe(p) == 0);
	int kq = kqueue();
	CHECK(kq >= 0);
	int cloexec = fcntl(kq, F_GETFD) & FD_CLOEXEC;

	// Descriptor 0 free, so a helper descriptor can be numbered 0.
	close(0);
	record_open();

	struct kevent ev[4];
	EV_SET(&ev[0], p[0], EVFILT_READ, EV_ADD, 0, 0, NULL);
	EV_SET(&ev[1], 1, EVFILT_TIMER, EV_ADD, 0, 60000, NULL);
	EV_SET(&ev[2], 1, EVFILT_USER, EV_ADD | EV_CLEAR, 0, 0, NULL);
	EV_SET(&ev[3], SIGUSR2, EVFILT_SIGNAL, EV_ADD, 0, 0, NULL);
	CHECK(kevent(kq, ev, 4, NULL, 0, NULL) == 0);

	pid_t pid = fork();
	CHECK(pid >= 0);
	if (pid == 0) {
		int stray = 0;
		for (int fd = 0; fd < MAXFD; fd++) {
			if (!expected[fd] && fcntl(fd, F_GETFD) != -1) {
				fprintf(stderr, "child still has descriptor %d\n", fd);
				stray++;
			}
		}
		_exit(stray ? 2 : 0);
	}
	int status;
	CHECK(waitpid(pid, &status, 0) == pid);
	CHECK(WIFEXITED(status));
	int child_clean = WEXITSTATUS(status) == 0;

	// The parent's registrations are untouched.
	CHECK(write(p[1], "x", 1) == 1);
	struct kevent out;
	struct timespec ts = { 5, 0 };
	CHECK(kevent(kq, NULL, 0, &out, 1, &ts) == 1);
	CHECK(out.filter == EVFILT_READ);

	if (!cloexec)
		fprintf(stderr, "FAIL: kqueue descriptor is not close-on-exec\n");
	if (!child_clean)
		fprintf(stderr, "FAIL: child holds descriptors the parent did not open\n");
	if (!cloexec || !child_clean)
		return 1;

	printf("PASS: fork child holds no kqueue helper descriptors\n");
	return 0;
}
