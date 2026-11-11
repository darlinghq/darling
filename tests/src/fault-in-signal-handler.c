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

// Faults inside and after application signal handlers.
//
// 1. A fault inside another signal's handler is an ordinary fault: the
//    application's SIGSEGV handler takes it, and the faulting access
//    completes once that handler has made the page writable.
// 2. After a SIGSEGV handler leaves with siglongjmp(), later faults are
//    still delivered to the handler.

#define _DARWIN_C_SOURCE
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

static int failed;
#define CHECK(cond, ...) do { if (!(cond)) { printf("FAIL: " __VA_ARGS__); printf("\n"); failed = 1; } } while (0)

static volatile char* page;
static size_t page_size;
static volatile int* volatile bad_ptr = (int*)8;
static sigjmp_buf env;
static volatile int unprotect_count, jump_count;
static volatile int usr1_done;

static void segv_unprotect(int sig)
{
	(void)sig;
	unprotect_count++;
	mprotect((void*)page, page_size, PROT_READ | PROT_WRITE);
}

static void segv_jump(int sig)
{
	(void)sig;
	jump_count++;
	siglongjmp(env, 1);
}

static void usr1_faults(int sig)
{
	(void)sig;
	page[16] = 7; // faults: the page is read-only
	usr1_done = 1;
}

static void set_handler(int sig, void (*h)(int))
{
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = h;
	sigemptyset(&sa.sa_mask);
	sigaction(sig, &sa, NULL);
}

int main(void)
{
	page_size = (size_t)sysconf(_SC_PAGESIZE);
	page = mmap(NULL, page_size, PROT_READ, MAP_ANON | MAP_PRIVATE, -1, 0);
	if (page == MAP_FAILED) {
		perror("mmap");
		return 1;
	}

	// 1: a fault inside the SIGUSR1 handler goes to the SIGSEGV handler.
	set_handler(SIGSEGV, segv_unprotect);
	set_handler(SIGBUS, segv_unprotect);
	set_handler(SIGUSR1, usr1_faults);
	raise(SIGUSR1);
	CHECK(unprotect_count == 1, "fault in SIGUSR1 handler: SIGSEGV handler ran %d times", unprotect_count);
	CHECK(usr1_done == 1 && page[16] == 7, "SIGUSR1 handler did not complete its store");

	// 2: a handler that left with siglongjmp() does not stop later faults.
	set_handler(SIGSEGV, segv_jump);
	set_handler(SIGBUS, segv_jump);
	for (int i = 0; i < 3; i++) {
		if (sigsetjmp(env, 1) == 0)
			*bad_ptr = 1;
	}
	CHECK(jump_count == 3, "after siglongjmp: SIGSEGV handler ran %d times, expected 3", jump_count);

	if (!failed)
		printf("PASS\n");
	return failed;
}
