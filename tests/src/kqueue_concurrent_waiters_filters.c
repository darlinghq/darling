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
 * Several threads wait on one kqueue at the same time (one with no timeout,
 * one with KEVENT_FLAG_IMMEDIATE, one with a zero timeout). Each round makes
 * one event happen, waits until a waiter has received it, and then changes
 * the kqueue, which must still work. Every event must be delivered exactly
 * once. A waiter that woke for an event another thread already took must
 * go back to waiting, not hold up the kqueue.
 *
 * Part 1: EVFILT_USER with EV_CLEAR, triggered with NOTE_TRIGGER.
 * Part 2: EVFILT_TIMER, a one-shot NOTE_ABSOLUTE deadline set each round.
 * Part 3: EVFILT_PROC with NOTE_FORK on a child that forks once per round.
 *
 * A watchdog fails the test when nothing happens for WATCHDOG_SECONDS.
 *
 * Runs on macOS and in Darling; prints PASS and exits 0. An optional
 * argument of 1, 2 or 3 runs only that part.
 */
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define WATCHDOG_SECONDS 10
#define PART_SECONDS 3
#define STOP_IDENT 0x5354
#define CHECK_IDENT 0x5355
#define EVENT_IDENT 0x4556

static _Atomic uint64_t progress;
static _Atomic int finished;
static const char* volatile phase = "setup";

static int kq;
static int16_t watched_filter;
static _Atomic int stop_waiters;
static _Atomic uint64_t received;

static double now(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + tv.tv_usec / 1e6;
}

static void fail(const char* what, long code)
{
	printf("FAIL: %s: %s (%ld)\n", phase, what, code);
	fflush(stdout);
	_exit(1);
}

static void* watchdog(void* arg)
{
	uint64_t last = (uint64_t) -1;
	double last_change = now();
	(void) arg;

	while (!atomic_load(&finished)) {
		uint64_t cur = atomic_load(&progress);
		if (cur != last) {
			last = cur;
			last_change = now();
		} else if (now() - last_change > WATCHDOG_SECONDS) {
			printf("FAIL: %s: no progress for %d s (%llu events received)\n", phase,
				WATCHDOG_SECONDS, (unsigned long long) atomic_load(&received));
			fflush(stdout);
			_exit(1);
		}
		usleep(100000);
	}
	return NULL;
}

enum wait_mode { WAIT_BLOCKING, WAIT_IMMEDIATE, WAIT_ZERO_TIMEOUT };

static void* waiter(void* arg)
{
	enum wait_mode mode = (enum wait_mode) (uintptr_t) arg;
	const struct timespec zero = { 0, 0 };

	while (!atomic_load(&stop_waiters)) {
		struct kevent64_s ev[4];
		int i, n;

		switch (mode) {
		case WAIT_BLOCKING:
			n = kevent64(kq, NULL, 0, ev, 4, 0, NULL);
			break;
		case WAIT_IMMEDIATE:
			n = kevent64(kq, NULL, 0, ev, 4, KEVENT_FLAG_IMMEDIATE, NULL);
			break;
		default:
			n = kevent64(kq, NULL, 0, ev, 4, 0, &zero);
			break;
		}

		if (n < 0) {
			if (errno == EINTR)
				continue;
			fail("kevent64 wait", errno);
		}

		for (i = 0; i < n; i++) {
			if (ev[i].filter == EVFILT_USER && ev[i].ident == STOP_IDENT)
				continue;
			if (ev[i].filter != watched_filter)
				fail("unexpected filter", ev[i].filter);
			if (ev[i].flags & EV_ERROR)
				fail("EV_ERROR event", (long) ev[i].data);
			atomic_fetch_add(&received, 1);
			atomic_fetch_add(&progress, 1);
		}

		if (mode != WAIT_BLOCKING && n == 0)
			sched_yield();
	}
	return NULL;
}

static void change1(uint64_t ident, int16_t filter, uint16_t flags, uint32_t fflags, int64_t data)
{
	struct kevent64_s ev;

	EV_SET64(&ev, ident, filter, flags, fflags, data, 0, 0, 0);
	if (kevent64(kq, &ev, 1, NULL, 0, 0, NULL) < 0)
		fail("kevent64 change", errno);
}

static pthread_t waiters[3];

static void start(const char* name, int16_t filter)
{
	int i;

	phase = name;
	watched_filter = filter;
	atomic_store(&stop_waiters, 0);
	atomic_store(&received, 0);

	kq = kqueue();
	if (kq < 0)
		fail("kqueue", errno);
	change1(STOP_IDENT, EVFILT_USER, EV_ADD | EV_ENABLE, 0, 0);

	for (i = 0; i < 3; i++)
		pthread_create(&waiters[i], NULL, waiter, (void*) (uintptr_t) i);
}

