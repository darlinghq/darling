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
 * Several threads may wait on one kqueue at the same time. When a Mach port
 * registered with EVFILT_MACHPORT becomes ready, every waiter may wake, but
 * only one of them gets the event; the others must go back to waiting (or
 * return, for a poll) and must not stop the kqueue from working.
 *
 * Part 1: one kqueue with an EVFILT_MACHPORT registration that receives
 * each message (MACH_RCV_MSG). One thread waits with no timeout, one polls
 * with KEVENT_FLAG_IMMEDIATE and one polls with a zero timeout, while a
 * sender posts short bursts of messages at random intervals. After each
 * burst has been received, the kqueue must still take a change. Every
 * message must be delivered exactly once, by one of the waiters.
 *
 * Part 2: two threads each create, resume and cancel
 * DISPATCH_SOURCE_TYPE_MACH_RECV sources on their own port in a loop, while
 * a sender posts messages to both ports at random intervals. The handlers
 * receive the messages; every message must be received once.
 *
 * A watchdog fails the test when nothing happens for WATCHDOG_SECONDS.
 *
 * Runs on macOS and in Darling; prints PASS and exits 0. An optional
 * argument of 1 or 2 runs only that part.
 */
#include <dispatch/dispatch.h>
#include <errno.h>
#include <mach/mach.h>
#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/event.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define WATCHDOG_SECONDS 10
#define PART1_SECONDS 3
#define PART2_SECONDS 3
#define STOP_IDENT 0x5354

typedef struct {
	mach_msg_header_t header;
	uint32_t seq;
} test_msg_t;

typedef struct {
	mach_msg_header_t header;
	uint32_t seq;
	mach_msg_max_trailer_t trailer;
} test_rcv_msg_t;

static _Atomic uint64_t progress;
static _Atomic int finished;
static const char* volatile phase = "setup";

static double now(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + tv.tv_usec / 1e6;
}

static void fail(const char* what, long code)
{
	printf("FAIL: %s (%ld)\n", what, code);
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
			printf("FAIL: %s: no progress for %d s (progress count %llu)\n", phase,
				WATCHDOG_SECONDS, (unsigned long long) cur);
			fflush(stdout);
			_exit(1);
		}
		usleep(100000);
	}
	return NULL;
}

static mach_port_t make_port(void)
{
	mach_port_t port = MACH_PORT_NULL;
	mach_port_limits_t limits = { .mpl_qlimit = MACH_PORT_QLIMIT_LARGE };
	kern_return_t kr;

	kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &port);
	if (kr != KERN_SUCCESS)
		fail("mach_port_allocate", kr);
	kr = mach_port_insert_right(mach_task_self(), port, port, MACH_MSG_TYPE_MAKE_SEND);
	if (kr != KERN_SUCCESS)
		fail("mach_port_insert_right", kr);
	kr = mach_port_set_attributes(mach_task_self(), port, MACH_PORT_LIMITS_INFO,
		(mach_port_info_t) &limits, MACH_PORT_LIMITS_INFO_COUNT);
	if (kr != KERN_SUCCESS)
		fail("mach_port_set_attributes", kr);
	return port;
}

static void send_one(mach_port_t port, uint32_t seq)
{
	test_msg_t msg;
	kern_return_t kr;

	memset(&msg, 0, sizeof(msg));
	msg.header.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, 0);
	msg.header.msgh_size = sizeof(msg);
	msg.header.msgh_remote_port = port;
	msg.header.msgh_id = 0x4b51;
	msg.seq = seq;

	/* Blocks only when the queue is full; the watchdog covers a stall. */
	kr = mach_msg(&msg.header, MACH_SEND_MSG, sizeof(msg), 0, MACH_PORT_NULL,
		MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	if (kr != MACH_MSG_SUCCESS)
		fail("mach_msg send", kr);
}

static void random_pause(unsigned* seed)
{
	unsigned r = rand_r(seed) % 8;
	if (r < 3)
		return; /* back to back */
	if (r < 6)
		usleep(rand_r(seed) % 50);
	else
		usleep(rand_r(seed) % 500);
}

/* Part 1 */

static int kq;
static mach_port_t p1_port;
static _Atomic int p1_stop;
static _Atomic uint64_t p1_received;
static test_rcv_msg_t p1_buffer;

enum wait_mode { WAIT_BLOCKING, WAIT_IMMEDIATE, WAIT_ZERO_TIMEOUT };

