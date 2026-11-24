// A signal handler that leaves through siglongjmp() must not leak anything per signal: the emulated x86_64
// path keeps the interrupted context somewhere while a handler runs, and a longjmp skips rt_sigreturn.
// Each variant takes 10000 real faults (more than the 8 MB host stack held when every one left a 2 KB
// backup behind) and checks that the handler ran every time and that the process is still healthy.
//
// Run as: darling exec fex-signal-longjmp-loop   (x86_64 and arm64 slices both pass)

#define _DARWIN_C_SOURCE
#include <fcntl.h>
#include <setjmp.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define ROUNDS 10000
#define ALTSIZE (64 * 1024)

static int failed;
#define CHECK(cond, ...) do { if (!(cond)) { printf("FAIL: " __VA_ARGS__); printf("\n"); failed = 1; } } while (0)

static volatile char* prot_none;   // reading it raises SIGSEGV
static volatile char* beyond_eof;  // reading it raises SIGBUS
static sigjmp_buf env;
static volatile sig_atomic_t segv_runs, bus_runs, depth_max;
static volatile int depth;

static void install(int sig, void (*h)(int, siginfo_t*, void*), int flags)
{
	struct sigaction sa;
	memset(&sa, 0, sizeof sa);
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = h;
	sa.sa_flags = SA_SIGINFO | flags;
	if (sigaction(sig, &sa, NULL) != 0) {
		perror("sigaction");
		exit(2);
	}
}

static void segv_leave(int sig, siginfo_t* si, void* uc)
{
	(void)sig; (void)si; (void)uc;
	segv_runs++;
	siglongjmp(env, 1);
}

// A handler that takes another signal (SIGBUS) before it leaves: two host frames to drop per fault.
static void bus_leave(int sig, siginfo_t* si, void* uc)
{
	(void)sig; (void)si; (void)uc;
	bus_runs++;
	siglongjmp(env, 1);
}

static void segv_then_bus(int sig, siginfo_t* si, void* uc)
{
	(void)sig; (void)si; (void)uc;
	segv_runs++;
	(void)*beyond_eof;
	siglongjmp(env, 2);  // not reached: the SIGBUS handler leaves
}

// Handler that unblocks its signal and faults again from inside itself, a few levels deep, then leaves from the
// innermost one (it does not rely on SA_NODEFER).
static void segv_recursive(int sig, siginfo_t* si, void* uc)
{
	(void)sig; (void)si; (void)uc;
	sigset_t unblock;
	sigemptyset(&unblock);
	sigaddset(&unblock, SIGSEGV);
	sigprocmask(SIG_UNBLOCK, &unblock, NULL);
	segv_runs++;
	if (++depth > depth_max)
		depth_max = depth;
	if (depth < 3)
		(void)*prot_none;
	siglongjmp(env, 1);
}

static int one_fault(volatile char* p)
{
	if (sigsetjmp(env, 1))
		return 1;
	(void)*p;
	return 0;
}

static void run_variant(const char* name, int (*body)(void))
{
	int rc = body();
	CHECK(rc == 0, "%s (code %d)", name, rc);
	if (rc == 0)
		printf("PASS: %s\n", name);
}

static int plain(void)
{
	segv_runs = 0;
	install(SIGSEGV, segv_leave, 0);
	for (int i = 0; i < ROUNDS; i++)
		if (!one_fault(prot_none))
			return 1;
	return segv_runs == ROUNDS ? 0 : 2;
}

static int on_altstack(void)
{
	static char alt[ALTSIZE];
	stack_t ss = { .ss_sp = alt, .ss_size = sizeof alt, .ss_flags = 0 };
	if (sigaltstack(&ss, NULL) != 0)
		return 3;
	segv_runs = 0;
	install(SIGSEGV, segv_leave, SA_ONSTACK);
	for (int i = 0; i < ROUNDS; i++)
		if (!one_fault(prot_none))
			return 1;
	ss.ss_flags = SS_DISABLE;
	sigaltstack(&ss, NULL);
	return segv_runs == ROUNDS ? 0 : 2;
}

static int nested(void)
{
	segv_runs = bus_runs = 0;
	install(SIGSEGV, segv_then_bus, 0);
	install(SIGBUS, bus_leave, 0);
	for (int i = 0; i < ROUNDS; i++)
		if (one_fault(prot_none) != 1)
			return 1;
	if (segv_runs != ROUNDS || bus_runs != ROUNDS)
		printf("nested: segv handler ran %d times, bus handler %d times\n", (int)segv_runs, (int)bus_runs);
	return segv_runs == ROUNDS && bus_runs == ROUNDS ? 0 : 2;
}

static int recursive(void)
{
	segv_runs = 0;
	install(SIGSEGV, segv_recursive, 0);
	for (int i = 0; i < ROUNDS; i++) {
		depth = 0;
		if (!one_fault(prot_none))
			return 1;
	}
	return segv_runs == 3 * ROUNDS && depth_max == 3 ? 0 : 2;
}

// After all the longjmps a handler that returns normally still works (the saved context is the right one).
static volatile sig_atomic_t fixed_runs;
static void segv_fix(int sig, siginfo_t* si, void* uc)
{
	(void)sig; (void)si; (void)uc;
	fixed_runs++;
	mprotect((void*)prot_none, 4096, PROT_READ | PROT_WRITE);
}

static int returns_after(void)
{
	install(SIGSEGV, segv_fix, 0);
	fixed_runs = 0;
	volatile char c = *prot_none;
	(void)c;
	mprotect((void*)prot_none, 4096, PROT_NONE);
	return fixed_runs == 1 ? 0 : 1;
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	prot_none = mmap(NULL, 4096, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
	char path[] = "/tmp/fex-longjmp.XXXXXX";
	int fd = mkstemp(path);
	unlink(path);
	if (prot_none == MAP_FAILED || fd < 0 || ftruncate(fd, 1) != 0) {
		perror("setup");
		return 2;
	}
	// The file is one byte long: the page of the host (up to 64 KB) holding it is valid, the next one raises SIGBUS.
	char* file_map = mmap(NULL, 2 * 64 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	beyond_eof = file_map + 64 * 1024;
	if (file_map == MAP_FAILED) {
		perror("mmap");
		return 2;
	}

	run_variant("10000 SIGSEGV handlers left by siglongjmp", plain);
	run_variant("10000 SIGSEGV handlers on the alternate stack left by siglongjmp", on_altstack);
	run_variant("10000 SIGSEGV handlers that take a SIGBUS and leave from it", nested);
	run_variant("10000 recursive (3 deep) handlers left from the innermost", recursive);
	run_variant("a handler that returns still works after the longjmps", returns_after);
	printf(failed ? "SUMMARY: FAIL\n" : "SUMMARY: all passed\n");
	return failed;
}
