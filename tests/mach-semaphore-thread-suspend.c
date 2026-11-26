// Copyright (C) 2026 Lubos Dolezel
//
// Suspending and resuming a thread that waits on a Mach semaphore doesn't end
// the wait, as observed on macOS 26.7.3 x86_64:
// - semaphore_timedwait() and semaphore_wait() of a thread that another thread
//   suspends (thread_suspend, thread_get_state) and resumes go on waiting and
//   return KERN_SUCCESS once the semaphore is signaled;
// - a timed wait that is suspended and resumed but never signaled times out
//   at its original deadline;
// - a signal delivered to the waiting thread (with a handler installed) still
//   ends the wait with KERN_ABORTED.
// JavaScriptCore suspends threads this way to scan their stacks, so a
// suspension must not look like a failed wait to the suspended thread.

#include <mach/mach.h>
#include <mach/semaphore.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

static int failures;
#define CHECK(c, ...) do { if (!(c)) { printf("FAIL line %d: %s: ", __LINE__, #c); printf(__VA_ARGS__); printf("\n"); failures++; } } while (0)

static double now(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + tv.tv_usec / 1e6;
}

enum wait_kind { TIMED, UNTIMED };

struct waiter {
	semaphore_t semaphore;
	enum wait_kind kind;
	mach_timespec_t timeout;
	pthread_t pthread;
	volatile mach_port_t thread;
	kern_return_t result;
	double elapsed;
};

static void *wait_thread(void *arg)
{
	struct waiter *w = arg;
	w->thread = mach_thread_self();
	double start = now();
	w->result = w->kind == TIMED ? semaphore_timedwait(w->semaphore, w->timeout) : semaphore_wait(w->semaphore);
	w->elapsed = now() - start;
	return NULL;
}

static void start_waiter(struct waiter *w, enum wait_kind kind, unsigned sec, int nsec)
{
	memset(w, 0, sizeof(*w));
	semaphore_create(mach_task_self(), &w->semaphore, SYNC_POLICY_FIFO, 0);
	w->kind = kind;
	w->timeout = (mach_timespec_t){ sec, nsec };
	pthread_create(&w->pthread, NULL, wait_thread, w);
	while (!w->thread)
		usleep(1000);
	usleep(200000); // let it block in the wait
}

static void suspend_and_resume(struct waiter *w)
{
	CHECK(thread_suspend(w->thread) == KERN_SUCCESS, "thread_suspend");
	x86_thread_state64_t state;
	mach_msg_type_number_t count = x86_THREAD_STATE64_COUNT;
	CHECK(thread_get_state(w->thread, x86_THREAD_STATE64, (thread_state_t) &state, &count) == KERN_SUCCESS, "thread_get_state");
	usleep(100000);
	CHECK(thread_resume(w->thread) == KERN_SUCCESS, "thread_resume");
}

static void finish_waiter(struct waiter *w)
{
	pthread_join(w->pthread, NULL);
	mach_port_deallocate(mach_task_self(), w->thread);
	semaphore_destroy(mach_task_self(), w->semaphore);
}

static void alarm_handler(int sig)
{
}

int main(void)
{
	struct waiter w;

	// a timed wait goes on after a suspension and succeeds when signaled
	start_waiter(&w, TIMED, 3, 0);
	suspend_and_resume(&w);
	usleep(500000);
	semaphore_signal(w.semaphore);
	finish_waiter(&w);
	CHECK(w.result == KERN_SUCCESS, "timed wait returned %d", w.result);
	CHECK(w.elapsed > 0.7 && w.elapsed < 2.5, "timed wait ended after %.2fs", w.elapsed);

	// so does an untimed wait
	start_waiter(&w, UNTIMED, 0, 0);
	suspend_and_resume(&w);
	usleep(500000);
	semaphore_signal(w.semaphore);
	finish_waiter(&w);
	CHECK(w.result == KERN_SUCCESS, "wait returned %d", w.result);
	CHECK(w.elapsed > 0.7, "wait ended after %.2fs", w.elapsed);

	// a suspension doesn't move the deadline of a timed wait
	start_waiter(&w, TIMED, 1, 0);
	suspend_and_resume(&w);
	finish_waiter(&w);
	CHECK(w.result == KERN_OPERATION_TIMED_OUT, "unsignaled timed wait returned %d", w.result);
	CHECK(w.elapsed > 0.9 && w.elapsed < 1.6, "unsignaled timed wait ended after %.2fs", w.elapsed);

	// a signal with a handler still ends the wait
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = alarm_handler;
	sigaction(SIGUSR1, &sa, NULL);
	start_waiter(&w, TIMED, 3, 0);
	pthread_kill(w.pthread, SIGUSR1);
	finish_waiter(&w);
	CHECK(w.result == KERN_ABORTED, "signaled-by-pthread_kill wait returned %d", w.result);
	CHECK(w.elapsed < 1.0, "signaled-by-pthread_kill wait ended after %.2fs", w.elapsed);

	printf(failures ? "FAILED\n" : "PASSED\n");
	return failures != 0;
}
