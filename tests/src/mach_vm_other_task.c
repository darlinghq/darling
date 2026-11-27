/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Calls on another task's port (mach_vm_protect, mach_vm_purgable_control), which are answered by the Mach
 * server rather than the caller's own trap:
 *  - mach_vm_protect() with set_maximum only lowers the maximum protection and never grants a right: raising
 *    the maximum of an inaccessible page to VM_PROT_ALL leaves it inaccessible;
 *  - mach_vm_purgable_control() reports memory that is not purgeable as VM_PURGABLE_NONVOLATILE.
 */
#include <dlfcn.h>
#include <errno.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)

static sigjmp_buf jump;
static void on_segv(int sig) { (void) sig; siglongjmp(jump, 1); }

static int child_main(volatile char* guard, int rfd, int wfd)
{
	char c;
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = on_segv;
	sigaction(SIGSEGV, &sa, NULL);
	sigaction(SIGBUS, &sa, NULL);
	write(wfd, "r", 1);
	read(rfd, &c, 1);
	/* the page is still inaccessible: the write faults */
	if (sigsetjmp(jump, 1) == 0) {
		guard[0] = 1;
		write(wfd, "w", 1); /* wrote: the maximum protection was granted */
	} else {
		write(wfd, "f", 1);
	}
	return 0;
}

int main(void)
{
	int up[2], down[2];
	long page = sysconf(_SC_PAGESIZE);
	char* guard = mmap(NULL, page, PROT_NONE, MAP_ANON | MAP_PRIVATE, -1, 0);

	CHECK(guard != MAP_FAILED);
	CHECK(pipe(up) == 0 && pipe(down) == 0);
	pid_t pid = fork();
	if (pid == 0)
		_exit(child_main((volatile char*) guard, down[0], up[1]));

	char c = 0;
	CHECK(read(up[0], &c, 1) == 1 && c == 'r');

	task_t task = MACH_PORT_NULL;
	kern_return_t kr = task_for_pid(mach_task_self(), pid, &task);
	if (kr != KERN_SUCCESS) {
		printf("skipped: task_for_pid failed with %d\n", kr);
		write(down[1], "g", 1);
		waitpid(pid, NULL, 0);
		return 0;
	}

	kr = mach_vm_protect(task, (mach_vm_address_t) guard, page, TRUE, VM_PROT_ALL);
	CHECK(kr == KERN_SUCCESS);

	write(down[1], "g", 1);
	CHECK(read(up[0], &c, 1) == 1 && c == 'f');

	/* the Mach server's answer, which mach_vm_purgable_control() asks when the trap does not apply */
	kern_return_t (*purgable_control)(task_t, mach_vm_address_t, vm_purgable_t, int*) =
		dlsym(RTLD_DEFAULT, "_kernelrpc_mach_vm_purgable_control");
	if (purgable_control) {
		int state = VM_PURGABLE_VOLATILE;
		kr = purgable_control(task, (mach_vm_address_t) guard, VM_PURGABLE_GET_STATE, &state);
		CHECK(kr == KERN_SUCCESS);
		CHECK(state == VM_PURGABLE_NONVOLATILE);
	}

	waitpid(pid, NULL, 0);
	if (!failures)
		puts("PASS");
	return failures != 0;
}
