// Host-side test of mldr's commpage_setup(): the commpage is mapped at its
// fixed address without replacing anything already there (setup exits with
// status 1 instead), it is filled in, and it is read-only afterwards.
// Built as a 64-bit program, and as a 32-bit one where mldr32 is built.
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include "commpage.h"

#define PRIVATE
#if defined(__aarch64__)
#include <arm/cpu_capabilities.h>
#else
#include <i386/cpu_capabilities.h>
#endif

#define SKIP_STATUS 77

static int failures;

static void check(int ok, const char* what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

static const bool is64 = sizeof(void*) == 8;

static uintptr_t area_base(void)
{
	return (uintptr_t) commpage_address(is64);
}

static size_t area_length(void)
{
	return (size_t) commpage_length(is64);
}

static sigjmp_buf jb;

static void on_segv(int sig)
{
	(void) sig;
	siglongjmp(jb, 1);
}

// The permissions ("r--p") of the mapping that contains `addr`; "" if none does.
static void perms_at(uintptr_t addr, char* out)
{
	FILE* f = fopen("/proc/self/maps", "r");
	char line[512];

	out[0] = '\0';
	if (f == NULL)
		return;
	while (fgets(line, sizeof(line), f)) {
		unsigned long lo, hi;
		char p[8];
		if (sscanf(line, "%lx-%lx %7s", &lo, &hi, p) == 3 && addr >= lo && addr < hi) {
			strcpy(out, p);
			break;
		}
	}
	fclose(f);
}

static bool write_faults(volatile uint8_t* p)
{
	struct sigaction sa, old;
	bool faulted = false;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = on_segv;
	sigaction(SIGSEGV, &sa, &old);
	if (sigsetjmp(jb, 1) == 0)
		*p = *p ^ 0xff;
	else
		faulted = true;
	sigaction(SIGSEGV, &old, NULL);
	return faulted;
}

// Runs fn in a child; returns its exit status, or -1 if it was killed.
static int in_child(int (*fn)(uintptr_t), uintptr_t arg)
{
	fflush(stdout);
	pid_t pid = fork();
	if (pid == 0) {
		freopen("/dev/null", "w", stderr);
		_exit(fn(arg));
	}
	int status;
	waitpid(pid, &status, 0);
	return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

// Something already occupies the page at `where` inside the commpage range.
// Exit status 1: setup refused (expected); 2: setup returned; 3: the
// scenario could not be set up.
static int occupied(uintptr_t where)
{
	uint8_t* mine = mmap((void*) where, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
	if (mine != (uint8_t*) where)
		return 3;
	memset(mine, 0x5a, 4096);
	commpage_setup(is64);
	return 2;
}

// Exit status 0: filled in and read-only.
static int protections(uintptr_t unused)
{
	(void) unused;
	char p[8];
	uintptr_t base = area_base();
	size_t length = area_length();

	commpage_setup(is64);

	if (strcmp((const char*) base, is64 ? "commpage 64-bit" : "commpage 32-bit") != 0)
		return 10;
	if (*(volatile uint16_t*) _COMM_PAGE_VERSION != _COMM_PAGE_THIS_VERSION)
		return 11;

	perms_at(base, p);
	if (strcmp(p, "r--p") != 0)
		return 12;
	perms_at(base + length - 1, p);
	if (strcmp(p, "r--p") != 0)
		return 13;
	if (!write_faults((volatile uint8_t*) base))
		return 14;
	return 0;
}

int main(void)
{
	uintptr_t base = area_base();
	size_t length = area_length();

	// A 32-bit process may find its own stack in the range; mldr32 then
	// executes itself again (ensure_top_range_free()). Nothing to test then.
	void* probe = mmap((void*) base, length, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
	if (probe != (void*) base) {
		printf("SKIP: the commpage range is not free in this process\n");
		return SKIP_STATUS;
	}
	munmap(probe, length);

	check(in_child(protections, 0) == 0, "free range: filled in, every page r--p, writes fault");
	check(in_child(occupied, base) == 1, "a mapping at the start of the range is not replaced");
	check(in_child(occupied, base + length - 4096) == 1, "a mapping at the end of the range is not replaced");

	return failures ? 1 : 0;
}
