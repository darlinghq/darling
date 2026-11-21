// Copyright (C) 2026 Lubos Dolezel
//
// Process-private condition variables (psynch_cvwait, cvsignal, cvbroad,
// cvclrprepost):
// - ping-pong: two threads hand a token back and forth many times;
// - producer/consumer: several producers and consumers through a bounded
//   queue with pthread_cond_signal, every item consumed exactly once;
// - broadcast: many waiters all wake on each of many broadcasts;
// - pthread_cond_signal_thread_np wakes the named waiter;
// - pthread_cond_timedwait with an absolute and
//   pthread_cond_timedwait_relative_np with a relative timeout return
//   ETIMEDOUT no earlier than the deadline and not much later, with the mutex
//   held again; a timed wait that is signalled returns 0 well before it.
// A watchdog turns a lost wakeup into a failure.

#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

static int failures;

#define CHECK(cond, ...) do { if (!(cond)) { printf("FAIL: " __VA_ARGS__); printf("\n"); failures++; } } while (0)

static double now(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + tv.tv_usec / 1e6;
}

// ---- ping-pong ----

#define PINGPONG_ROUNDS 50000

static pthread_mutex_t pp_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t pp_cond = PTHREAD_COND_INITIALIZER;
static int pp_turn;
static long pp_count;

static void* pingpong(void* arg)
{
	int me = (int) (intptr_t) arg;

	pthread_mutex_lock(&pp_mutex);
	for (int i = 0; i < PINGPONG_ROUNDS; i++)
	{
		while (pp_turn != me)
			pthread_cond_wait(&pp_cond, &pp_mutex);
		pp_count++;
		pp_turn = !me;
		pthread_cond_signal(&pp_cond);
	}
	pthread_mutex_unlock(&pp_mutex);
	return NULL;
}

static void test_pingpong(void)
{
	pthread_t a, b;
	double start = now();

	pthread_create(&a, NULL, pingpong, (void*) 0);
	pthread_create(&b, NULL, pingpong, (void*) 1);
	pthread_join(a, NULL);
	pthread_join(b, NULL);
	CHECK(pp_count == 2 * PINGPONG_ROUNDS, "ping-pong: %ld handoffs, expected %d", pp_count, 2 * PINGPONG_ROUNDS);
	printf("ok: ping-pong: %ld handoffs in %.2f s\n", pp_count, now() - start);
}

// ---- producer/consumer ----

#define QUEUE_SIZE 16
#define PRODUCERS 4
#define CONSUMERS 4
#define ITEMS_PER_PRODUCER 25000

static pthread_mutex_t q_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t q_not_empty = PTHREAD_COND_INITIALIZER;
static pthread_cond_t q_not_full = PTHREAD_COND_INITIALIZER;
static int q_items[QUEUE_SIZE];
static int q_head, q_count, q_done_producers;
static long q_sum;
static long q_consumed;

static void* producer(void* arg)
{
	int base = (int) (intptr_t) arg * ITEMS_PER_PRODUCER;

	for (int i = 1; i <= ITEMS_PER_PRODUCER; i++)
	{
		pthread_mutex_lock(&q_mutex);
		while (q_count == QUEUE_SIZE)
			pthread_cond_wait(&q_not_full, &q_mutex);
		q_items[(q_head + q_count) % QUEUE_SIZE] = base + i;
		q_count++;
		pthread_cond_signal(&q_not_empty);
		pthread_mutex_unlock(&q_mutex);
	}

	pthread_mutex_lock(&q_mutex);
	q_done_producers++;
	pthread_cond_broadcast(&q_not_empty);
	pthread_mutex_unlock(&q_mutex);
	return NULL;
}

static void* consumer(void* arg)
{
	(void) arg;
	pthread_mutex_lock(&q_mutex);
	for (;;)
	{
		while (q_count == 0 && q_done_producers < PRODUCERS)
			pthread_cond_wait(&q_not_empty, &q_mutex);
		if (q_count == 0)
			break;
		q_sum += q_items[q_head];
		q_consumed++;
		q_head = (q_head + 1) % QUEUE_SIZE;
		q_count--;
		pthread_cond_signal(&q_not_full);
	}
	pthread_mutex_unlock(&q_mutex);
	return NULL;
}

