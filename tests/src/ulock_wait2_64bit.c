// Copyright (C) 2026 Lubos Dolezel
//
// __ulock_wait2 / __ulock_wake: the 64-bit compare-and-wait operations compare
// the whole 64-bit word, and the *_SHARED operations work across processes
// (shared memory after fork). Contract: open-source XNU bsd/sys/ulock.h.
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <pthread.h>

#define UL_COMPARE_AND_WAIT          1
#define UL_COMPARE_AND_WAIT_SHARED   3
#define UL_COMPARE_AND_WAIT64        5
#define UL_COMPARE_AND_WAIT64_SHARED 6
#define ULF_WAKE_ALL                 0x00000100
#define ULF_NO_ERRNO                 0x01000000

extern int __ulock_wait2(uint32_t operation, void *addr, uint64_t value, uint64_t timeout, uint64_t value2);
extern int __ulock_wake(uint32_t operation, void *addr, uint64_t wake_value);

static int g_failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static double now(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec + ts.tv_nsec / 1e9;
}

// Returns the ULF_NO_ERRNO result: >= 0 on success, -errno on failure.
static int wait64(uint32_t op, volatile uint64_t *addr, uint64_t expected, uint64_t timeout_ns)
{
	return __ulock_wait2(op | ULF_NO_ERRNO, (void *) addr, expected, timeout_ns, 8);
}

static volatile uint64_t g_word;
static volatile int g_thread_ret = 12345;

static void *waiter(void *arg)
{
	(void) arg;
	g_thread_ret = wait64(UL_COMPARE_AND_WAIT64, &g_word, g_word, 5ULL * 1000000000ULL);
	return NULL;
}

static volatile uint64_t g_race_expected;

static void *race_waiter(void *arg)
{
	(void) arg;
	g_thread_ret = wait64(UL_COMPARE_AND_WAIT64, &g_word, g_race_expected, 1000ULL * 1000000ULL);
	return NULL;
}

int main(void)
{
	// 1. A matching 64-bit value with a short timeout times out.
	g_word = 0x0000000200000005ULL;
	double t0 = now();
	int r = wait64(UL_COMPARE_AND_WAIT64, &g_word, g_word, 100ULL * 1000000);
	double dt = now() - t0;
	printf("matching wait: ret=%d dt=%.3f\n", r, dt);
	CHECK(r == -ETIMEDOUT);
	CHECK(dt >= 0.08 && dt < 1.0);

	// 2. Low halves equal, high halves differ: the compare fails and the call
	// returns at once instead of sleeping until the timeout.
	t0 = now();
	r = wait64(UL_COMPARE_AND_WAIT64, &g_word, 0x0000000300000005ULL, 2ULL * 1000000000ULL);
	dt = now() - t0;
	printf("high-half mismatch wait: ret=%d dt=%.3f\n", r, dt);
	CHECK(dt < 0.5);
	CHECK(r != -ETIMEDOUT);

	// 3. Completely different value: also returns at once.
	t0 = now();
	r = wait64(UL_COMPARE_AND_WAIT64, &g_word, 0x1111111122222222ULL, 2ULL * 1000000000ULL);
	dt = now() - t0;
	printf("full mismatch wait: ret=%d dt=%.3f\n", r, dt);
	CHECK(dt < 0.5);

	// 4. A 64-bit waiter is woken by __ulock_wake.
	pthread_t th;
	g_word = 0x0000000700000001ULL;
	pthread_create(&th, NULL, waiter, NULL);
	usleep(200000);
	t0 = now();
	r = __ulock_wake(UL_COMPARE_AND_WAIT64 | ULF_NO_ERRNO, (void *) &g_word, 0);
	printf("wake64: ret=%d\n", r);
	CHECK(r == 0);
	pthread_join(th, NULL);
	dt = now() - t0;
	printf("woken thread ret=%d dt=%.3f\n", g_thread_ret, dt);
	CHECK(dt < 1.0);
	CHECK(g_thread_ret >= 0);

	// 5. Waking with no waiters reports ENOENT.
	r = __ulock_wake(UL_COMPARE_AND_WAIT64 | ULF_NO_ERRNO, (void *) &g_word, 0);
	printf("wake without waiters: ret=%d\n", r);
	CHECK(r == -ENOENT);

	// 6. Shared operations across fork() on a MAP_SHARED word, 32-bit and 64-bit.
	for (int is64 = 0; is64 < 2; is64++) {
		volatile uint64_t *w = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
		CHECK(w != MAP_FAILED);
		*w = 0x0000000100000009ULL;
		uint32_t waitop = is64 ? UL_COMPARE_AND_WAIT64_SHARED : UL_COMPARE_AND_WAIT_SHARED;
		pid_t pid = fork();
		if (pid == 0) {
			double s = now();
			int rr = is64 ? wait64(waitop, w, *w, 5ULL * 1000000000ULL)
			              : __ulock_wait2(waitop | ULF_NO_ERRNO, (void *) w, (uint32_t) *w, 5ULL * 1000000000ULL, 0);
			double d = now() - s;
			_exit((rr >= 0 && d < 3.0) ? 0 : 1);
		}
		usleep(300000);
		r = __ulock_wake(waitop | ULF_NO_ERRNO, (void *) w, 0);
		printf("shared%s wake: ret=%d\n", is64 ? "64" : "", r);
		CHECK(r == 0);
		int status = 0;
		waitpid(pid, &status, 0);
		printf("shared%s child status=%d\n", is64 ? "64" : "", status);
		CHECK(WIFEXITED(status) && WEXITSTATUS(status) == 0);
	}

	// 7. Waking with no waiters reports ENOENT for every wait operation that waits on an address.
	{
		uint64_t idle = 0;
		static const uint32_t ops[] = { 1, 2, 3, UL_COMPARE_AND_WAIT64, UL_COMPARE_AND_WAIT64_SHARED };
		for (unsigned i = 0; i < sizeof(ops) / sizeof(ops[0]); i++) {
			r = __ulock_wake(ops[i] | ULF_NO_ERRNO, &idle, 0);
			CHECK(r == -ENOENT);
		}
		r = __ulock_wake(UL_COMPARE_AND_WAIT | ULF_WAKE_ALL | ULF_NO_ERRNO, &idle, 0);
		CHECK(r == -ENOENT);
	}

	// 8. The comparison and the decision to sleep are one step: a thread that changes only the high
	// half of the word and then wakes must never leave a 64-bit waiter asleep until its timeout.
	{
		int lost = 0;
		for (int i = 0; i < 1000 && !lost; i++) {
			g_word = ((uint64_t) i << 32) | 0x42;
			g_race_expected = g_word;
			g_thread_ret = 12345;
			pthread_create(&th, NULL, race_waiter, NULL);
			if (i & 1)
				usleep(i % 7);
			g_word = ((uint64_t) (i + 1) << 32) | 0x42;
			__ulock_wake(UL_COMPARE_AND_WAIT64 | ULF_NO_ERRNO | ULF_WAKE_ALL, (void *) &g_word, 0);
			pthread_join(th, NULL);
			if (g_thread_ret == -ETIMEDOUT)
				lost = 1;
		}
		CHECK(!lost);
	}

	printf(g_failures ? "FAILED (%d)\n" : "PASSED\n", g_failures);
	return g_failures != 0;
}