static void* p1_waiter(void* arg)
{
	enum wait_mode mode = (enum wait_mode) (uintptr_t) arg;
	const struct timespec zero = { 0, 0 };

	while (!atomic_load(&p1_stop)) {
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
			if (ev[i].filter == EVFILT_USER)
				continue;
			if (ev[i].filter != EVFILT_MACHPORT)
				fail("unexpected filter", ev[i].filter);
			if (ev[i].flags & EV_ERROR)
				fail("EV_ERROR on the Mach port event", (long) ev[i].data);
			if (ev[i].fflags != MACH_MSG_SUCCESS)
				fail("message receive failed in kevent", ev[i].fflags);
			atomic_fetch_add(&p1_received, 1);
			atomic_fetch_add(&progress, 1);
		}

		if (mode != WAIT_BLOCKING && n == 0)
			sched_yield();
	}
	return NULL;
}

static void part1(void)
{
	struct kevent64_s change[2];
	pthread_t waiters[3];
	unsigned seed = (unsigned) time(NULL) ^ 0x1234;
	uint32_t sent = 0;
	unsigned bursts = 0;
	double end;
	int i;

	phase = "part 1 (kevent64 waiters)";
	p1_port = make_port();
	kq = kqueue();
	if (kq < 0)
		fail("kqueue", errno);

	memset(change, 0, sizeof(change));
	EV_SET64(&change[0], p1_port, EVFILT_MACHPORT, EV_ADD | EV_ENABLE,
		MACH_RCV_MSG, 0, 0, (uint64_t) (uintptr_t) &p1_buffer, sizeof(p1_buffer));
	EV_SET64(&change[1], STOP_IDENT, EVFILT_USER, EV_ADD | EV_ENABLE, 0, 0, 0, 0, 0);
	if (kevent64(kq, change, 2, NULL, 0, 0, NULL) < 0)
		fail("kevent64 register", errno);

	pthread_create(&waiters[0], NULL, p1_waiter, (void*) (uintptr_t) WAIT_BLOCKING);
	pthread_create(&waiters[1], NULL, p1_waiter, (void*) (uintptr_t) WAIT_IMMEDIATE);
	pthread_create(&waiters[2], NULL, p1_waiter, (void*) (uintptr_t) WAIT_ZERO_TIMEOUT);

	/*
	 * Bursts of messages, each followed by a quiet period: once the waiters
	 * have taken every message, no new message arrives that could wake a
	 * waiter stuck on the old one. The kqueue must then still accept a
	 * change (which needs the kqueue to be free).
	 */
	end = now() + PART1_SECONDS;
	while (now() < end) {
		int burst = 1 + rand_r(&seed) % 4;

		for (i = 0; i < burst; i++) {
			send_one(p1_port, sent++);
			random_pause(&seed);
		}
		while (atomic_load(&p1_received) < sent)
			usleep(100);
		usleep(rand_r(&seed) % 300);

		EV_SET64(&change[0], STOP_IDENT + 1, EVFILT_USER, EV_ADD | EV_DISABLE, 0, 0, 0, 0, 0);
		EV_SET64(&change[1], STOP_IDENT + 1, EVFILT_USER, EV_DELETE, 0, 0, 0, 0, 0);
		if (kevent64(kq, change, 2, NULL, 0, 0, NULL) < 0)
			fail("kevent64 change after a burst", errno);
		atomic_fetch_add(&progress, 1);
		bursts++;
	}

	if (atomic_load(&p1_received) != sent)
		fail("part 1 received more events than messages", (long) atomic_load(&p1_received));

	atomic_store(&p1_stop, 1);
	EV_SET64(&change[0], STOP_IDENT, EVFILT_USER, 0, NOTE_TRIGGER, 0, 0, 0, 0);
	if (kevent64(kq, change, 1, NULL, 0, 0, NULL) < 0)
		fail("kevent64 trigger", errno);
	for (i = 0; i < 3; i++)
		pthread_join(waiters[i], NULL);

	close(kq);
	mach_port_mod_refs(mach_task_self(), p1_port, MACH_PORT_RIGHT_RECEIVE, -1);
	mach_port_deallocate(mach_task_self(), p1_port);
	printf("part 1: %u messages in %u bursts through 3 concurrent waiters\n", sent, bursts);
}

/* Part 2 */

#define P2_PORTS 2

static mach_port_t p2_ports[P2_PORTS];
static _Atomic int p2_stop;
static _Atomic uint64_t p2_received;
static _Atomic uint64_t p2_cycles;