static void test_producer_consumer(void)
{
	pthread_t p[PRODUCERS], c[CONSUMERS];
	long expected_sum = 0;

	for (int k = 0; k < PRODUCERS; k++)
		for (int i = 1; i <= ITEMS_PER_PRODUCER; i++)
			expected_sum += (long) k * ITEMS_PER_PRODUCER + i;

	for (int i = 0; i < CONSUMERS; i++)
		pthread_create(&c[i], NULL, consumer, NULL);
	for (int i = 0; i < PRODUCERS; i++)
		pthread_create(&p[i], NULL, producer, (void*) (intptr_t) i);
	for (int i = 0; i < PRODUCERS; i++)
		pthread_join(p[i], NULL);
	for (int i = 0; i < CONSUMERS; i++)
		pthread_join(c[i], NULL);

	CHECK(q_consumed == (long) PRODUCERS * ITEMS_PER_PRODUCER && q_sum == expected_sum,
		"producer/consumer: consumed %ld (sum %ld), expected %ld (sum %ld)",
		q_consumed, q_sum, (long) PRODUCERS * ITEMS_PER_PRODUCER, expected_sum);
	printf("ok: producer/consumer: %ld items\n", q_consumed);
}

// ---- broadcast ----

#define BROADCAST_WAITERS 32
#define BROADCAST_ROUNDS 300

static pthread_mutex_t b_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t b_cond = PTHREAD_COND_INITIALIZER;
static pthread_cond_t b_arrived = PTHREAD_COND_INITIALIZER;
static int b_generation;
static int b_waiting;
static long b_wakeups;

static void* broadcast_waiter(void* arg)
{
	(void) arg;
	pthread_mutex_lock(&b_mutex);
	for (int round = 0; round < BROADCAST_ROUNDS; round++)
	{
		int generation = b_generation;
		b_waiting++;
		pthread_cond_signal(&b_arrived);
		while (b_generation == generation)
			pthread_cond_wait(&b_cond, &b_mutex);
		b_wakeups++;
	}
	pthread_mutex_unlock(&b_mutex);
	return NULL;
}

static void test_broadcast(void)
{
	pthread_t t[BROADCAST_WAITERS];

	for (int i = 0; i < BROADCAST_WAITERS; i++)
		pthread_create(&t[i], NULL, broadcast_waiter, NULL);

	pthread_mutex_lock(&b_mutex);
	for (int round = 0; round < BROADCAST_ROUNDS; round++)
	{
		// all waiters are in this round's wait (or about to check the generation)
		while (b_waiting < BROADCAST_WAITERS)
			pthread_cond_wait(&b_arrived, &b_mutex);
		b_waiting = 0;
		b_generation++;
		pthread_cond_broadcast(&b_cond);
	}
	pthread_mutex_unlock(&b_mutex);

	for (int i = 0; i < BROADCAST_WAITERS; i++)
		pthread_join(t[i], NULL);
	CHECK(b_wakeups == (long) BROADCAST_WAITERS * BROADCAST_ROUNDS, "broadcast: %ld wakeups, expected %d",
		b_wakeups, BROADCAST_WAITERS * BROADCAST_ROUNDS);
	printf("ok: broadcast: %d waiters x %d rounds\n", BROADCAST_WAITERS, BROADCAST_ROUNDS);
}

// ---- signal a named thread ----

static pthread_mutex_t n_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t n_cond = PTHREAD_COND_INITIALIZER;
static int n_waiting;
static int n_woken[2];
static int n_go[2];

static void* named_waiter(void* arg)
{
	int me = (int) (intptr_t) arg;

	pthread_mutex_lock(&n_mutex);
	n_waiting++;
	while (!n_go[me])
		pthread_cond_wait(&n_cond, &n_mutex);
	n_woken[me] = 1;
	pthread_mutex_unlock(&n_mutex);
	return NULL;
}

