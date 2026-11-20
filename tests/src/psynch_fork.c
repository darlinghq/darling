// Copyright (C) 2026 Lubos Dolezel
//
// fork() while other threads hold and wait on process-private mutexes and
// condition variables: the child (with only the forking thread) can use a
// mutex that was unlocked at the time of the fork and fresh mutexes and
// condition variables with new threads of its own, under contention. The
// parent keeps working. Repeated many times, so a child that inherits a
// locked internal state or stale wait queues hangs (and the watchdog fails it).
//
// A PTHREAD_PROCESS_SHARED mutex and condition variable in MAP_SHARED memory
// still synchronize parent and child across fork.

#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUSY_THREADS 6
#define FORKS 40
#define CHILD_THREADS 4
#define CHILD_ITERATIONS 20000

static pthread_mutex_t busy_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t busy_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t free_mutex = PTHREAD_MUTEX_INITIALIZER;
static volatile int stop;
static long busy_counter;

// keep the mutex and the condition variable contended and waited on
static void* busy(void* arg)
{
	(void) arg;
	while (!stop)
	{
		pthread_mutex_lock(&busy_mutex);
		busy_counter++;
		if (busy_counter % 3 == 0)
			pthread_cond_broadcast(&busy_cond);
		else
		{
			struct timespec ts = { 0, 1000000 };
			pthread_cond_timedwait_relative_np(&busy_cond, &busy_mutex, &ts);
		}
		pthread_mutex_unlock(&busy_mutex);
	}
	return NULL;
}

static pthread_mutex_t child_mutex;
static pthread_cond_t child_cond;
static long child_counter;
static int child_turn;

static void* child_worker(void* arg)
{
	int me = (int) (intptr_t) arg;

	for (int i = 0; i < CHILD_ITERATIONS; i++)
	{
		pthread_mutex_lock(&child_mutex);
		child_counter++;
		pthread_mutex_unlock(&child_mutex);
	}

	// a short round-robin through the condition variable
	pthread_mutex_lock(&child_mutex);
	for (int i = 0; i < 100; i++)
	{
		while (child_turn % CHILD_THREADS != me)
			pthread_cond_wait(&child_cond, &child_mutex);
		child_turn++;
		pthread_cond_broadcast(&child_cond);
	}
	pthread_mutex_unlock(&child_mutex);
	return NULL;
}

static int child_main(void)
{
	pthread_t t[CHILD_THREADS];

	alarm(30);

	// unlocked when the parent forked
	if (pthread_mutex_lock(&free_mutex) != 0)
		return 2;
	pthread_mutex_unlock(&free_mutex);

	pthread_mutex_init(&child_mutex, NULL);
	pthread_cond_init(&child_cond, NULL);
	for (int i = 0; i < CHILD_THREADS; i++)
		pthread_create(&t[i], NULL, child_worker, (void*) (intptr_t) i);
	for (int i = 0; i < CHILD_THREADS; i++)
		pthread_join(t[i], NULL);

	if (child_counter != (long) CHILD_THREADS * CHILD_ITERATIONS || child_turn != CHILD_THREADS * 100)
		return 3;
	return 0;
}

struct shared {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	int value;
};

static int test_pshared(void)
{
	struct shared* sh = mmap(NULL, sizeof(*sh), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
	pthread_mutexattr_t ma;
	pthread_condattr_t ca;
	int status;

	pthread_mutexattr_init(&ma);
	pthread_mutexattr_setpshared(&ma, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&sh->mutex, &ma);
	pthread_condattr_init(&ca);
	pthread_condattr_setpshared(&ca, PTHREAD_PROCESS_SHARED);
	pthread_cond_init(&sh->cond, &ca);
	sh->value = 0;

	pid_t pid = fork();
	if (pid == 0)
	{
		alarm(30);
		// ping-pong with the parent: child takes odd values
		pthread_mutex_lock(&sh->mutex);
		for (int i = 0; i < 200; i++)
		{
			while (sh->value % 2 == 0)
				pthread_cond_wait(&sh->cond, &sh->mutex);
			sh->value++;
			pthread_cond_signal(&sh->cond);
		}
		pthread_mutex_unlock(&sh->mutex);
		_exit(0);
	}

	pthread_mutex_lock(&sh->mutex);
	for (int i = 0; i < 200; i++)
	{
		while (sh->value % 2 == 1)
			pthread_cond_wait(&sh->cond, &sh->mutex);
		sh->value++;
		pthread_cond_signal(&sh->cond);
	}
	while (sh->value != 400)
		pthread_cond_wait(&sh->cond, &sh->mutex);
	pthread_mutex_unlock(&sh->mutex);

	waitpid(pid, &status, 0);
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0 || sh->value != 400)
	{
		printf("FAIL: process-shared ping-pong: value %d, child status 0x%x\n", sh->value, status);
		return 1;
	}
	printf("ok: process-shared mutex and condvar across fork: %d handoffs\n", sh->value);
	return 0;
}

int main(void)
{
	pthread_t threads[BUSY_THREADS];
	int failures = 0;

	setvbuf(stdout, NULL, _IONBF, 0);
	alarm(200);

	for (int i = 0; i < BUSY_THREADS; i++)
		pthread_create(&threads[i], NULL, busy, NULL);

	for (int i = 0; i < FORKS; i++)
	{
		int status;
		pid_t pid = fork();

		if (pid == 0)
			_exit(child_main());
		if (pid < 0)
		{
			printf("FAIL: fork: %d\n", errno);
			failures++;
			break;
		}
		waitpid(pid, &status, 0);
		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		{
			printf("FAIL: fork %d: child status 0x%x\n", i, status);
			failures++;
		}
	}

	stop = 1;
	for (int i = 0; i < BUSY_THREADS; i++)
		pthread_join(threads[i], NULL);
	if (failures == 0)
		printf("ok: %d forks with %d busy threads, children used locks and threads (parent counter %ld)\n",
			FORKS, BUSY_THREADS, busy_counter);

	failures += test_pshared();

	if (failures)
		return 1;
	printf("PASS\n");
	return 0;
}
