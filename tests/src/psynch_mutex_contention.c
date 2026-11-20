// Copyright (C) 2026 Lubos Dolezel
//
// Contended process-private pthread mutexes (the psynch_mutexwait and
// psynch_mutexdrop paths) keep mutual exclusion and lose no wakeup: N threads
// increment shared counters under a mutex many times, for the default
// (first-fit) and the fair-share policy, for a recursive mutex, and with many
// mutexes at once (many wait queues alive). Each counter must be exact, and
// the run is bounded by a watchdog so a lost wakeup fails instead of hanging.

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define THREADS 8
#define ITERATIONS 100000
#define MANY_MUTEXES 64

static pthread_mutex_t mutex;
static volatile long counter;
static volatile int inside;
static int failed;

static pthread_mutex_t many[MANY_MUTEXES];
static volatile long many_counter[MANY_MUTEXES];

static void* worker(void* arg)
{
	(void) arg;
	for (int i = 0; i < ITERATIONS; i++)
	{
		pthread_mutex_lock(&mutex);
		if (__atomic_add_fetch(&inside, 1, __ATOMIC_RELAXED) != 1)
			failed = 1;
		counter++;
		// a little work in the critical section keeps the mutex contended
		for (volatile int k = 0; k < 20; k++)
			;
		__atomic_sub_fetch(&inside, 1, __ATOMIC_RELAXED);
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}

static void* recursive_worker(void* arg)
{
	(void) arg;
	for (int i = 0; i < ITERATIONS / 4; i++)
	{
		pthread_mutex_lock(&mutex);
		pthread_mutex_lock(&mutex);
		counter++;
		pthread_mutex_unlock(&mutex);
		counter++;
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}

static void* many_worker(void* arg)
{
	unsigned seed = (unsigned) (uintptr_t) arg;
	for (int i = 0; i < ITERATIONS; i++)
	{
		seed = seed * 1103515245 + 12345;
		int m = (seed >> 16) % MANY_MUTEXES;
		pthread_mutex_lock(&many[m]);
		many_counter[m]++;
		pthread_mutex_unlock(&many[m]);
	}
	return NULL;
}

static int run(const char* name, void* (*fn)(void*), long expected)
{
	pthread_t threads[THREADS];

	counter = 0;
	failed = 0;
	for (int i = 0; i < THREADS; i++)
		pthread_create(&threads[i], NULL, fn, (void*) (uintptr_t) (i + 1));
	for (int i = 0; i < THREADS; i++)
		pthread_join(threads[i], NULL);

	if (counter != expected || failed)
	{
		printf("FAIL: %s: counter %ld, expected %ld, exclusion %s\n", name, counter, expected, failed ? "broken" : "ok");
		return 1;
	}
	printf("ok: %s: %ld increments\n", name, counter);
	return 0;
}

int main(void)
{
	pthread_mutexattr_t attr;
	int failures = 0;

	setvbuf(stdout, NULL, _IONBF, 0);
	alarm(240);

	pthread_mutex_init(&mutex, NULL);
	failures += run("default mutex", worker, (long) THREADS * ITERATIONS);
	pthread_mutex_destroy(&mutex);

	pthread_mutexattr_init(&attr);
	pthread_mutexattr_setpolicy_np(&attr, PTHREAD_MUTEX_POLICY_FAIRSHARE_NP);
	pthread_mutex_init(&mutex, &attr);
	failures += run("fair-share mutex", worker, (long) THREADS * ITERATIONS);
	pthread_mutex_destroy(&mutex);
	pthread_mutexattr_destroy(&attr);

	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&mutex, &attr);
	failures += run("recursive mutex", recursive_worker, (long) THREADS * (ITERATIONS / 4) * 2);
	pthread_mutex_destroy(&mutex);
	pthread_mutexattr_destroy(&attr);

	for (int i = 0; i < MANY_MUTEXES; i++)
		pthread_mutex_init(&many[i], NULL);
	pthread_t threads[THREADS];
	for (int i = 0; i < THREADS; i++)
		pthread_create(&threads[i], NULL, many_worker, (void*) (uintptr_t) (i + 1));
	for (int i = 0; i < THREADS; i++)
		pthread_join(threads[i], NULL);
	long total = 0;
	for (int i = 0; i < MANY_MUTEXES; i++)
		total += many_counter[i];
	if (total != (long) THREADS * ITERATIONS)
	{
		printf("FAIL: %d mutexes: %ld increments, expected %ld\n", MANY_MUTEXES, total, (long) THREADS * ITERATIONS);
		failures++;
	}
	else
		printf("ok: %d mutexes: %ld increments\n", MANY_MUTEXES, total);

	if (failures)
		return 1;
	printf("PASS\n");
	return 0;
}
