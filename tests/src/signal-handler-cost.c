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
 * Signal delivery micro-benchmark: 20000 handled SIGUSR1 round trips
 * (raise() and a pthread_kill() to a second thread). Every handled signal
 * used to make about two dozen trace requests to darlingserver; without them
 * a round trip takes about a hundred microseconds. The correctness checks
 * are that every raise() ran its handler and that most pthread_kill signals
 * did (pending signals of one number coalesce); the elapsed time is printed,
 * and the test fails if it is above a bound (3 s per 10000 signals, see BOUND_US).
 */
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#define N 10000
// Calibrated on a local Darling build: with the trace output compiled in a
// round trip took 4.5 s (i386) to 24-44 s (x86_64) per 10000 signals, without it
// 1.4-1.5 s; the bound sits between the two.
#define BOUND_US 3000000

static atomic_int handled;
static atomic_int remote;

static void handler(int sig)
{
	atomic_fetch_add(&handled, 1);
}

static void* target(void* arg)
{
	while (!atomic_load(&remote))
		usleep(100);
	for (int i = 0; i < 20000 && atomic_load(&remote) != 2; i++)
		usleep(500);
	return NULL;
}

static void watchdog(int sig)
{
	static const char msg[] = "FAIL: timed out\n";
	write(1, msg, sizeof(msg) - 1);
	_exit(1);
}

static long long now(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000LL + tv.tv_usec;
}

int main(void)
{
	struct sigaction sa = { .sa_handler = handler };
	pthread_t th;
	long long start, elapsed;
	int ok;

	signal(SIGALRM, watchdog);
	alarm(120);
	sigaction(SIGUSR1, &sa, NULL);

	start = now();
	for (int i = 0; i < N; i++)
		raise(SIGUSR1);
	elapsed = now() - start;
	printf("%d raise() round trips: %lld us (%lld ns each)\n", N, elapsed, elapsed * 1000 / N);
	ok = (atomic_load(&handled) == N) && elapsed < BOUND_US;

	pthread_create(&th, NULL, target, NULL);
	atomic_store(&remote, 1);
	start = now();
	for (int i = 0; i < N; i++)
		pthread_kill(th, SIGUSR1);
	elapsed = now() - start;
	// pthread_kill is asynchronous: wait until the handlers stop running
	for (int last = -1, stable = 0; stable < 20;)
	{
		int cur = atomic_load(&handled);
		stable = (cur == last) ? stable + 1 : 0;
		last = cur;
		usleep(5000);
	}
	atomic_store(&remote, 2);
	pthread_join(th, NULL);
	printf("%d pthread_kill() signals: %lld us\n", N, elapsed);
	// Signals of the same number may coalesce while pending, so only require
	// that most of them ran.
	ok = ok && atomic_load(&handled) > N + N / 10 && elapsed < BOUND_US;

	printf("%s: %d handlers ran\n", ok ? "PASS" : "FAIL", atomic_load(&handled));
	return !ok;
}
