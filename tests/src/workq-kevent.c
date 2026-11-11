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
 * Workqueue kevent deliveries from several threads at once: every thread
 * registers its own one-shot EVFILT_USER event on the process's workqueue
 * kqueue (kevent_qos with KEVENT_FLAG_WORKQ) and triggers it. Each event must
 * reach the kevent worker function exactly once. Uses the libpthread
 * workqueue SPI directly, so it must not touch libdispatch.
 */
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/event.h>
#include <unistd.h>

// SPI from the open-source xnu <sys/event.h> and libpthread
// <pthread/workqueue_private.h>.
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
#define KEVENT_FLAG_WORKQ_ 0x020
extern int kevent_qos(int kq, const struct kevent_qos_s_* changelist, int nchanges,
		struct kevent_qos_s_* eventlist, int nevents, void* data_out,
		size_t* data_available, unsigned int flags);
typedef unsigned long pthread_priority_t_;
extern int _pthread_workqueue_init_with_kevent(void (*queue_func)(pthread_priority_t_),
		void (*kevent_func)(void** events, int* nevents), int offset, int flags);

#define THREADS 8
#define ROUNDS 50
#define TOTAL (THREADS * ROUNDS)

static atomic_int delivered[TOTAL];
static atomic_int stray, total;

static void worker(pthread_priority_t_ pp)
{
}

static void kevent_worker(void** events, int* nevents)
{
	struct kevent_qos_s_* ev = *events;

	for (int i = 0; i < *nevents; i++)
	{
		if (ev[i].filter == EVFILT_USER && ev[i].ident >= 1000 && ev[i].ident < 1000 + TOTAL)
			atomic_fetch_add(&delivered[ev[i].ident - 1000], 1);
		else
			atomic_fetch_add(&stray, 1);
		atomic_fetch_add(&total, 1);
	}
	*nevents = 0;
}

static void* submitter(void* arg)
{
	long t = (long) arg;

	for (int r = 0; r < ROUNDS; r++)
	{
		struct kevent_qos_s_ ke;

		memset(&ke, 0, sizeof(ke));
		ke.ident = 1000 + t * ROUNDS + r;
		ke.filter = EVFILT_USER;
		ke.flags = EV_ADD | EV_ONESHOT;
		ke.fflags = NOTE_TRIGGER;
		if (kevent_qos(-1, &ke, 1, NULL, 0, NULL, NULL, KEVENT_FLAG_WORKQ_) < 0)
			perror("kevent_qos");
	}
	return NULL;
}

static void watchdog(int sig)
{
	static const char msg[] = "FAIL: timed out\n";
	write(1, msg, sizeof(msg) - 1);
	_exit(1);
}

int main(void)
{
	pthread_t th[THREADS];
	int r, missing = 0, dup = 0;

	signal(SIGALRM, watchdog);
	alarm(60);

	r = _pthread_workqueue_init_with_kevent(worker, kevent_worker, 0, 0);
	if (r != 0)
	{
		printf("SKIP: workqueue already set up (%d)\n", r);
		return 0;
	}

	for (long t = 0; t < THREADS; t++)
		pthread_create(&th[t], NULL, submitter, (void*) t);
	for (int t = 0; t < THREADS; t++)
		pthread_join(th[t], NULL);

	for (int w = 0; w < 10000 && atomic_load(&total) < TOTAL; w++)
		usleep(1000);
	usleep(100000);

	for (int i = 0; i < TOTAL; i++)
	{
		int n = atomic_load(&delivered[i]);
		if (n == 0)
			missing++;
		else if (n > 1)
			dup++;
	}

	r = (missing == 0 && dup == 0 && atomic_load(&stray) == 0);
	printf("%s: %d events delivered (%d missing, %d twice, %d stray)\n",
		r ? "PASS" : "FAIL", atomic_load(&total), missing, dup, atomic_load(&stray));
	return !r;
}
