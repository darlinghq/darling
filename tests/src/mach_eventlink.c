/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Mach eventlinks (<mach/mach_eventlink.h>): a pair of ports, each associated with a thread, that signal and
 * wait on each other. mach_eventlink_create() makes the pair, mach_eventlink_signal() and
 * mach_eventlink_wait_until() are the BSD syscalls 496 to 498 underneath.
 */
#include <dlfcn.h>
#include <errno.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#define MELC_OPTION_NO_COPYIN 1
#define MELA_OPTION_NONE 0
#define MELSW_OPTION_NONE 0
#define EL_CLOCK_MACH_ABSOLUTE_TIME 1

typedef kern_return_t (*create_f)(task_t, uint32_t, mach_port_t[2]);
typedef kern_return_t (*associate_f)(mach_port_t, thread_t, mach_vm_address_t, uint64_t, mach_vm_address_t, uint64_t, uint32_t);
typedef kern_return_t (*destroy_f)(mach_port_t);
typedef kern_return_t (*signal_f)(mach_port_t, uint64_t);
typedef kern_return_t (*wait_f)(mach_port_t, uint64_t*, uint32_t, uint32_t, uint64_t);
typedef kern_return_t (*signal_wait_f)(mach_port_t, uint64_t*, uint64_t, uint32_t, uint32_t, uint64_t);

static create_f el_create;
static associate_f el_associate;
static destroy_f el_destroy;
static signal_f el_signal;
static wait_f el_wait;
static signal_wait_f el_signal_wait;
static mach_port_t pair[2];
static int failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)

static uint64_t deadline_ms(unsigned ms)
{
	mach_timebase_info_data_t tb;
	mach_timebase_info(&tb);
	return mach_absolute_time() + (uint64_t) ms * 1000000ULL * tb.denom / tb.numer;
}

static void* peer(void* arg)
{
	uint64_t count = 0;
	(void) arg;
	CHECK(el_associate(pair[1], mach_thread_self(), 0, 0, 0, 0, MELA_OPTION_NONE) == KERN_SUCCESS);
	/* wait for the first signal, answer it, wait for the next */
	CHECK(el_wait(pair[1], &count, MELSW_OPTION_NONE, EL_CLOCK_MACH_ABSOLUTE_TIME, deadline_ms(10000)) == KERN_SUCCESS);
	CHECK(count == 1);
	CHECK(el_signal_wait(pair[1], &count, 1, MELSW_OPTION_NONE, EL_CLOCK_MACH_ABSOLUTE_TIME, deadline_ms(10000)) == KERN_SUCCESS);
	CHECK(count == 2);
	return NULL;
}

int main(void)
{
	void* h = RTLD_DEFAULT;
	pthread_t thread;
	uint64_t count = 0;

	el_create = dlsym(h, "mach_eventlink_create");
	el_associate = dlsym(h, "mach_eventlink_associate");
	el_destroy = dlsym(h, "mach_eventlink_destroy");
	el_signal = dlsym(h, "mach_eventlink_signal");
	el_wait = dlsym(h, "mach_eventlink_wait_until");
	el_signal_wait = dlsym(h, "mach_eventlink_signal_wait_until");
	if (!el_create || !el_associate || !el_destroy || !el_signal || !el_wait || !el_signal_wait) {
		puts("FAIL: the mach_eventlink calls are not exported");
		return 1;
	}

	kern_return_t kr = el_create(mach_task_self(), MELC_OPTION_NO_COPYIN, pair);
	if (kr != KERN_SUCCESS) {
		printf("FAIL: mach_eventlink_create returned %d\n", kr);
		return 1;
	}
	CHECK(el_associate(pair[0], mach_thread_self(), 0, 0, 0, 0, MELA_OPTION_NONE) == KERN_SUCCESS);

	/* nothing signalled: the wait times out */
	CHECK(el_wait(pair[0], &count, MELSW_OPTION_NONE, EL_CLOCK_MACH_ABSOLUTE_TIME, deadline_ms(50)) == KERN_OPERATION_TIMED_OUT);

	CHECK(pthread_create(&thread, NULL, peer, NULL) == 0);
	usleep(100000);
	CHECK(el_signal(pair[0], 1) == KERN_SUCCESS);
	/* answer: signal the peer again and wait for it to signal back */
	count = 0;
	CHECK(el_signal_wait(pair[0], &count, 1, MELSW_OPTION_NONE, EL_CLOCK_MACH_ABSOLUTE_TIME, deadline_ms(10000)) == KERN_SUCCESS);
	CHECK(count == 1);
	CHECK(pthread_join(thread, NULL) == 0);

	CHECK(el_destroy(pair[0]) == KERN_SUCCESS);
	CHECK(el_destroy(pair[1]) == KERN_SUCCESS);
	if (!failures)
		puts("PASS");
	return failures != 0;
}
