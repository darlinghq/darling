// Copyright (C) 2026 Lubos Dolezel
//
// POSIX: a thread that is unblocked by cancellation while blocked in
// pthread_cond_wait() must not consume a condition signal sent concurrently,
// as long as other threads are blocked on the condition variable.
//
// Each round three threads wait for a token, oldest first. The main thread
// cancels the oldest waiter and right after that (the cancel request has
// been made) adds one token and signals the condition variable. One of the
// two remaining waiters must take the token: a signal that went to the
// cancelled thread and ended with it is a lost wakeup, and the token stays.

#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#define ROUNDS 300
#define WAITERS 3

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static pthread_cond_t arrived = PTHREAD_COND_INITIALIZER;
static int tokens;
static int started;
static int stop;

static void unlock_mutex(void* arg)
{
	(void) arg;
	pthread_mutex_unlock(&mutex);
}

static void* waiter(void* arg)
{
	(void) arg;
	pthread_mutex_lock(&mutex);
	pthread_cleanup_push(unlock_mutex, NULL);
	started++;
	pthread_cond_broadcast(&arrived);
	while (tokens == 0 && !stop)
		pthread_cond_wait(&cond, &mutex);
	if (tokens > 0)
		tokens--;
	pthread_cleanup_pop(1);
	return NULL;
}

static void wait_started(int n)
{
	pthread_mutex_lock(&mutex);
	while (started < n)
		pthread_cond_wait(&arrived, &mutex);
	pthread_mutex_unlock(&mutex);
	// let the last one get from the mutex into its condition wait
	usleep(3000);
}

static double now(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + tv.tv_usec / 1e6;
}

int main(void)
{
	int lost = 0, cancelled = 0;

	setvbuf(stdout, NULL, _IONBF, 0);
	alarm(240);

	for (int round = 0; round < ROUNDS; round++)
	{
		pthread_t t[WAITERS];
		void* result;

		tokens = 0;
		started = 0;
		stop = 0;

		// the first waiter is the oldest, so it is the one a signal goes to
		pthread_create(&t[0], NULL, waiter, NULL);
		wait_started(1);
		for (int i = 1; i < WAITERS; i++)
			pthread_create(&t[i], NULL, waiter, NULL);
		wait_started(WAITERS);

		pthread_cancel(t[0]);

		pthread_mutex_lock(&mutex);
		tokens = 1;
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&mutex);

		pthread_join(t[0], &result);
		if (result == PTHREAD_CANCELED)
			cancelled++;

		// one of the other waiters takes the token
		double deadline = now() + 2;
		int left;
		do
		{
			pthread_mutex_lock(&mutex);
			left = tokens;
			pthread_mutex_unlock(&mutex);
			if (left != 0)
				usleep(1000);
		}
		while (left != 0 && now() < deadline);

		// a waiter that was cancelled after it took the token leaves it taken,
		// which is fine; only a token nobody took is a lost signal
		if (left != 0)
		{
			lost++;
			if (lost <= 5)
				printf("round %d: the signal was lost (the cancelled waiter %s)\n", round,
					result == PTHREAD_CANCELED ? "was cancelled" : "returned normally");
		}

		pthread_mutex_lock(&mutex);
		stop = 1;
		pthread_cond_broadcast(&cond);
		pthread_mutex_unlock(&mutex);
		for (int i = 1; i < WAITERS; i++)
			pthread_join(t[i], NULL);

		if (lost >= 5)
			break;
	}

	printf("%d waiters cancelled, %d signals lost\n", cancelled, lost);
	if (lost != 0)
	{
		printf("FAIL\n");
		return 1;
	}
	printf("PASS\n");
	return 0;
}
