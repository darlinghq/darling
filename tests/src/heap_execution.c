// Copyright (C) 2026 Lubos Dolezel
//
// Which read/write memory can be executed depends on the main executable's
// Mach-O header flags on x86 macOS (Mojave 10.14.4 black-box runs, i386 and
// x86_64 builds with and without -Wl,-allow_heap_execute / -allow_stack_execute):
//  - an i386 executable without MH_NO_HEAP_EXECUTION runs code from malloc'd
//    memory (tiny, small, large), anonymous private and shared mmap, memory
//    that went PROT_NONE -> PROT_READ|PROT_WRITE, vm_allocate, the data and bss
//    segments and file-backed private/shared read/write mappings, but not from
//    the stack;
//  - an i386 executable with MH_NO_HEAP_EXECUTION and any x86_64 executable
//    fault (SIGBUS or SIGSEGV) for all of these;
//  - MH_ALLOW_STACK_EXECUTION, on either architecture, makes all of them and the
//    stack executable;
//  - an explicit PROT_EXEC mapping always works.
// The expectation is derived from this program's own header flags, so build it
// each way to cover all rows, e.g. for i386:
//   clang -arch i386 -o heap_execution heap_execution.c                          (NO_HEAP_EXECUTION)
//   clang -arch i386 -Wl,-allow_heap_execute -o heap_execution heap_execution.c  (executable heap)
//   clang -arch i386 -Wl,-allow_stack_execute -o heap_execution heap_execution.c (all executable)
// and the same for x86_64 (-allow_heap_execute is rejected there).
// Each case runs in a fresh process (a fork child without exec does not keep
// the restriction on macOS).
#include <mach-o/ldsyms.h>
#include <mach-o/loader.h>
#include <mach/mach.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#if !defined(__i386__) && !defined(__x86_64__)
int main(void)
{
	printf("SKIP x86 only\n");
	return 0;
}
#else

static int failures;
static void check(int ok, const char* what) { printf("%s: %s\n", ok ? "PASS" : "FAIL", what); if (!ok) failures++; }

static const unsigned char code[] = { 0xb8, 42, 0, 0, 0, 0xc3 }; // mov eax, 42; ret

static unsigned char data_page[8192] __attribute__((aligned(4096))) = { 1 };
static unsigned char bss_page[8192] __attribute__((aligned(4096)));

static const char* const kinds[] = {
	"malloc-tiny", "malloc-small", "malloc-large", "mmap-anon", "mmap-anon-shared",
	"mmap-none-then-rw", "vm-allocate", "data", "bss", "file-private", "file-shared", "stack",
	"mmap-explicit-exec",
};
#define NKINDS (sizeof kinds / sizeof *kinds)

static int run_kind(const char* kind, const char* self)
{
	unsigned char* p = NULL;
	char stack_buf[256];
	const size_t len = 65536;

	if (!strcmp(kind, "malloc-tiny")) p = malloc(100);
	else if (!strcmp(kind, "malloc-small")) p = malloc(3000);
	else if (!strcmp(kind, "malloc-large")) p = malloc(600000);
	else if (!strcmp(kind, "mmap-anon")) p = mmap(0, len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
	else if (!strcmp(kind, "mmap-anon-shared")) p = mmap(0, len, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
	else if (!strcmp(kind, "mmap-none-then-rw"))
	{
		p = mmap(0, len, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
		if (p != MAP_FAILED) mprotect(p, len, PROT_READ | PROT_WRITE);
	}
	else if (!strcmp(kind, "vm-allocate"))
	{
		vm_address_t va = 0;
		if (vm_allocate(mach_task_self(), &va, len, VM_FLAGS_ANYWHERE) != KERN_SUCCESS) return 3;
		p = (unsigned char*) va;
	}
	else if (!strcmp(kind, "data")) p = data_page;
	else if (!strcmp(kind, "bss")) p = bss_page;
	else if (!strcmp(kind, "file-private"))
	{
		int fd = open(self, O_RDONLY);
		p = mmap(0, len, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	}
	else if (!strcmp(kind, "file-shared"))
	{
		char tmpl[] = "/tmp/heap_execution.XXXXXX";
		int fd = mkstemp(tmpl);
		if (fd < 0) return 3;
		unlink(tmpl);
		ftruncate(fd, len);
		p = mmap(0, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	}
	else if (!strcmp(kind, "stack")) p = (unsigned char*) stack_buf;
	else if (!strcmp(kind, "mmap-explicit-exec")) p = mmap(0, len, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANON, -1, 0);
	else return 3;

	if (p == NULL || p == (unsigned char*) MAP_FAILED) return 3;
	memcpy(p, code, sizeof code);
	return ((int (*)(void)) p)() == 42 ? 0 : 1;
}

int main(int argc, char** argv)
{
	if (argc > 1)
		return run_kind(argv[1], argv[0]);

	setvbuf(stdout, NULL, _IONBF, 0);
	uint32_t flags = _mh_execute_header.flags;
#ifdef __i386__
	int is_i386 = 1;
#else
	int is_i386 = 0;
#endif
	int stack_exec = (flags & MH_ALLOW_STACK_EXECUTION) != 0;
	int data_exec = stack_exec || (is_i386 && !(flags & MH_NO_HEAP_EXECUTION));
	printf("%s, header flags %#x: expect executable data %d, stack %d\n", is_i386 ? "i386" : "x86_64", flags, data_exec, stack_exec);

	for (size_t i = 0; i < NKINDS; i++)
	{
		int expect_exec = !strcmp(kinds[i], "mmap-explicit-exec") ? 1 : !strcmp(kinds[i], "stack") ? stack_exec : data_exec;
		pid_t pid = fork();
		if (pid == 0)
		{
			execl(argv[0], argv[0], kinds[i], (char*) NULL);
			_exit(99);
		}
		int st = 0;
		waitpid(pid, &st, 0);
		int ran = WIFEXITED(st) && WEXITSTATUS(st) == 0;
		int faulted = WIFSIGNALED(st) && (WTERMSIG(st) == SIGSEGV || WTERMSIG(st) == SIGBUS);
		char what[96];
		snprintf(what, sizeof what, "%s %s", kinds[i], expect_exec ? "is executable" : "faults");
		check(expect_exec ? ran : faulted, what);
	}
	return failures ? 1 : 0;
}

#endif
