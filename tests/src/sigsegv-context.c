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

// A SIGSEGV handler sees the fault address in si_addr and in the
// mcontext's exception state, the interrupted mask in uc_sigmask, and a
// full-size mcontext. Returning from the handler restores the mask.

#define _XOPEN_SOURCE 700
#define _DARWIN_C_SOURCE
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ucontext.h>

static volatile char* page;
static size_t page_size;

static volatile int handled;
static void* seen_si_addr;
static uint64_t seen_es_addr;
static uint64_t seen_pc;
static int seen_usr1_in_uc_mask, seen_usr2_blocked;
static size_t seen_mcsize;

static void handler(int sig, siginfo_t* info, void* vctx)
{
	ucontext_t* uc = vctx;
	sigset_t cur;

	handled++;
	seen_si_addr = info->si_addr;
#if defined(__arm64__) || defined(__aarch64__)
	seen_es_addr = uc->uc_mcontext->__es.__far;
	seen_pc = uc->uc_mcontext->__ss.__pc;
#elif defined(__x86_64__)
	seen_es_addr = uc->uc_mcontext->__es.__faultvaddr;
	seen_pc = uc->uc_mcontext->__ss.__rip;
#else
	seen_es_addr = uc->uc_mcontext->__es.__faultvaddr;
	seen_pc = uc->uc_mcontext->__ss.__eip;
#endif
	seen_mcsize = uc->uc_mcsize;
	seen_usr1_in_uc_mask = sigismember(&uc->uc_sigmask, SIGUSR1);

	sigprocmask(SIG_BLOCK, NULL, &cur);
	seen_usr2_blocked = sigismember(&cur, SIGUSR2);

	// Let the faulting store succeed when the handler returns.
	mprotect((void*)page, page_size, PROT_READ | PROT_WRITE);
}

int main(void)
{
	int failed = 0;
	struct sigaction sa;
	sigset_t set, after;

	page_size = (size_t)sysconf(_SC_PAGESIZE);
	page = mmap(NULL, page_size, PROT_READ, MAP_ANON | MAP_PRIVATE, -1, 0);
	if (page == MAP_FAILED) {
		perror("mmap");
		return 1;
	}

	memset(&sa, 0, sizeof(sa));
	sa.sa_sigaction = handler;
	sa.sa_flags = SA_SIGINFO;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGUSR2);
	sigaction(SIGSEGV, &sa, NULL);
	sigaction(SIGBUS, &sa, NULL);

	// SIGUSR1 is blocked when the fault happens: uc_sigmask must show it.
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	sigprocmask(SIG_SETMASK, &set, NULL);

	volatile char* target = page + 64;
	*target = 42;

	sigprocmask(SIG_BLOCK, NULL, &after);

#define CHECK(cond, ...) do { if (!(cond)) { printf("FAIL: " __VA_ARGS__); printf("\n"); failed = 1; } } while (0)
	CHECK(handled == 1, "handler ran %d times", handled);
	CHECK(*target == 42, "store did not complete after the handler");
	CHECK(seen_si_addr == (void*)target, "si_addr %p, expected %p", seen_si_addr, (void*)target);
	CHECK(seen_es_addr == (uint64_t)(uintptr_t)target, "exception state fault address 0x%llx, expected %p",
		(unsigned long long)seen_es_addr, (void*)target);
	CHECK(seen_pc != 0, "mcontext pc is 0");
	// macOS may hand out a larger (AVX) mcontext; it is never smaller.
	CHECK(seen_mcsize >= sizeof(*((ucontext_t*)0)->uc_mcontext), "uc_mcsize %zu, expected at least %zu",
		seen_mcsize, sizeof(*((ucontext_t*)0)->uc_mcontext));
	CHECK(seen_usr1_in_uc_mask == 1, "uc_sigmask does not hold the interrupted mask (SIGUSR1)");
	CHECK(seen_usr2_blocked == 1, "sa_mask (SIGUSR2) not blocked in the handler");
	CHECK(sigismember(&after, SIGUSR1) == 1, "SIGUSR1 no longer blocked after the handler");
	CHECK(sigismember(&after, SIGUSR2) == 0, "SIGUSR2 still blocked after the handler");
	CHECK(sigismember(&after, SIGSEGV) == 0, "SIGSEGV still blocked after the handler");

	if (!failed)
		printf("PASS\n");
	return failed;
}
