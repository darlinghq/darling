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

// The point of this file is build macOS threads on top of native libc's threads,
// otherwise it would not be possible to make native calls from these threads.

static __thread jmp_buf t_jmpbuf;
static __thread void* t_freeaddr;
static __thread size_t t_freesize;

typedef void (*thread_ep)(void**, int, ...);
struct arg_struct
{
	thread_ep entry_point;
	uintptr_t arg3;
	uintptr_t arg4;
	uintptr_t arg5;
	uintptr_t arg6;
	union
	{
		int (*thread_self_trap)();
		int port;
	};
	unsigned long pth_obj_size;
	void* pth;
};

static void* darling_thread_entry(void* p);

#ifndef PTHREAD_STACK_MIN
#	define PTHREAD_STACK_MIN 16384
#endif

void* __darling_thread_create(unsigned long stack_size, unsigned long pth_obj_size,
				void* entry_point, uintptr_t arg3,
				uintptr_t arg4, uintptr_t arg5, uintptr_t arg6,
				int (*thread_self_trap)())
{
	struct arg_struct args = { (thread_ep) entry_point, arg3,
		arg4, arg5, arg6, thread_self_trap, pth_obj_size, NULL };
	pthread_attr_t attr;
	pthread_t nativeLibcThread;
	void* pth;

	pthread_attr_init(&attr);
	//pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	// pthread_attr_setstacksize(&attr, stack_size);
	
	pth = mmap(NULL, stack_size + pth_obj_size + 0x1000, PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	
	// pthread_attr_setstack is buggy. The documentation states we should provide the lowest
	// address of the stack, yet some versions regard it as the highest address instead.
	// Therefore it's better to just make the pthread stack as small as possible and then switch
	// to our own stack instead.
	//pthread_attr_setstack(&attr, ((char*)pth) + pth_obj_size, stack_size - pth_obj_size - 0x1000);

	// std::cout << "Allocated stack at " << pth << ", size " << stack_size << std::endl;

	pth = ((char*) pth) + stack_size + 0x1000;
	pthread_attr_setstacksize(&attr, 4096);

	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

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

	args.port = args.thread_self_trap();
	in_args->pth = NULL;

	int freesize;
	if (setjmp(t_jmpbuf))
	{
		// Terminate the Linux thread
		munmap(t_freeaddr, t_freesize);
		return NULL;
	}

#ifdef __x86_64__
	__asm__ __volatile__ (
	"movq %1, %%rdi\n"
	"movq %%rdi, %%rsp\n"
	"movq 40(%0), %%rsi\n"
	"movq 8(%0), %%rdx\n"
	"testq %%rdx, %%rdx\n"
	"jnz 1f\n"
	"movq %%rsp, %%rdx\n" // wqthread hack: if 3rd arg is null, we pass sp
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
	"movl (%1), %%esp\n"
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
	"movl %%esp, %%ecx\n"
	"1:\n"
	"ret\n" // Jump to the address pushed at the beginning
	:: "c" (&args), "d" (args.pth));
#endif
	__builtin_unreachable();
}

int __darling_thread_terminate(void* stackaddr,
				unsigned long freesize, unsigned long pthobj_size)
{
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

void* __darling_thread_get_stack(void)
{
	pthread_attr_t attr;
	void* stackaddr;
	size_t stacksize;

	pthread_getattr_np(pthread_self(), &attr);
	pthread_attr_getstack(&attr, &stackaddr, &stacksize);

	return ((char*)stackaddr) + stacksize - 0x2000;
}
