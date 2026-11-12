/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * nanosleep(2)/usleep(3): duration, argument errors, the remaining time on EINTR
 * (also with SA_RESTART), signals that are ignored do not end the sleep, and
 * pthread_cancel() ends a sleeping thread (but not one with cancellation
 * disabled). With the argument "bench" it also prints the per-call cost of
 * 10000 usleep(0) / nanosleep(1us) calls.
 */
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

extern int __semwait_signal(int cond_sem, int mutex_sem, int timeout, int relative, int64_t tv_sec, int32_t tv_nsec);

static int failed;
#define CHECK(c) do { if (!(c)) { printf("FAIL: %s (line %d)\n", #c, __LINE__); failed = 1; } } while (0)

/* integer arithmetic only: floating point math is unreliable in 32-bit Darling programs */
static long long now_us(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000000LL + ts.tv_nsec / 1000;
}

static long long now_ms(void)
{
	return now_us() / 1000;
}

static volatile int handled;
static void on_alarm(int s) { (void) s; handled++; }

static void arm(int usec)
{
	struct itimerval it = { { 0, 0 }, { 0, usec } };
	setitimer(ITIMER_REAL, &it, NULL);
}

static void set_handler(void (*h)(int), int flags)
{
	struct sigaction sa;
	memset(&sa, 0, sizeof sa);
	sa.sa_handler = h;
	sa.sa_flags = flags;
	sigaction(SIGALRM, &sa, NULL);
}

static void test_basic(void)
{
	struct timespec ts = { 0, 30 * 1000000 };
	long long t = now_ms();
	CHECK(nanosleep(&ts, NULL) == 0);
	long long e = now_ms() - t;
	CHECK(e >= 29 && e < 500);

	t = now_ms();
	CHECK(usleep(20000) == 0);
	e = now_ms() - t;
	CHECK(e >= 19 && e < 500);

	/* a zero interval returns at once, with or without rmtp; rmtp is left alone */
	struct timespec zero = { 0, 0 }, rem = { 77, 77 };
	t = now_ms();
	CHECK(nanosleep(&zero, &rem) == 0);
	CHECK(usleep(0) == 0);
	CHECK(now_ms() - t < 100);
	CHECK(rem.tv_sec == 77 && rem.tv_nsec == 77);

	/* a completed sleep leaves rmtp alone */
	rem.tv_sec = 77; rem.tv_nsec = 77;
	ts.tv_nsec = 5 * 1000000;
	CHECK(nanosleep(&ts, &rem) == 0);
	CHECK(rem.tv_sec == 77 && rem.tv_nsec == 77);
}

static void test_errors(void)
{
	struct timespec ts;
	int r;

	ts.tv_sec = 0; ts.tv_nsec = 1000000000;
	errno = 0; r = nanosleep(&ts, NULL);
	CHECK(r == -1 && errno == EINVAL);
	ts.tv_sec = -1; ts.tv_nsec = 0;
	errno = 0; r = nanosleep(&ts, NULL);
	CHECK(r == -1 && errno == EINVAL);
	ts.tv_sec = 0; ts.tv_nsec = -1;
	errno = 0; r = nanosleep(&ts, NULL);
	CHECK(r == -1 && errno == EINVAL);
	errno = 0; r = nanosleep(NULL, NULL);
	CHECK(r == -1 && errno == EINVAL);
}

static void test_eintr(int sa_flags)
{
	struct timespec ts = { 1, 0 }, rem;
	long long t;
	int r;

	set_handler(on_alarm, sa_flags);

	handled = 0; arm(100000);
	rem.tv_sec = 77; rem.tv_nsec = 77;
	t = now_ms(); errno = 0; r = nanosleep(&ts, &rem);
	long long e = now_ms() - t;
	CHECK(r == -1 && errno == EINTR);
	CHECK(handled == 1);
	CHECK(e >= 95 && e < 600);
	CHECK(rem.tv_sec == 0 && rem.tv_nsec > 400000000 && rem.tv_nsec < 920000000);

	handled = 0; arm(100000);
	t = now_ms(); errno = 0; r = nanosleep(&ts, NULL);
	CHECK(r == -1 && errno == EINTR && handled == 1);
	CHECK(now_ms() - t < 600);

	handled = 0; arm(100000);
	t = now_ms(); errno = 0; r = usleep(1000000);
	CHECK(r == -1 && errno == EINTR && handled == 1);
	CHECK(now_ms() - t < 600);

	/* a signal that arrives before the sleep starts is handled and the sleep still runs in full */
	handled = 0; arm(1000);
	usleep(20000);
	CHECK(handled == 1);
}

