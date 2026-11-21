/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

// thread_get_state() for the x86 debug-register flavors (x86_DEBUG_STATE32,
// x86_DEBUG_STATE64 and the unified x86_DEBUG_STATE) of a suspended thread of
// this task: the flavor matching the task's architecture succeeds with the
// documented count, the other width is refused with KERN_INVALID_ARGUMENT,
// and so is a buffer count that is too small.  Nothing arms a debug register
// here, so every register reads as zero.

#include <mach/mach.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#if !defined(__x86_64__) && !defined(__i386__)
int main(void) { printf("PASSED (not an x86 build)\n"); return 0; }
#else

static int failures;

#define CHECK(cond, ...) do { \
	if (cond) { printf("PASS: %s\n", #cond); } \
	else { failures++; printf("FAIL: %s  ", #cond); printf(__VA_ARGS__); printf("\n"); } \
	fflush(stdout); \
} while (0)

static void* spin(void* arg) {
	(void)arg;
	for (;;) usleep(1000);
	return NULL;
}

#ifdef __x86_64__
#define OWN_FLAVOR x86_DEBUG_STATE64
#define OWN_COUNT x86_DEBUG_STATE64_COUNT
#define OTHER_FLAVOR x86_DEBUG_STATE32
#define OTHER_COUNT x86_DEBUG_STATE32_COUNT
#else
#define OWN_FLAVOR x86_DEBUG_STATE32
#define OWN_COUNT x86_DEBUG_STATE32_COUNT
#define OTHER_FLAVOR x86_DEBUG_STATE64
#define OTHER_COUNT x86_DEBUG_STATE64_COUNT
#endif

int main(void) {
	pthread_t t;
	pthread_create(&t, NULL, spin, NULL);
	usleep(50000);
	mach_port_t port = pthread_mach_thread_np(t);
	kern_return_t kr = thread_suspend(port);
	CHECK(kr == KERN_SUCCESS, "suspend kr=%d", kr);

	unsigned int buf[64];
	mach_msg_type_number_t count;

	memset(buf, 0xaa, sizeof(buf));
	count = OWN_COUNT;
	kr = thread_get_state(port, OWN_FLAVOR, (thread_state_t)buf, &count);
	CHECK(kr == KERN_SUCCESS, "own-width debug state kr=%d", kr);
	CHECK(count == OWN_COUNT, "count=%u expected %u", count, (unsigned)OWN_COUNT);
	int zero = 1;
	for (unsigned i = 0; i < OWN_COUNT; i++) if (buf[i]) zero = 0;
	CHECK(zero, "all debug registers read as zero");

	count = OWN_COUNT - 1;
	kr = thread_get_state(port, OWN_FLAVOR, (thread_state_t)buf, &count);
	CHECK(kr == KERN_INVALID_ARGUMENT, "short buffer kr=%d", kr);

	count = OTHER_COUNT;
	kr = thread_get_state(port, OTHER_FLAVOR, (thread_state_t)buf, &count);
	CHECK(kr == KERN_INVALID_ARGUMENT, "other-width debug state kr=%d", kr);

	memset(buf, 0xaa, sizeof(buf));
	count = x86_DEBUG_STATE_COUNT;
	kr = thread_get_state(port, x86_DEBUG_STATE, (thread_state_t)buf, &count);
	CHECK(kr == KERN_SUCCESS, "unified debug state kr=%d", kr);
	CHECK(count == x86_DEBUG_STATE_COUNT, "unified count=%u expected %u", count, (unsigned)x86_DEBUG_STATE_COUNT);

	thread_resume(port);
	printf("%s\n", failures ? "FAILED" : "PASSED");
	return failures ? 1 : 0;
}
#endif
