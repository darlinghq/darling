// Copyright (C) 2026 Lubos Dolezel
//
// Signals delivered to a thread the Darwin runtime did not create. A signal
// sent to such a thread (pthread_kill/tgkill) must not take the process down:
// the installed handler runs, on that thread or, where a thread without Darwin
// state cannot run it, on another thread of the process.
//
//   prog            the signalled thread is made with pthread_create, and a
//                   fault on it also runs the SIGSEGV handler there (the
//                   baseline; runs on macOS)
//   prog native     the thread is created with a raw Linux clone() on the Darling
//                   guest, with no Darwin thread state (stands in for a thread
//                   created by a host library). x86_64 and i386 only. Only
//                   SIGUSR1 is checked: a fault on a thread without Darwin
//                   state is not handed to the application's handler.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>

static int g_failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static volatile int g_segv_handled, g_usr1_handled, g_thread_done, g_segv_in_handler;

static void on_alarm(int s) { (void) s; printf("FAIL: timed out (hang)\n"); _exit(2); }

// Ends the calling thread without returning to the faulting instruction.
static void exit_native_thread(void)
{
#if defined(__x86_64__)
	__asm__ volatile("mov $60, %eax\n xor %edi, %edi\n syscall");
#elif defined(__i386__)
	__asm__ volatile("mov $1, %eax\n xor %ebx, %ebx\n int $0x80");
#endif
	for (;;) { }
}

static void on_segv(int sig, siginfo_t *info, void *ctx)
{
	g_segv_in_handler = 1;
	g_segv_handled = 1;
	if (g_thread_done == 0) {
		// Thread-exit for the raw thread; the pthread variant ends the thread
		// through pthread_exit instead.
		extern int g_use_native;
		if (g_use_native)
			exit_native_thread();
		pthread_exit(NULL);
	}
}

static void on_usr1(int sig)
{
	g_usr1_handled = 1;
}

int g_use_native;

static void thread_body(void)
{
	// 1. A signal sent to this thread: the handler runs and returns.
#if defined(__x86_64__)
	long tid;
	__asm__ volatile("syscall" : "=a"(tid) : "0"(186L) : "rcx", "r11", "memory");
	long pid;
	__asm__ volatile("syscall" : "=a"(pid) : "0"(39L) : "rcx", "r11", "memory");
	long r;
	if (g_use_native)
		__asm__ volatile("syscall" : "=a"(r) : "0"(234L), "D"(pid), "S"(tid), "d"(10L) : "rcx", "r11", "memory");
	else
		pthread_kill(pthread_self(), SIGUSR1);
#elif defined(__i386__)
	long tid, pid, r;
	if (g_use_native) {
		__asm__ volatile("int $0x80" : "=a"(tid) : "0"(224L) : "memory");
		__asm__ volatile("int $0x80" : "=a"(pid) : "0"(20L) : "memory");
		__asm__ volatile("int $0x80" : "=a"(r) : "0"(270L), "b"(pid), "c"(tid), "d"(10L) : "memory");
	} else
		pthread_kill(pthread_self(), SIGUSR1);
#else
	pthread_kill(pthread_self(), SIGUSR1);
#endif
	if (g_use_native) {
		g_thread_done = 1;
		return;
	}
	// 2. A fault on this thread.
	g_thread_done = 1;
	g_thread_done = 0;
	*(volatile int *) 8 = 1;
	g_thread_done = 1;
}

static void *pthread_main(void *arg)
{
	thread_body();
	return NULL;
}

static void *new_stack(size_t size)
{
	return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
}

#if defined(__x86_64__)
static long raw_clone_thread(void (*fn)(void))
{
	size_t size = 256 * 1024;
	char *base = new_stack(size);
	// Entry point is stored at the top; the child pops it and calls it with a
	// 16-byte aligned stack.
	uintptr_t top = ((uintptr_t) base + size - 64) & ~(uintptr_t) 15;
	top -= 8;
	*(void (**)(void)) top = fn;
	// A zeroed TLS block that points to itself, like a C library's thread block.
	void **tls = calloc(1, 4096);
	tls[0] = tls;
	long flags = 0x100 | 0x200 | 0x400 | 0x800 | 0x10000 | 0x40000 | 0x80000;
	long ret;
	register long r10 __asm__("r10") = 0;
	register void *r8 __asm__("r8") = tls;
	__asm__ volatile(
		"syscall\n\t"
		"test %%rax, %%rax\n\t"
		"jnz 1f\n\t"
		"xor %%ebp, %%ebp\n\t"
		"pop %%rax\n\t"
		"call *%%rax\n\t"
		"mov $60, %%eax\n\t"
		"xor %%edi, %%edi\n\t"
		"syscall\n"
		"1:"
		: "=a"(ret)
		: "0"(56L), "D"(flags), "S"(top), "d"(0L), "r"(r10), "r"(r8)
		: "rcx", "r11", "memory");
	return ret;
}
#elif defined(__i386__)
static long raw_clone_thread(void (*fn)(void))
{
	size_t size = 256 * 1024;
	char *base = new_stack(size);
	uintptr_t top = ((uintptr_t) base + size - 64) & ~(uintptr_t) 15;
	top -= 4;
	*(void (**)(void)) top = fn;
	long flags = 0x100 | 0x200 | 0x400 | 0x800 | 0x10000 | 0x40000;
	long ret;
	__asm__ volatile(
		"int $0x80\n\t"
		"test %%eax, %%eax\n\t"
		"jnz 1f\n\t"
		"xor %%ebp, %%ebp\n\t"
		"pop %%eax\n\t"
		"call *%%eax\n\t"
		"mov $1, %%eax\n\t"
		"xor %%ebx, %%ebx\n\t"
		"int $0x80\n"
		"1:"
		: "=a"(ret)
		: "0"(120L), "b"(flags), "c"(top), "d"(0L), "S"(0L), "D"(0L)
		: "memory");
	return ret;
}
#else
static long raw_clone_thread(void (*fn)(void)) { return -1; }
#endif

int main(int argc, char **argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	signal(SIGALRM, on_alarm);
	alarm(30);
	g_use_native = argc > 1 && !strcmp(argv[1], "native");

	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_sigaction = on_segv;
	sa.sa_flags = SA_SIGINFO;
	sigemptyset(&sa.sa_mask);
	CHECK(sigaction(SIGSEGV, &sa, NULL) == 0);
	sa.sa_sigaction = NULL;
	sa.sa_handler = on_usr1;
	sa.sa_flags = 0;
	CHECK(sigaction(SIGUSR1, &sa, NULL) == 0);

	if (g_use_native) {
		long r = raw_clone_thread(thread_body);
		printf("raw clone: %ld\n", r);
		CHECK(r > 0);
	} else {
		pthread_t th;
		CHECK(pthread_create(&th, NULL, pthread_main, NULL) == 0);
	}

	for (int i = 0; i < 300 && !(g_use_native ? g_usr1_handled : g_segv_handled); i++)
		usleep(10000);
	printf("%s thread: SIGUSR1 handled=%d, SIGSEGV handled=%d\n", g_use_native ? "native" : "pthread", g_usr1_handled, g_segv_handled);
	CHECK(g_usr1_handled == 1);
	if (!g_use_native)
		CHECK(g_segv_handled == 1);

	// The process is still functional.
	usleep(100000);
	printf(g_failures ? "FAILED (%d)\n" : "PASSED\n", g_failures);
	return g_failures != 0;
}