static void test_signal_thread(void)
{
	pthread_t t[2];

	for (int i = 0; i < 2; i++)
		pthread_create(&t[i], NULL, named_waiter, (void*) (intptr_t) i);

	for (;;)
	{
		pthread_mutex_lock(&n_mutex);
		int waiting = n_waiting;
		pthread_mutex_unlock(&n_mutex);
		if (waiting == 2)
			break;
		usleep(1000);
	}
	usleep(50000);

	pthread_mutex_lock(&n_mutex);
	n_go[1] = 1;
	pthread_cond_signal_thread_np(&n_cond, t[1]);
	pthread_mutex_unlock(&n_mutex);
	pthread_join(t[1], NULL);

	pthread_mutex_lock(&n_mutex);
	CHECK(n_woken[1] == 1 && n_woken[0] == 0, "signal_thread_np: woken %d %d", n_woken[0], n_woken[1]);
	n_go[0] = 1;
	pthread_cond_signal_thread_np(&n_cond, t[0]);
	pthread_mutex_unlock(&n_mutex);
	pthread_join(t[0], NULL);
	printf("ok: pthread_cond_signal_thread_np\n");
}

// ---- timed waits ----

static pthread_mutex_t t_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t t_cond = PTHREAD_COND_INITIALIZER;
static int t_flag;

static void check_timeout(const char* name, int relative, double seconds)
{
	struct timespec ts;
	double start, elapsed;
	int ret;

	pthread_mutex_lock(&t_mutex);
	start = now();
	if (relative)
	{
		ts.tv_sec = (time_t) seconds;
		ts.tv_nsec = (long) ((seconds - (time_t) seconds) * 1e9);
		ret = pthread_cond_timedwait_relative_np(&t_cond, &t_mutex, &ts);
	}
	else
	{
		double deadline = start + seconds;
		ts.tv_sec = (time_t) deadline;
		ts.tv_nsec = (long) ((deadline - (time_t) deadline) * 1e9);
		ret = pthread_cond_timedwait(&t_cond, &t_mutex, &ts);
	}
	elapsed = now() - start;

	// the mutex is held again: a trylock from this thread on a normal mutex fails
	int relocked = pthread_mutex_trylock(&t_mutex) == EBUSY;
	pthread_mutex_unlock(&t_mutex);

	CHECK(ret == ETIMEDOUT, "%s %.3f s: returned %d", name, seconds, ret);
	CHECK(relocked, "%s: mutex not held after the wait", name);
	// a little slack below for the clock granularity; scheduling may add some above
	CHECK(elapsed >= seconds - 0.002 && elapsed < seconds + 0.25, "%s %.3f s: took %.4f s", name, seconds, elapsed);
	printf("ok: %s %.3f s: ETIMEDOUT after %.4f s (late by %.1f ms)\n", name, seconds, elapsed, (elapsed - seconds) * 1000);
}

static void* timed_signaller(void* arg)
{
	(void) arg;
	usleep(100000);
	pthread_mutex_lock(&t_mutex);
	t_flag = 1;
	pthread_cond_signal(&t_cond);
	pthread_mutex_unlock(&t_mutex);
	return NULL;
}

static void test_timed(void)
{
	check_timeout("absolute timedwait", 0, 0.05);
	check_timeout("absolute timedwait", 0, 0.5);
	check_timeout("relative timedwait", 1, 0.05);
	check_timeout("relative timedwait", 1, 0.5);

	// an absolute deadline in the past times out at once
	struct timespec past = { 1, 0 };
	pthread_mutex_lock(&t_mutex);
	int ret = pthread_cond_timedwait(&t_cond, &t_mutex, &past);
	pthread_mutex_unlock(&t_mutex);
	CHECK(ret == ETIMEDOUT, "timedwait with a past deadline returned %d", ret);

	// signalled before the deadline: 0, early
	pthread_t t;
	struct timespec ts = { 5, 0 };
	double start = now();
	pthread_create(&t, NULL, timed_signaller, NULL);
	pthread_mutex_lock(&t_mutex);
	ret = 0;
	while (!t_flag && ret == 0)
		ret = pthread_cond_timedwait_relative_np(&t_cond, &t_mutex, &ts);
	pthread_mutex_unlock(&t_mutex);
	pthread_join(t, NULL);
	double elapsed = now() - start;
	CHECK(ret == 0 && t_flag && elapsed < 2, "signalled timedwait: ret %d after %.3f s", ret, elapsed);
	printf("ok: signalled timedwait returned 0 after %.3f s\n", elapsed);
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	alarm(240);

	test_pingpong();
	test_producer_consumer();
	test_broadcast();
	test_signal_thread();
	test_timed();

	if (failures)
		return 1;
	printf("PASS\n");
	return 0;
}
