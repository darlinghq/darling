/*
This file is part of Darling.

Copyright (C) 2015-2018 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "threads.h"
#include <pthread.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <setjmp.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <stdio.h>
#include <fcntl.h>

#include "dthreads.h"

#include <darlingserver/rpc.h>

extern int __mldr_create_rpc_socket(void);
extern void __mldr_close_rpc_socket(int socket);

// The point of this file is build macOS threads on top of native libc's threads,
// otherwise it would not be possible to make native calls from these threads.

static __thread jmp_buf t_jmpbuf;
static __thread void* t_freeaddr;
static __thread size_t t_freesize;
static __thread int t_server_socket = -1;
static __thread darling_thread_create_callbacks_t t_callbacks = NULL;

typedef void (*thread_ep)(void**, int, ...);
struct arg_struct
{
	thread_ep entry_point;
	uintptr_t real_entry_point;
	uintptr_t arg1; // `user_arg` for normal threads; `keventlist` for workqueues
	uintptr_t arg2; // `stack_addr` for normal threads; `flags` for workqueues
	uintptr_t arg3; // `flags` for normal threads; `nkevents` for workqueues
	union {
		void* _backwards_compat; // kept around to avoid modifiying assembly
		int port;
	};
	unsigned long pth_obj_size;
	void* pth;
	darling_thread_create_callbacks_t callbacks;
	uintptr_t stack_bottom;
	uintptr_t stack_addr;
	bool is_workqueue;
};

static void* darling_thread_entry(void* p);

#ifndef PTHREAD_STACK_MIN
#	define PTHREAD_STACK_MIN 16384
#endif

#define DEFAULT_DTHREAD_GUARD_SIZE 0x1000

static dthread_t dthread_structure_init(dthread_t dthread, size_t guard_size, void* stack_addr, size_t stack_size, void* base_addr, size_t total_size) {
	// the pthread signature is the address of the pthread XORed with the "pointer munge" token passed in by the kernel
	// since the LKM doesn't pass in a token, it's always zero, so the signature is equal to just the address
	dthread->sig = (uintptr_t)dthread;

	dthread->tsd[DTHREAD_TSD_SLOT_PTHREAD_SELF] = dthread;
	dthread->tsd[DTHREAD_TSD_SLOT_ERRNO] = &dthread->err_no;
	dthread->tsd[DTHREAD_TSD_SLOT_PTHREAD_QOS_CLASS] = (void*)(uintptr_t)(DTHREAD_DEFAULT_PRIORITY);
	dthread->tsd[DTHREAD_TSD_SLOT_PTR_MUNGE] = 0;
	dthread->tl_has_custom_stack = 0;
	dthread->lock = (darwin_os_unfair_lock){0};

	dthread->stackaddr = stack_addr;
	dthread->stackbottom = (char*)stack_addr - stack_size;
	dthread->freeaddr = base_addr;
	dthread->freesize = total_size;
	dthread->guardsize = guard_size;

	dthread->cancel_state = DTHREAD_CANCEL_ENABLE | DTHREAD_CANCEL_DEFERRED;

	// technically, these next values are defaults; we don't have a way to get more info from the user
	//
	// it's not too important since the only cases where we initialize the dthread structure ourselves is when we're working with workqueues,
	// and those initialize their own dthread structures when they get them

	dthread->tl_joinable = 1;
	dthread->inherit = DTHREAD_INHERIT_SCHED;
	dthread->tl_policy = DARWIN_POLICY_TIMESHARE;

	return dthread;
};

static dthread_t dthread_structure_allocate(size_t stack_size, size_t guard_size, void** stack_addr) {
	size_t total_size = guard_size + stack_size + sizeof(struct _dthread);

	// allocate our stack, guard page, and dthread structure
	void* base_addr = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	// protect our guard page
	mprotect(base_addr, guard_size, PROT_NONE);

	/**
	 * memory layout of newly allocated block:
	 *
	 * [base_addr]         [base_addr + total_size]
	 * --------------------------------------------
	 * | guard page |       stack       | dthread |
	 */

	// stack_addr points to the top of the stack (i.e. the highest address)
	*stack_addr = ((char*)base_addr) + stack_size + guard_size;

	// the dthread sits above the stack
	// (and by "above", i mean the lowest address of the dthread is the highest address of the stack)
	dthread_t dthread = (dthread_t)*stack_addr;
	// zero-out the entrire dthread structure
	memset(dthread, 0, sizeof(struct _dthread));

	return dthread_structure_init(dthread, guard_size, *stack_addr, stack_size, base_addr, total_size);
};

