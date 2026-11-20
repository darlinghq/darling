// Copyright (C) 2026 Lubos Dolezel
//
// Cancellation and signals around process-private psynch waits:
// - pthread_cancel of a thread blocked in pthread_cond_wait and in
//   pthread_cond_timedwait ends it with PTHREAD_CANCELED; its cleanup handler
//   runs with the mutex held, and the condition variable keeps working for
//   the other waiters afterwards;
// - with cancellation disabled, pthread_cancel does not end the wait; a later
//   signal does, and the thread is cancelled at the next cancellation point;
// - a signal handler (without SA_RESTART, and with it) interrupting a thread
//   blocked in pthread_cond_wait or pthread_mutex_lock: the mutex wait never
//   fails (POSIX: no EINTR), the condition wait returns 0 (possibly
//   spuriously) and the thread still sees the real signal later;
// - the handler itself locks and signals other process-private objects while
//   the interrupted thread is inside its wait.

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static int failures;

#define CHECK(cond, ...) do { if (!(cond)) { printf("FAIL: " __VA_ARGS__); printf("\n"); failures++; } } while (0)

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static volatile int waiting;
static volatile int cleanup_ran;
static volatile int cleanup_had_mutex;
static int predicate;

static void wait_until(volatile int* value, int expected)
{
	for (int i = 0; i < 5000 && *value != expected; i++)
		usleep(1000);
}

static void cleanup(void* arg)
{
	(void) arg;
	// the mutex is held again when a cancelled condition wait runs its cleanup handlers
	cleanup_had_mutex = pthread_mutex_trylock(&mutex) == EBUSY;
	cleanup_ran = 1;
	pthread_mutex_unlock(&mutex);
}

static void* cancel_waiter(void* arg)
{
	int timed = (int) (intptr_t) arg;

	pthread_mutex_lock(&mutex);
	pthread_cleanup_push(cleanup, NULL);
	waiting = 1;
	while (!predicate)
	{
		if (timed)
		{
			// (pthread_cond_timedwait_relative_np is not a cancellation point)
			struct timespec ts = { time(NULL) + 30, 0 };
			pthread_cond_timedwait(&cond, &mutex, &ts);
		}
		else
			pthread_cond_wait(&cond, &mutex);
	}
	pthread_cleanup_pop(0);
	pthread_mutex_unlock(&mutex);
	return NULL;
}

static void* plain_waiter(void* arg)
{
	(void) arg;
	pthread_mutex_lock(&mutex);
	while (!predicate)
		pthread_cond_wait(&cond, &mutex);
	pthread_mutex_unlock(&mutex);
	return (void*) 1;
}

static void test_cancel(int timed)
{
	pthread_t victim, other;
	void* result;
	const char* name = timed ? "cancel in timedwait" : "cancel in cond_wait";

	predicate = 0;
	waiting = 0;
	cleanup_ran = 0;
	cleanup_had_mutex = 0;

	pthread_create(&other, NULL, plain_waiter, NULL);
	pthread_create(&victim, NULL, cancel_waiter, (void*) (intptr_t) timed);
	wait_until(&waiting, 1);
	usleep(50000);

	pthread_cancel(victim);
	pthread_join(victim, &result);
	CHECK(result == PTHREAD_CANCELED, "%s: thread result %p", name, result);
	CHECK(cleanup_ran && cleanup_had_mutex, "%s: cleanup ran %d, with the mutex %d", name, cleanup_ran, cleanup_had_mutex);

	// the other waiter is still woken normally
	pthread_mutex_lock(&mutex);
	predicate = 1;
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);
	pthread_join(other, &result);
	CHECK(result == (void*) 1, "%s: the other waiter returned %p", name, result);
	printf("ok: %s\n", name);
}

static void* disabled_waiter(void* arg)
{
	int old;
	(void) arg;

	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old);
	pthread_mutex_lock(&mutex);
	waiting = 1;
	while (!predicate)
		pthread_cond_wait(&cond, &mutex);
	pthread_mutex_unlock(&mutex);
	waiting = 2;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &old);
	pthread_testcancel();
	return (void*) 1;
}

