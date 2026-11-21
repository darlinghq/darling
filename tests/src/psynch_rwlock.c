// Copyright (C) 2026 Lubos Dolezel
//
// Process-private read-write locks under contention (psynch_rw_rdlock,
// rw_wrlock, rw_unlock): readers never overlap a writer, writers never
// overlap each other, every write is counted, readers do overlap each other,
// and a writer waiting behind readers gets in. Bounded by a watchdog.

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#define READERS 6
#define WRITERS 3
#define READ_ITERATIONS 40000
#define WRITE_ITERATIONS 10000

static pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
static volatile int readers_inside;
static volatile int writers_inside;
static volatile int max_readers_inside;
static long writes;
static int violation;

static void* reader(void* arg)
{
	(void) arg;
	for (int i = 0; i < READ_ITERATIONS; i++)
	{
		pthread_rwlock_rdlock(&rwlock);
		int inside = __atomic_add_fetch(&readers_inside, 1, __ATOMIC_SEQ_CST);
		if (__atomic_load_n(&writers_inside, __ATOMIC_SEQ_CST) != 0)
			violation = 1;
		if (inside > max_readers_inside)
			max_readers_inside = inside;
		for (volatile int k = 0; k < 50; k++)
			;
		__atomic_sub_fetch(&readers_inside, 1, __ATOMIC_SEQ_CST);
		pthread_rwlock_unlock(&rwlock);
	}
	return NULL;
}

static void* writer(void* arg)
{
	(void) arg;
	for (int i = 0; i < WRITE_ITERATIONS; i++)
	{
		pthread_rwlock_wrlock(&rwlock);
		if (__atomic_add_fetch(&writers_inside, 1, __ATOMIC_SEQ_CST) != 1 ||
			__atomic_load_n(&readers_inside, __ATOMIC_SEQ_CST) != 0)
			violation = 1;
		writes++;
		for (volatile int k = 0; k < 50; k++)
			;
		__atomic_sub_fetch(&writers_inside, 1, __ATOMIC_SEQ_CST);
		pthread_rwlock_unlock(&rwlock);
	}
	return NULL;
}

static volatile int held_by_readers;

static void* slow_reader(void* arg)
{
	(void) arg;
	pthread_rwlock_rdlock(&rwlock);
	__atomic_add_fetch(&held_by_readers, 1, __ATOMIC_SEQ_CST);
	usleep(200000);
	pthread_rwlock_unlock(&rwlock);
	return NULL;
}

int main(void)
{
	pthread_t r[READERS], w[WRITERS];
	int failures = 0;

	setvbuf(stdout, NULL, _IONBF, 0);
	alarm(200);

	for (int i = 0; i < READERS; i++)
		pthread_create(&r[i], NULL, reader, NULL);
	for (int i = 0; i < WRITERS; i++)
		pthread_create(&w[i], NULL, writer, NULL);
	for (int i = 0; i < READERS; i++)
		pthread_join(r[i], NULL);
	for (int i = 0; i < WRITERS; i++)
		pthread_join(w[i], NULL);

	if (violation || writes != (long) WRITERS * WRITE_ITERATIONS)
	{
		printf("FAIL: rwlock stress: violation %d, writes %ld of %d\n", violation, writes, WRITERS * WRITE_ITERATIONS);
		failures++;
	}
	else
		printf("ok: rwlock stress: %ld writes, up to %d readers at once\n", writes, max_readers_inside);

	// readers share the lock; a writer waits for them and then gets it
	pthread_t s[3], wt;
	for (int i = 0; i < 3; i++)
		pthread_create(&s[i], NULL, slow_reader, NULL);
	while (__atomic_load_n(&held_by_readers, __ATOMIC_SEQ_CST) < 3)
		usleep(1000);
	pthread_create(&wt, NULL, writer, NULL);
	for (int i = 0; i < 3; i++)
		pthread_join(s[i], NULL);
	pthread_join(wt, NULL);
	if (held_by_readers != 3 || violation)
	{
		printf("FAIL: shared read hold: %d readers, violation %d\n", held_by_readers, violation);
		failures++;
	}
	else
		printf("ok: 3 readers held the lock together, then the writer got it\n");

	if (failures)
		return 1;
	printf("PASS\n");
	return 0;
}