void* __darling_thread_create(unsigned long stack_size, unsigned long pth_obj_size,
				void* entry_point, uintptr_t real_entry_point,
				uintptr_t arg1, uintptr_t arg2, uintptr_t arg3,
				darling_thread_create_callbacks_t callbacks, void* pth)
{
	struct arg_struct args = {
		.entry_point      = (thread_ep)entry_point,
		.real_entry_point = real_entry_point,
		.arg1             = arg1,
		.arg2             = arg2,
		.arg3             = arg3,
		.port             = 0,
		.pth_obj_size     = pth_obj_size,
		.pth              = NULL, // set later on
		.callbacks        = callbacks,
		.stack_addr       = 0, // set later on
		.is_workqueue     = real_entry_point == 0, // our `workq_kernreturn` sets `real_entry_point` to NULL; `bsdthread_create` actually passes a value
	};
	pthread_attr_t attr;
	pthread_t nativeLibcThread;

	pthread_attr_init(&attr);
	//pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	// pthread_attr_setstacksize(&attr, stack_size);

	// in some cases, we're already given a pthread object, stack, and guard page;
	// in those cases, just use what we're given (it also contains more information)
	//
	// otherwise, allocate them ourselves
	if (pth == NULL || args.is_workqueue) {
		pth = dthread_structure_allocate(stack_size, DEFAULT_DTHREAD_GUARD_SIZE, (void**)&args.stack_addr);
	} else if (!args.is_workqueue) {
		// `arg2` is `stack_addr` for normal threads
		args.stack_addr = arg2;
	}

	args.stack_bottom = args.stack_addr - stack_size;

	// pthread_attr_setstack is buggy. The documentation states we should provide the lowest
	// address of the stack, yet some versions regard it as the highest address instead.
	// Therefore it's better to just make the pthread stack as small as possible and then switch
	// to our own stack instead.
	//pthread_attr_setstack(&attr, ((char*)pth) + pth_obj_size, stack_size - pth_obj_size - 0x1000);

	// std::cout << "Allocated stack at " << pth << ", size " << stack_size << std::endl;

	pthread_attr_setstacksize(&attr, 4096);

	//pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	args.pth = pth;
	pthread_create(&nativeLibcThread, &attr, darling_thread_entry, &args);
	pthread_attr_destroy(&attr);

	while (args.pth != NULL)
		sched_yield();

	return pth;
}

static void* darling_thread_entry(void* p)
{
	struct arg_struct* in_args = (struct arg_struct*) p;
	struct arg_struct args;

	memcpy(&args, in_args, sizeof(args));

	dthread_t dthread = args.pth;
	uintptr_t* flags = args.is_workqueue ? &args.arg2 : &args.arg3;

	// create a new dserver RPC socket
	int new_rpc_fd = __mldr_create_rpc_socket();
	if (new_rpc_fd < 0) {
		// we can't do anything if we don't get our own separate connection to darlingserver
		fprintf(stderr, "Failed to create socket\n");
		abort();
	}

	// guard the new RPC FD
	args.callbacks->rpc_guard(new_rpc_fd);

	// the socket is ready; assign it now
	t_server_socket = new_rpc_fd;
	t_callbacks = args.callbacks;

	// libpthread now expects the kernel to set the TSD
	// so, since we're pretending to be the kernel handling threads...
	args.callbacks->thread_set_tsd_base(&dthread->tsd[0], 0);
	*flags |= args.is_workqueue ? DWQ_FLAG_THREAD_TSD_BASE_SET : DTHREAD_START_TSD_BASE_SET;

	// let's check-in with darlingserver on this new thread
	int dummy_stack_variable;
	// the lifetime pipe fd is ignored as the proces should already have been registered
	if (dserver_rpc_explicit_checkin(t_server_socket, false, &dummy_stack_variable, -1) < 0) {
		// we can't do ANYTHING if darlingserver doesn't acknowledge us successfully
		abort();
	}

	int thread_self_port = args.callbacks->thread_self_trap();
	dthread->tsd[DTHREAD_TSD_SLOT_MACH_THREAD_SELF] = (void*)(intptr_t)thread_self_port;
	args.port = thread_self_port;

	in_args->pth = NULL;

	if (setjmp(t_jmpbuf))
	{
		// Terminate the Linux thread
		munmap(t_freeaddr, t_freesize);
		pthread_detach(pthread_self());
		return NULL;
	}

#ifdef __x86_64__
	__asm__ __volatile__ (
	"movq %1, %%rdi\n"
	"movq 80(%0), %%rsp\n"
	"movq 40(%0), %%rsi\n"
	"movq 8(%0), %%rdx\n"
	"testq %%rdx, %%rdx\n"
	"jnz 1f\n"
	"movq 72(%0), %%rdx\n" // wqthread hack: if 3rd arg is null, we pass the stack bottom
	"1:\n"
	"movq 16(%0), %%rcx\n"
	"movq 24(%0), %%r8\n"
	"movq 32(%0), %%r9\n"
	"movq %%rdi, 56(%0)\n"
	"movq (%0), %%rax\n"
	"andq $-0x10, %%rsp\n"
	"pushq $0\n"
	"pushq $0\n"
	"jmpq *%%rax\n"
	:: "a" (&args), "di" (args.pth));
#elif defined(__i386__) // args in eax, ebx, ecx, edx, edi, esi
	__asm__ __volatile__ (
	"movl (%0), %%eax\n"
	"movl 40(%0), %%esp\n"
	"pushl %%eax\n" // address to be jumped to
	"movl %1, 28(%0)\n"
	"movl %1, %%eax\n" // 1st arg
	"movl 20(%0), %%ebx\n" // 2nd arg
	"movl 8(%0), %%edx\n" // 4th arg
	"movl 12(%0), %%edi\n" // 5th arg
	"movl 16(%0), %%esi\n" // 6th arg
	"movl 4(%0), %%ecx\n" // 3rd arg
	"testl %%ecx, %%ecx\n" // FIXME: clobbered ecx!
	"jnz 1f\n"
	"movl 36(%0), %%ecx\n" // if the 3rd argument is null, pass the stack bottom
	"1:\n"
	"ret\n" // Jump to the address pushed at the beginning
	:: "c" (&args), "d" (args.pth));
#else
#error Not implemented
#endif
	__builtin_unreachable();
}

