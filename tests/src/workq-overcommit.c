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
 * Serial queues run on overcommit threads, which the constrained pool limit
 * does not hold back: 200 serial queues whose blocks each wait until all 200
 * have started must all run at the same time. macOS 27 runs 600 that way.
 */
#include <dispatch/dispatch.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <unistd.h>

#define N 200

static atomic_int started, finished;

static void watchdog(int sig)
{
	static const char msg[] = "FAIL: timed out\n";
	write(1, msg, sizeof(msg) - 1);
	_exit(1);
}

int main(void)
{
	signal(SIGALRM, watchdog);
	alarm(60);

	for (int i = 0; i < N; i++)
	{
		dispatch_queue_t q = dispatch_queue_create("org.darlinghq.test.overcommit", DISPATCH_QUEUE_SERIAL);
		dispatch_async(q, ^{
			atomic_fetch_add(&started, 1);
			for (int w = 0; w < 10000 && atomic_load(&started) < N; w++)
				usleep(1000);
			atomic_fetch_add(&finished, 1);
		});
	}

	for (int w = 0; w < 20000 && atomic_load(&finished) < N; w++)
		usleep(1000);

	int ok = atomic_load(&finished) == N;
	printf("%s: %d of %d serial queues ran together\n", ok ? "PASS" : "FAIL", atomic_load(&finished), N);
	return !ok;
}
