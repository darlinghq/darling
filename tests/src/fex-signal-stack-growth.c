// A signal that arrives while the main stack is a few bytes from the end of its mapping needs a frame below it:
// the kernel grows the stack when the frame is written, as it does for the program's own calls. Under the
// emulated x86_64 path the handler's frame must be written in a way that grows it too (a write through
// process_vm_writev, for one, does not), or the process dies by SIGSEGV with "signal frame does not fit".
//
// Each variant moves the stack pointer to about 2.5 KB above the lower end of the mapping that holds it, then
// takes a signal whose handler returns: a kill() to itself (delivered when the syscall returns) and an
// interval timer while the program spins (delivered into generated code).

#define _DARWIN_C_SOURCE
#include <alloca.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

static volatile sig_atomic_t handled;

static void on_signal(int sig, siginfo_t* si, void* uc)
{
	(void)sig; (void)si; (void)uc;
	handled++;
}

// The lower end of the mapping that holds `addr`, or 0.
static uintptr_t mapping_start(uintptr_t addr)
{
	FILE* f = fopen("/proc/self/maps", "r");
	if (!f)
		return 0;
	char line[512];
	uintptr_t lo = 0;
	while (fgets(line, sizeof line, f)) {
		unsigned long a, b;
		if (sscanf(line, "%lx-%lx", &a, &b) == 2 && addr >= a && addr < b) {
			lo = a;
			break;
		}
	}
	fclose(f);
	return lo;
}

static int variant;

static int signal_here(void)
{
	handled = 0;
	if (variant == 0) {
		kill(getpid(), SIGUSR1);
	} else {
		struct itimerval t = { {0, 0}, {0, 20000} };
		setitimer(ITIMER_REAL, &t, NULL);
		for (volatile long i = 0; !handled && i < 2000000000L; i++)
			;
	}
	return handled == 1 ? 0 : 1;
}

// Moves the stack pointer to `margin` bytes above the lower end of its mapping, then calls signal_here.
static int near_the_end(size_t margin)
{
	char here;
	uintptr_t lo = mapping_start((uintptr_t)&here);
	if (!lo || (uintptr_t)&here < lo + margin + 4096)
		return 2; // not a stack that has room left to take the test
	size_t down = (uintptr_t)&here - lo - margin;
	volatile char* pad = alloca(down);
	pad[0] = 1; // inside the mapping: the stack has not grown
	return signal_here();
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	struct sigaction sa;
	memset(&sa, 0, sizeof sa);
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = on_signal;
	sa.sa_flags = SA_SIGINFO;
	sigaction(SIGUSR1, &sa, NULL);
	sigaction(SIGALRM, &sa, NULL);

	int failed = 0;
	for (variant = 0; variant < 2; variant++) {
		int rc = near_the_end(2560);
		if (rc == 2) {
			printf("SKIP: variant %d: the stack mapping has no room left to position \n", variant);
			continue;
		}
		printf("%s: a handler's signal frame below a stack that ends 2.5 KB further down (%s)\n", rc ? "FAIL" : "PASS",
			variant ? "timer signal into running code" : "kill() to itself");
		failed |= rc;
	}
	printf(failed ? "SUMMARY: FAIL\n" : "SUMMARY: all passed\n");
	return failed;
}