static void test_ignored_signal(void)
{
	/* a signal with SIG_IGN does not interrupt the sleep, as on macOS */
	struct timespec ts = { 0, 300 * 1000000 };
	signal(SIGALRM, SIG_IGN);
	arm(50000);
	long long t = now_ms();
	int r = nanosleep(&ts, NULL);
	long long e = now_ms() - t;
	CHECK(r == 0);
	CHECK(e >= 295 && e < 800);

	/* nor does SIGURG, whose default action is to ignore it */
	signal(SIGURG, SIG_DFL);
	t = now_ms();
	r = usleep(1000);
	CHECK(r == 0);
}

static void *sleeper(void *arg)
{
	struct timespec ts = { 10, 0 };
	int disable = (int) (intptr_t) arg;
	int old;
	if (disable)
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old);
	nanosleep(&ts, NULL);
	return (void *) 1;
}

static void *usleeper(void *arg)
{
	(void) arg;
	usleep(10000000);
	return (void *) 1;
}

static void test_cancel(void)
{
	pthread_t th;
	void *res = NULL;
	long long t;

	pthread_create(&th, NULL, sleeper, (void *) 0);
	usleep(100000);
	t = now_ms();
	pthread_cancel(th);
	pthread_join(th, &res);
	CHECK(res == PTHREAD_CANCELED);
	CHECK(now_ms() - t < 2000);

	pthread_create(&th, NULL, usleeper, NULL);
	usleep(100000);
	t = now_ms();
	pthread_cancel(th);
	pthread_join(th, &res);
	CHECK(res == PTHREAD_CANCELED);
	CHECK(now_ms() - t < 2000);

	/* cancelled before it sleeps */
	pthread_create(&th, NULL, usleeper, NULL);
	pthread_cancel(th);
	pthread_join(th, &res);
	CHECK(res == PTHREAD_CANCELED);

	/* with cancellation disabled the sleep is not cut short; stop it with a signal handler instead */
	set_handler(on_alarm, 0);
	pthread_create(&th, NULL, sleeper, (void *) 1);
	usleep(100000);
	t = now_ms();
	pthread_cancel(th);
	usleep(300000);                 /* give a wrong implementation the chance to end the sleep early */
	pthread_kill(th, SIGALRM);      /* handler runs in the sleeper and ends its sleep */
	pthread_join(th, &res);
	CHECK(res == (void *) 1);
	CHECK(now_ms() - t >= 290);
}

/* __semwait_signal() with neither semaphore is not a sleep: EINVAL, as on macOS */
static void test_semwait_no_semaphore(void)
{
	errno = 0;
	long long t = now_ms();
	int r = __semwait_signal(0, 0, 1, 1, (int64_t) 0, (int32_t) 20000000);
	CHECK(r == -1 && errno == EINVAL);
	CHECK(now_ms() - t < 15);
}

static void bench(void)
{
	struct timespec ts = { 0, 1000 }, rem;
	long long t = now_us(), d;
	for (int i = 0; i < 10000; i++) usleep(0);
	d = now_us() - t;
	printf("usleep(0)      x10000: %lld.%02lld us/call\n", d / 10000, d % 10000 / 100);
	t = now_us();
	for (int i = 0; i < 10000; i++) nanosleep(&ts, NULL);
	d = now_us() - t;
	printf("nanosleep(1us) x10000: %lld.%02lld us/call\n", d / 10000, d % 10000 / 100);
	t = now_us();
	for (int i = 0; i < 10000; i++) nanosleep(&ts, &rem);
	d = now_us() - t;
	printf("nanosleep(1us, rmtp) x10000: %lld.%02lld us/call\n", d / 10000, d % 10000 / 100);
}

int main(int argc, char **argv)
{
	test_basic();
	test_errors();
	test_eintr(0);
	test_eintr(SA_RESTART);
	test_ignored_signal();
	test_cancel();
	test_semwait_no_semaphore();
	if (argc > 1 && strcmp(argv[1], "bench") == 0)
		bench();
	puts(failed ? "FAILED" : "PASSED");
	return failed;
}
