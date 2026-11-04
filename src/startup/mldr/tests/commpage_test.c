#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include "commpage.h"

#define PRIVATE
#if defined(__aarch64__)
#include <arm/cpu_capabilities.h>
#else
#include <i386/cpu_capabilities.h>
#endif

static int failures;
#define EXPECT(cond, msg) do { if (!(cond)) { printf("FAIL: %s\n", msg); ++failures; } } while (0)

static const uintptr_t base = (uintptr_t) _COMM_PAGE64_BASE_ADDRESS;
static const size_t length = _COMM_PAGE64_AREA_LENGTH;

static sigjmp_buf jb;
static void on_segv(int sig) { (void) sig; siglongjmp(jb, 1); }

// Returns the permissions string ("r-xp") of the mapping containing `addr`, or "" if unmapped.
static void perms_at(uintptr_t addr, char* out) {
	FILE* f = fopen("/proc/self/maps", "r");
	char line[512];
	out[0] = '\0';
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

static bool write_faults(volatile uint8_t* p) {
	struct sigaction sa = { .sa_handler = on_segv }, old;
	sigaction(SIGSEGV, &sa, &old);
	bool faulted = false;
	if (sigsetjmp(jb, 1) == 0) {
		*p = *p ^ 0xff;
		*p = *p ^ 0xff;
	} else {
		faulted = true;
	}
	sigaction(SIGSEGV, &old, NULL);
	return faulted;
}

// Runs fn in a child; returns its exit status (-1 if it was killed).
static int in_child(int (*fn)(uintptr_t), uintptr_t arg) {
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

// Something else already holds `where` inside the commpage range: setup must exit 1 instead of replacing it.
// Exit codes: 1 = refused (expected), 2 = setup returned, 3 = could not set up the scenario.
static int occupied(uintptr_t where) {
	uint8_t* mine = mmap((void*) where, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
	if (mine != (uint8_t*) where) {
		return 3;
	}
	memset(mine, 0x5a, 4096);
	commpage_setup(true);
	return 2;
}

static int protections(uintptr_t unused) {
	(void) unused;
	char p[8];
	commpage_setup(true);
	volatile uint8_t* data = (uint8_t*) base;
	const char* sig = (const char*) _COMM_PAGE_SIGNATURE;

	if (strcmp(sig, "commpage 64-bit") != 0) return 10;
	// the 'ret' fill of the text part (only x86_64 has one)
	if (length > 0x1000 && (data[0x1000] != 0xc3 || data[length - 1] != 0xc3)) return 11;

	perms_at(base, p);
	if (strcmp(p, "r--p") != 0) { printf("data page perms: %s\n", p); return 12; }
	if (length > 0x1000) {
		perms_at(base + 0x1000, p);
		if (strcmp(p, "r-xp") != 0) { printf("text perms: %s\n", p); return 13; }
		perms_at(base + length - 1, p);
		if (strcmp(p, "r-xp") != 0) { printf("text end perms: %s\n", p); return 14; }
	}
	if (!write_faults(data)) return 15;
	return 0;
}

int main(void) {
	EXPECT(in_child(protections, 0) == 0, "free range: signature, ret fill, data r--p, text r-xp, data not writable");
	EXPECT(in_child(occupied, base) == 1, "mapping at the start of the range is refused");
	// on x86_64 the range spans 32 pages; a mapping in its middle must be refused too
	uintptr_t inside = length > 0x1000 ? base + 5 * 4096 : base;
	EXPECT(in_child(occupied, inside) == 1, "mapping inside the range is refused");

	printf(failures ? "FAILED\n" : "PASS\n");
	return failures ? 1 : 0;
}