/* Waits for the round's event, then checks that the kqueue takes changes. */
static void finish_round(uint64_t expected, unsigned* seed)
{
	while (atomic_load(&received) < expected)
		usleep(100);
	usleep(rand_r(seed) % 300);

	change1(CHECK_IDENT, EVFILT_USER, EV_ADD | EV_DISABLE, 0, 0);
	change1(CHECK_IDENT, EVFILT_USER, EV_DELETE, 0, 0);
	atomic_fetch_add(&progress, 1);
}

static void stop(uint64_t rounds)
{
	int i;

	/* Give a duplicate delivery the chance to show up. */
	usleep(100000);
	if (atomic_load(&received) != rounds)
		fail("events received differ from events caused", (long) atomic_load(&received));

	atomic_store(&stop_waiters, 1);
	change1(STOP_IDENT, EVFILT_USER, 0, NOTE_TRIGGER, 0);
	for (i = 0; i < 3; i++)
		pthread_join(waiters[i], NULL);
	close(kq);
	printf("%s: %llu events\n", phase, (unsigned long long) rounds);
}

static void part_user(void)
{
	unsigned seed = (unsigned) time(NULL) ^ 0x1111;
	uint64_t rounds = 0;
	double end;

	start("part 1 (EVFILT_USER EV_CLEAR)", EVFILT_USER);
	change1(EVENT_IDENT, EVFILT_USER, EV_ADD | EV_ENABLE | EV_CLEAR, 0, 0);

	end = now() + PART_SECONDS;
	while (now() < end) {
		change1(EVENT_IDENT, EVFILT_USER, 0, NOTE_TRIGGER, 0);
		finish_round(++rounds, &seed);
	}
	stop(rounds);
}

static void part_timer(void)
{
	unsigned seed = (unsigned) time(NULL) ^ 0x2222;
	uint64_t rounds = 0;
	double end;

	start("part 2 (EVFILT_TIMER NOTE_ABSOLUTE)", EVFILT_TIMER);

	end = now() + PART_SECONDS;
	while (now() < end) {
		struct timeval tv;
		int64_t deadline;

		gettimeofday(&tv, NULL);
		deadline = (int64_t) tv.tv_sec * 1000000 + tv.tv_usec + 200 + rand_r(&seed) % 300;
		change1(EVENT_IDENT, EVFILT_TIMER, EV_ADD | EV_ENABLE, NOTE_ABSOLUTE | NOTE_USECONDS, deadline);
		finish_round(++rounds, &seed);
	}
	stop(rounds);
}

static int proc_cmd = -1, proc_ack = -1;
static pid_t proc_child = -1;

/* Called before any other thread exists; the child forks once per byte read. */
static void proc_child_start(void)
{
	int cmd[2], ack[2];
	char c;

	if (pipe(cmd) < 0 || pipe(ack) < 0)
		fail("pipe", errno);

	proc_child = fork();
	if (proc_child < 0)
		fail("fork", errno);
	if (proc_child == 0) {
		close(cmd[1]);
		close(ack[0]);
		while (read(cmd[0], &c, 1) == 1) {
			pid_t gc = fork();
			if (gc == 0)
				_exit(0);
			if (gc > 0)
				waitpid(gc, NULL, 0);
			if (write(ack[1], &c, 1) != 1)
				_exit(2);
		}
		_exit(0);
	}
	close(cmd[0]);
	close(ack[1]);
	proc_cmd = cmd[1];
	proc_ack = ack[0];
}

static void part_proc(void)
{
	unsigned seed = (unsigned) time(NULL) ^ 0x3333;
	uint64_t rounds = 0;
	double end;
	char c;

	start("part 3 (EVFILT_PROC NOTE_FORK)", EVFILT_PROC);
	change1(proc_child, EVFILT_PROC, EV_ADD | EV_ENABLE, NOTE_FORK, 0);

	end = now() + PART_SECONDS;
	while (now() < end) {
		c = 'f';
		if (write(proc_cmd, &c, 1) != 1 || read(proc_ack, &c, 1) != 1)
			fail("talking to the child", errno);
		finish_round(++rounds, &seed);
	}
	stop(rounds);

	close(proc_cmd);
	close(proc_ack);
	waitpid(proc_child, NULL, 0);
}

int main(int argc, char** argv)
{
	pthread_t dog;
	int only = (argc > 1) ? atoi(argv[1]) : 0;

	setvbuf(stdout, NULL, _IOLBF, 0);
	if (only == 0 || only == 3)
		proc_child_start();
	pthread_create(&dog, NULL, watchdog, NULL);

	if (only == 0 || only == 1)
		part_user();
	if (only == 0 || only == 2)
		part_timer();
	if (only == 0 || only == 3)
		part_proc();

	atomic_store(&finished, 1);
	pthread_join(dog, NULL);
	printf("PASS\n");
	return 0;
}
