// Copyright (C) 2026 Lubos Dolezel
//
// Throughput of contended process-private pthread objects, for comparing
// psynch implementations (not a pass/fail test beyond completing):
// - mutex: 4 threads lock and unlock one mutex with a short critical section
//   for 2 s; lock acquisitions per second;
// - handoff: 2 threads pass the mutex strictly in turn (each waits for the
//   other's turn under the mutex with a condition variable); handoffs per second;
// - cv round trip: ping-pong between two threads through two condition
//   variables; round trips per second.
// Usage: psynch_bench [seconds]

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

static double seconds = 2.0;
static volatile int stop;

static double now(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + tv.tv_usec / 1e6;
}

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static long acquisitions[4];

static void* mutex_worker(void* arg)
{
	int me = (int) (intptr_t) arg;
	long n = 0;

	while (!stop)
	{
		pthread_mutex_lock(&mutex);
		for (volatile int k = 0; k < 30; k++)
			;
		pthread_mutex_unlock(&mutex);
		n++;
	}
	acquisitions[me] = n;
	return NULL;
}

static pthread_mutex_t pp_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t pp_cond[2] = { PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER };
static int pp_turn;
static long pp_rounds;

static void* pingpong(void* arg)
{
	int me = (int) (intptr_t) arg;

	pthread_mutex_lock(&pp_mutex);
	while (!stop)
	{
		while (pp_turn != me && !stop)
			pthread_cond_wait(&pp_cond[me], &pp_mutex);
		if (me == 0)
			pp_rounds++;
		pp_turn = !me;
		pthread_cond_signal(&pp_cond[!me]);
	}
	pp_turn = !me;
	pthread_cond_signal(&pp_cond[!me]);
	pthread_mutex_unlock(&pp_mutex);
	return NULL;
}

static pthread_mutex_t ho_mutex = PTHREAD_MUTEX_INITIALIZER;
static volatile int ho_turn;
static long ho_count;

// strict alternation with a spin-free wait: whoever does not have the turn
// unlocks and relocks, so the mutex itself is handed over each time
static void* handoff(void* arg)
{
	int me = (int) (intptr_t) arg;

	while (!stop)
	{
		pthread_mutex_lock(&ho_mutex);
		if (ho_turn == me)
		{
			ho_count++;
			ho_turn = !me;
		}
		pthread_mutex_unlock(&ho_mutex);
	}
	return NULL;
}

static double run(void* (*fn)(void*), int threads)
{
	pthread_t t[4];
	double start;

	stop = 0;
	for (int i = 0; i < threads; i++)
		pthread_create(&t[i], NULL, fn, (void*) (intptr_t) i);
	start = now();
	usleep((useconds_t) (seconds * 1e6));
	stop = 1;
	for (int i = 0; i < threads; i++)
		pthread_join(t[i], NULL);
	return now() - start;
}

int main(int argc, char** argv)
{
	double elapsed;

	if (argc > 1)
		seconds = atof(argv[1]);
	setvbuf(stdout, NULL, _IONBF, 0);
	alarm((unsigned) (seconds * 3 + 30));

	elapsed = run(mutex_worker, 4);
	long total = acquisitions[0] + acquisitions[1] + acquisitions[2] + acquisitions[3];
	printf("mutex: %.0f acquisitions/s (4 threads)\n", total / elapsed);

	elapsed = run(handoff, 2);
	printf("handoff: %.0f handoffs/s (2 threads)\n", ho_count / elapsed);

	elapsed = run(pingpong, 2);
	printf("cv round trip: %.0f round trips/s\n", pp_rounds / elapsed);

	printf("PASS\n");
	return 0;
}