static void test_cancel_disabled(void)
{
	pthread_t t;
	void* result;

	predicate = 0;
	waiting = 0;
	pthread_create(&t, NULL, disabled_waiter, NULL);
	wait_until(&waiting, 1);
	usleep(50000);
	pthread_cancel(t);
	usleep(200000);
	CHECK(waiting == 1, "cancel disabled: the wait ended (%d)", waiting);

	pthread_mutex_lock(&mutex);
	predicate = 1;
	pthread_cond_broadcast(&cond);
	pthread_mutex_unlock(&mutex);
	pthread_join(t, &result);
	CHECK(waiting == 2 && result == PTHREAD_CANCELED, "cancel disabled: state %d, result %p", waiting, result);
	printf("ok: cancel while disabled is acted on after the wait\n");
}

// ---- signals ----

static pthread_mutex_t handler_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t handler_cond = PTHREAD_COND_INITIALIZER;
static volatile int handler_runs;

static void handler(int sig)
{
	(void) sig;
	// not async-signal-safe by the letter, but common, and it works on macOS:
	// use other process-private objects inside the handler
	pthread_mutex_lock(&handler_mutex);
	handler_runs++;
	pthread_cond_broadcast(&handler_cond);
	pthread_mutex_unlock(&handler_mutex);
}

static volatile int sig_cond_returns;
static volatile int sig_cond_done;
static volatile int sig_mutex_done;
static volatile int sig_mutex_error;

static void* sig_cond_waiter(void* arg)
{
	(void) arg;
	pthread_mutex_lock(&mutex);
	waiting = 1;
	while (!predicate)
	{
		int ret = pthread_cond_wait(&cond, &mutex);
		if (ret != 0)
			sig_mutex_error = ret;
		sig_cond_returns++;
	}
	pthread_mutex_unlock(&mutex);
	sig_cond_done = 1;
	return NULL;
}

static pthread_mutex_t held = PTHREAD_MUTEX_INITIALIZER;

static void* sig_mutex_waiter(void* arg)
{
	(void) arg;
	waiting = 1;
	int ret = pthread_mutex_lock(&held);
	if (ret != 0)
		sig_mutex_error = ret;
	sig_mutex_done = 1;
	pthread_mutex_unlock(&held);
	return NULL;
}

static void test_signals(int restart)
{
	struct sigaction sa;
	pthread_t t;
	const char* name = restart ? "SA_RESTART" : "no SA_RESTART";

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = handler;
	sa.sa_flags = restart ? SA_RESTART : 0;
	sigaction(SIGUSR1, &sa, NULL);

	// condition wait
	predicate = 0;
	waiting = 0;
	sig_cond_done = 0;
	sig_cond_returns = 0;
	sig_mutex_error = 0;
	handler_runs = 0;
	pthread_create(&t, NULL, sig_cond_waiter, NULL);
	wait_until(&waiting, 1);
	usleep(50000);
	for (int i = 0; i < 20; i++)
	{
		pthread_kill(t, SIGUSR1);
		usleep(5000);
	}
	wait_until(&handler_runs, 20);
	CHECK(handler_runs == 20, "%s: cond: the handler ran %d times", name, handler_runs);
	CHECK(!sig_cond_done && sig_mutex_error == 0, "%s: cond: the wait ended early or failed (%d)", name, sig_mutex_error);
	pthread_mutex_lock(&mutex);
	predicate = 1;
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);
	pthread_join(t, NULL);
	CHECK(sig_cond_done, "%s: cond: the signalled wait did not end", name);
	printf("ok: %s: cond wait interrupted 20 times, %d returns (spurious ones allowed)\n", name, sig_cond_returns);

	// mutex wait
	waiting = 0;
	sig_mutex_done = 0;
	handler_runs = 0;
	pthread_mutex_lock(&held);
	pthread_create(&t, NULL, sig_mutex_waiter, NULL);
	wait_until(&waiting, 1);
	usleep(50000);
	for (int i = 0; i < 20; i++)
	{
		pthread_kill(t, SIGUSR1);
		usleep(5000);
	}
	wait_until(&handler_runs, 20);
	CHECK(!sig_mutex_done, "%s: mutex: the lock returned while held", name);
	pthread_mutex_unlock(&held);
	pthread_join(t, NULL);
	CHECK(sig_mutex_done && sig_mutex_error == 0, "%s: mutex: done %d, error %d", name, sig_mutex_done, sig_mutex_error);
	printf("ok: %s: mutex wait interrupted 20 times, then acquired\n", name);
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	alarm(120);

	test_cancel(0);
	test_cancel(1);
	test_cancel_disabled();
	test_signals(0);
	test_signals(1);

	if (failures)
		return 1;
	printf("PASS\n");
	return 0;
}