int __darling_thread_terminate(void* stackaddr,
				unsigned long freesize, unsigned long pthobj_size)
{
	int checkout_result = 0;

	if (t_server_socket != -1) {
		checkout_result = dserver_rpc_explicit_checkout(t_server_socket, -1, false);
	} else {
		checkout_result = dserver_rpc_checkout(-1, false);
	}

	if (checkout_result < 0) {
		// failing to check-out is not fatal.
		// it's not ideal, but it's not fatal.
		#define CHECKOUT_FAILURE_MESSAGE "Failed to checkout"
		if (t_server_socket != -1) {
			dserver_rpc_explicit_kprintf(t_server_socket, CHECKOUT_FAILURE_MESSAGE, sizeof(CHECKOUT_FAILURE_MESSAGE) - 1);
		} else {
			dserver_rpc_kprintf(CHECKOUT_FAILURE_MESSAGE, sizeof(CHECKOUT_FAILURE_MESSAGE) - 1);
		}
	}

	// close the RPC FD (if necessary)
	// it should already have been unguarded by our caller
	if (t_server_socket != -1) {
		__mldr_close_rpc_socket(t_server_socket);
	}

	if (getpid() == syscall(SYS_gettid))
	{
		// dispatch_main() calls pthread_exit(NULL) on the main thread,
		// which turns our process into a zombie on Linux.
		// Let's just hang around forever.
		sigset_t mask;
		memset(&mask, 0, sizeof(mask));

		while (1)
			sigsuspend(&mask);
	}

	t_freeaddr = stackaddr;
	t_freesize = freesize;

	longjmp(t_jmpbuf, 1);

	__builtin_unreachable();
}

extern void* __mldr_main_stack_top;

void* __darling_thread_get_stack(void)
{
	return __mldr_main_stack_top;
}

extern int __dserver_main_thread_socket_fd;

int __darling_thread_rpc_socket(void) {
	if (t_server_socket == -1) {
		if (getpid() == syscall(SYS_gettid)) {
			// this is the main thread
			t_server_socket = __dserver_main_thread_socket_fd;
		} else {
			// threads should already have a per-thread socket assigned when they're created
			abort();
		}
	}
	return t_server_socket;
};

void __darling_thread_rpc_socket_refresh(void) {
	int new_rpc_fd = __mldr_create_rpc_socket();
	if (new_rpc_fd < 0) {
		abort();
	}

	t_server_socket = new_rpc_fd;

	// if this is the main thread, also update the socket used by mldr
	if (getpid() == syscall(SYS_gettid)) {
		__dserver_main_thread_socket_fd = t_server_socket;
	}
};