static unsigned drain_port(mach_port_t port)
{
	unsigned count = 0;

	for (;;) {
		test_rcv_msg_t msg;
		kern_return_t kr;

		memset(&msg, 0, sizeof(msg));
		kr = mach_msg(&msg.header, MACH_RCV_MSG | MACH_RCV_TIMEOUT, 0, sizeof(msg),
			port, 0, MACH_PORT_NULL);
		if (kr == MACH_RCV_TIMED_OUT)
			break;
		if (kr != MACH_MSG_SUCCESS)
			fail("mach_msg receive", kr);
		count++;
	}
	return count;
}

static void* p2_cycler(void* arg)
{
	mach_port_t port = p2_ports[(uintptr_t) arg];
	dispatch_queue_t queue = dispatch_queue_create("kqueue-machport-test", NULL);
	unsigned seed = (unsigned) time(NULL) ^ (unsigned) (uintptr_t) arg ^ 0x5678;

	while (!atomic_load(&p2_stop)) {
		dispatch_semaphore_t cancelled = dispatch_semaphore_create(0);
		dispatch_source_t source = dispatch_source_create(DISPATCH_SOURCE_TYPE_MACH_RECV, port, 0, queue);

		if (source == NULL)
			fail("dispatch_source_create", 0);
		dispatch_source_set_event_handler(source, ^{
			unsigned n = drain_port(port);
			atomic_fetch_add(&p2_received, n);
			atomic_fetch_add(&progress, n);
		});
		dispatch_source_set_cancel_handler(source, ^{
			dispatch_semaphore_signal(cancelled);
		});
		dispatch_resume(source);

		random_pause(&seed);
		if (rand_r(&seed) % 4 == 0)
			usleep(rand_r(&seed) % 2000);

		dispatch_source_cancel(source);
		dispatch_semaphore_wait(cancelled, DISPATCH_TIME_FOREVER);
		dispatch_release(source);
		dispatch_release(cancelled);

		atomic_fetch_add(&p2_cycles, 1);
		atomic_fetch_add(&progress, 1);
	}

	dispatch_release(queue);
	return NULL;
}

static void part2(void)
{
	pthread_t cyclers[P2_PORTS];
	unsigned seed = (unsigned) time(NULL) ^ 0x9abc;
	uint32_t sent = 0;
	uint64_t leftover = 0;
	double end;
	int i;

	phase = "part 2 (dispatch mach sources)";
	for (i = 0; i < P2_PORTS; i++)
		p2_ports[i] = make_port();
	for (i = 0; i < P2_PORTS; i++)
		pthread_create(&cyclers[i], NULL, p2_cycler, (void*) (uintptr_t) i);

	/*
	 * After each burst, wait for the cycling threads to get through more
	 * source cycles before sending again, so that a stall is not hidden by
	 * the next message.
	 */
	end = now() + PART2_SECONDS;
	while (now() < end) {
		int burst = 1 + rand_r(&seed) % 4;
		uint64_t cycles = atomic_load(&p2_cycles);

		for (i = 0; i < burst; i++) {
			send_one(p2_ports[rand_r(&seed) % P2_PORTS], sent++);
			random_pause(&seed);
		}
		while (atomic_load(&p2_cycles) < cycles + P2_PORTS)
			usleep(100);
	}

	atomic_store(&p2_stop, 1);
	for (i = 0; i < P2_PORTS; i++)
		pthread_join(cyclers[i], NULL);

	/* The last source may have been cancelled before it saw the last messages. */
	for (i = 0; i < P2_PORTS; i++)
		leftover += drain_port(p2_ports[i]);

	if (atomic_load(&p2_received) + leftover != sent) {
		printf("FAIL: part 2: sent %u, handlers received %llu, left in ports %llu\n", sent,
			(unsigned long long) atomic_load(&p2_received), (unsigned long long) leftover);
		exit(1);
	}

	for (i = 0; i < P2_PORTS; i++) {
		mach_port_mod_refs(mach_task_self(), p2_ports[i], MACH_PORT_RIGHT_RECEIVE, -1);
		mach_port_deallocate(mach_task_self(), p2_ports[i]);
	}
	printf("part 2: %u messages, %llu source cycles, %llu received by handlers\n", sent,
		(unsigned long long) atomic_load(&p2_cycles), (unsigned long long) atomic_load(&p2_received));
}

int main(int argc, char** argv)
{
	pthread_t dog;
	int only = (argc > 1) ? atoi(argv[1]) : 0;

	setvbuf(stdout, NULL, _IOLBF, 0);
	pthread_create(&dog, NULL, watchdog, NULL);

	/* An optional argument (1 or 2) runs only that part. */
	if (only == 0 || only == 1)
		part1();
	if (only == 0 || only == 2)
		part2();

	atomic_store(&finished, 1);
	pthread_join(dog, NULL);
	printf("PASS\n");
	return 0;
}
