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
 * GCD thread pool saturation. Blocks that block are submitted one at a time
 * to the default global (constrained) queue until the pool stops starting
 * them. The submitting thread must never be held up by the limit: a
 * dispatch_async() that reaches the kernel's thread request while every
 * worker is busy is queued there and returns at once, as on macOS. Once the
 * blocks are released, every one of them must run.
 *
 * macOS 27 (14 CPUs): 70 blocks run at once (kern.wq_max_constrained_threads),
 * dispatch_async takes well under a millisecond, everything completes.
 */
#include <dispatch/dispatch.h>
#include <mach/mach_time.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define N 120

static atomic_int started, finished;

static double now_ms(void)
{
	static mach_timebase_info_data_t tb;
	if (!tb.denom)
		mach_timebase_info(&tb);
	return (double) mach_absolute_time() * tb.numer / tb.denom / 1e6;
}

static void watchdog(int sig)
{
	static const char msg[] = "FAIL: timed out (the submitting thread or the pool hung)\n";
	write(1, msg, sizeof(msg) - 1);
	_exit(1);
}

int main(void)
{
	dispatch_queue_t q = dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0);
	dispatch_semaphore_t release = dispatch_semaphore_create(0);
	double worst = 0;
	int failures = 0, i, at_limit = -1;

	signal(SIGALRM, watchdog);
	alarm(60);

	for (i = 0; i < N; i++)
	{
		double t = now_ms(), d;

		dispatch_async(q, ^{
			atomic_fetch_add(&started, 1);
			dispatch_semaphore_wait(release, DISPATCH_TIME_FOREVER);
			atomic_fetch_add(&finished, 1);
		});
		d = now_ms() - t;
		if (d > worst)
			worst = d;

		// Give the block a moment to start, so that the next submission
		// finds the queue empty and has to ask for a thread again.
		for (int w = 0; w < 200 && atomic_load(&started) <= i; w++)
			usleep(1000);
		if (atomic_load(&started) <= i && at_limit < 0)
			at_limit = i;
	}

	printf("%d of %d blocks running (pool limit reached at %d), slowest dispatch_async %.3f ms\n",
		atomic_load(&started), N, at_limit, worst);
	if (at_limit < 0)
		printf("note: no limit reached with %d blocks\n", N);
	if (worst < 100)
		printf("PASS: dispatch_async never waits for a worker\n");
	else
	{
		printf("FAIL: dispatch_async took %.3f ms\n", worst);
		failures++;
	}

	for (i = 0; i < N; i++)
		dispatch_semaphore_signal(release);
	for (i = 0; i < 20000 && atomic_load(&finished) < N; i++)
		usleep(1000);

	if (atomic_load(&finished) == N)
		printf("PASS: all %d blocks ran\n", N);
	else
	{
		printf("FAIL: %d of %d blocks ran\n", atomic_load(&finished), N);
		failures++;
	}

	printf(failures ? "FAILED\n" : "ALL PASSED\n");
	return failures != 0;
}
