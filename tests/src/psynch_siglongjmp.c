// Copyright (C) 2026 Lubos Dolezel
//
// A signal handler that leaves a blocked condition wait or
// pthread_mutex_lock() with siglongjmp(). In XNU the signal first interrupts
// the wait (the thread is taken off the wait queue and the call returns
// EINTR), and the handler runs on the way back to user space; so the waiter
// that jumped away is no longer queued, and later signals and unlocks reach
// the threads that are still waiting.
//
// Condition variable: thread A waits and jumps out of the wait; thread B
// waits; one pthread_cond_signal must wake B.
// Mutex: the main thread holds the mutex; A blocks on it and jumps out; B
// blocks on it; one unlock must let B in.
// Each repeated several times, on fresh objects.

#include <errno.h>
#include <pthread.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#define ROUNDS 20

static __thread sigjmp_buf* jump_target;

static void handler(int sig)
{
	(void) sig;
	if (jump_target != NULL)
	{
		sigjmp_buf* target = jump_target;
		jump_target = NULL;
		siglongjmp(*target, 1);
	}
}

static double now(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + tv.tv_usec / 1e6;
}

static void wait_flag(volatile int* flag)
{
	for (int i = 0; i < 5000 && !*flag; i++)
		usleep(1000);
}

// true if *flag becomes set within the time limit
static int wait_flag_for(volatile int* flag, double seconds)
{
	double deadline = now() + seconds;
	while (!*flag && now() < deadline)
		usleep(1000);
	return *flag;
}

// ---- condition variable ----

static pthread_mutex_t c_mutex;
static pthread_cond_t c_cond;
static int c_go;
static volatile int a_waiting, a_jumped, b_waiting, b_woken;

static void* cond_jumper(void* arg)
{
	sigjmp_buf jb;
	(void) arg;

	pthread_mutex_lock(&c_mutex);
	if (sigsetjmp(jb, 1) == 0)
	{
		jump_target = &jb;
		a_waiting = 1;
		// not pthread_cond_wait(): as a cancellation point it keeps a cleanup
		// record on this stack, which a jump out would leave behind (and thread
		// exit would run) - with libpthread on macOS too
		struct timespec ts = { 30, 0 };
		while (!c_go)
			pthread_cond_timedwait_relative_np(&c_cond, &c_mutex, &ts);
		jump_target = NULL;
		pthread_mutex_unlock(&c_mutex);
		return NULL;
	}
	// the jump left the wait: like on macOS, the mutex is not held here
	a_jumped = 1;
	return NULL;
}

static void* cond_waiter(void* arg)
{
	(void) arg;
	pthread_mutex_lock(&c_mutex);
	b_waiting = 1;
	while (!c_go)
		pthread_cond_wait(&c_cond, &c_mutex);
	b_woken = 1;
	pthread_mutex_unlock(&c_mutex);
	return NULL;
}

static int test_cond(int round)
{
	pthread_t a, b;
	int ok;

	pthread_mutex_init(&c_mutex, NULL);
	pthread_cond_init(&c_cond, NULL);
	c_go = 0;
	a_waiting = a_jumped = b_waiting = b_woken = 0;

	pthread_create(&a, NULL, cond_jumper, NULL);
	wait_flag(&a_waiting);
	usleep(20000);
	pthread_kill(a, SIGUSR1);
	pthread_join(a, NULL);

	pthread_create(&b, NULL, cond_waiter, NULL);
	wait_flag(&b_waiting);
	usleep(20000);

	pthread_mutex_lock(&c_mutex);
	c_go = 1;
	pthread_cond_signal(&c_cond);
	pthread_mutex_unlock(&c_mutex);

	ok = wait_flag_for(&b_woken, 2);
	if (!ok)
	{
		printf("round %d: cond: the signal did not reach the remaining waiter (jumped %d)\n", round, a_jumped);
		// let it go
		pthread_mutex_lock(&c_mutex);
		pthread_cond_broadcast(&c_cond);
		pthread_mutex_unlock(&c_mutex);
		if (!wait_flag_for(&b_woken, 2))
		{
			printf("FAIL: cond: the waiter is stuck\n");
			_exit(1);
		}
	}
	pthread_join(b, NULL);
	return ok && a_jumped;
}

// ---- mutex ----

static pthread_mutex_t m_mutex;
static volatile int ma_waiting, ma_jumped, mb_waiting, mb_locked;

static void* mutex_jumper(void* arg)
{
	sigjmp_buf jb;
	(void) arg;

	if (sigsetjmp(jb, 1) == 0)
	{
		jump_target = &jb;
		ma_waiting = 1;
		pthread_mutex_lock(&m_mutex);
		jump_target = NULL;
		pthread_mutex_unlock(&m_mutex);
		return NULL;
	}
	ma_jumped = 1;
	return NULL;
}

static void* mutex_waiter(void* arg)
{
	(void) arg;
	mb_waiting = 1;
	pthread_mutex_lock(&m_mutex);
	mb_locked = 1;
	pthread_mutex_unlock(&m_mutex);
	return NULL;
}

static int test_mutex(int round)
{
	pthread_t a, b;
	int ok;

	pthread_mutex_init(&m_mutex, NULL);
	ma_waiting = ma_jumped = mb_waiting = mb_locked = 0;

	pthread_mutex_lock(&m_mutex);
	pthread_create(&a, NULL, mutex_jumper, NULL);
	wait_flag(&ma_waiting);
	usleep(20000);
	pthread_kill(a, SIGUSR1);
	pthread_join(a, NULL);

	pthread_create(&b, NULL, mutex_waiter, NULL);
	wait_flag(&mb_waiting);
	usleep(20000);
	pthread_mutex_unlock(&m_mutex);

	ok = wait_flag_for(&mb_locked, 2);
	if (!ok)
	{
		printf("round %d: mutex: the unlock did not reach the remaining waiter (jumped %d)\n", round, ma_jumped);
		printf("FAIL: mutex: the waiter is stuck\n");
		_exit(1);
	}
	pthread_join(b, NULL);
	return ok && ma_jumped;
}

int main(void)
{
	struct sigaction sa;
	int cond_ok = 0, mutex_ok = 0;

	setvbuf(stdout, NULL, _IONBF, 0);
	alarm(120);

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = handler;
	sigaction(SIGUSR1, &sa, NULL);

	for (int i = 0; i < ROUNDS; i++)
		cond_ok += test_cond(i);
	printf("cond: %d of %d rounds: the waiter jumped out and the signal reached the other one\n", cond_ok, ROUNDS);

	for (int i = 0; i < ROUNDS; i++)
		mutex_ok += test_mutex(i);
	printf("mutex: %d of %d rounds: the waiter jumped out and the unlock reached the other one\n", mutex_ok, ROUNDS);

	if (cond_ok != ROUNDS || mutex_ok != ROUNDS)
	{
		printf("FAIL\n");
		return 1;
	}
	printf("PASS\n");
	return 0;
}
