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
 * A workqueue kevent call (kevent_qos with KEVENT_FLAG_WORKQ) that returns no
 * events must return 0 to the caller without handing anything to a workqueue
 * thread: the kevent worker function must not run for it. A call that does
 * return an event is delivered to the kevent worker exactly once.
 */
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/event.h>
#include <unistd.h>

struct kevent_qos_s_ {
	uint64_t ident;
	int16_t filter;
	uint16_t flags;
	int32_t qos;
	uint64_t udata;
	uint32_t fflags;
	uint32_t xflags;
	int64_t data;
	uint64_t ext[4];
};
#define KEVENT_FLAG_IMMEDIATE_ 0x001
#define KEVENT_FLAG_WORKQ_ 0x020
extern int kevent_qos(int kq, const struct kevent_qos_s_* changelist, int nchanges,
		struct kevent_qos_s_* eventlist, int nevents, void* data_out,
		size_t* data_available, unsigned int flags);
typedef unsigned long pthread_priority_t_;
extern int _pthread_workqueue_init_with_kevent(void (*queue_func)(pthread_priority_t_),
		void (*kevent_func)(void** events, int* nevents), int offset, int flags);

static atomic_int deliveries;
static atomic_int stray;

static void worker(pthread_priority_t_ pp)
{
}

static void kevent_worker(void** events, int* nevents)
{
	struct kevent_qos_s_* ev = *events;

	for (int i = 0; i < *nevents; i++)
	{
		if (ev[i].filter == EVFILT_USER && ev[i].ident == 77)
			atomic_fetch_add(&deliveries, 1);
		else
			atomic_fetch_add(&stray, 1);
	}
	if (*nevents == 0)
		atomic_fetch_add(&stray, 1);
	*nevents = 0;
}

static void watchdog(int sig)
{
	static const char msg[] = "FAIL: timed out\n";
	write(1, msg, sizeof(msg) - 1);
	_exit(1);
}

int main(void)
{
	struct kevent_qos_s_ ke, out;
	int r, ok = 1;

	signal(SIGALRM, watchdog);
	alarm(30);

	r = _pthread_workqueue_init_with_kevent(worker, kevent_worker, 0, 0);
	if (r != 0)
	{
		printf("SKIP: workqueue already set up (%d)\n", r);
		return 0;
	}

	// Nothing pending: the call returns 0, and no worker runs for it
	for (int i = 0; i < 100; i++)
	{
		r = kevent_qos(-1, NULL, 0, &out, 1, NULL, NULL,
			KEVENT_FLAG_WORKQ_ | KEVENT_FLAG_IMMEDIATE_);
		if (r != 0)
		{
			printf("FAIL: empty call returned %d\n", r);
			ok = 0;
			break;
		}
	}
	usleep(200000);
	if (atomic_load(&stray) != 0)
	{
		printf("FAIL: kevent worker ran %d times for empty calls\n", atomic_load(&stray));
		ok = 0;
	}

	// A triggered event is delivered to the kevent worker once
	memset(&ke, 0, sizeof(ke));
	ke.ident = 77;
	ke.filter = EVFILT_USER;
	ke.flags = EV_ADD | EV_ONESHOT;
	ke.fflags = NOTE_TRIGGER;
	r = kevent_qos(-1, &ke, 1, NULL, 0, NULL, NULL, KEVENT_FLAG_WORKQ_);
	if (r < 0)
	{
		perror("kevent_qos");
		ok = 0;
	}
	for (int w = 0; w < 5000 && atomic_load(&deliveries) == 0; w++)
		usleep(1000);
	usleep(100000);
	if (atomic_load(&deliveries) != 1)
	{
		printf("FAIL: event delivered %d times\n", atomic_load(&deliveries));
		ok = 0;
	}

	printf("%s\n", ok ? "PASS" : "FAIL");
	return !ok;
}
