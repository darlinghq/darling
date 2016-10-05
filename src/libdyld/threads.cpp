/*
This file is part of Darling.

Copyright (C) 2015 Lubos Dolezel

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
#include <cstring>
#include <queue>
#include <iostream>

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
struct reaper_item
{
	pthread_t thread;
	void* stack;
	size_t stacksize;
};

static void* darling_thread_entry(void* p);
static void start_reaper();

static sem_t reaper_sem;
static pthread_mutex_t reaper_mutex = PTHREAD_MUTEX_INITIALIZER;
static std::queue<reaper_item> reaper_items;

#ifndef PTHREAD_STACK_MIN
#	define PTHREAD_STACK_MIN 16384
#endif

void* __darling_thread_create(unsigned long stack_size, unsigned long pth_obj_size,
				void* entry_point, uintptr_t arg3,
				uintptr_t arg4, uintptr_t arg5, uintptr_t arg6,
				int (*thread_self_trap)())
{
	static pthread_once_t reaper_once = PTHREAD_ONCE_INIT;

	arg_struct args = { (thread_ep) entry_point, arg3,
		arg4, arg5, arg6, thread_self_trap, pth_obj_size, nullptr };
	pthread_attr_t attr;
	pthread_t nativeLibcThread;
	void* pth;

	pthread_once(&reaper_once, start_reaper);

	pthread_attr_init(&attr);
	//pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	// pthread_attr_setstacksize(&attr, stack_size);
	
	pth = ::mmap(nullptr, stack_size + pth_obj_size + 0x1000, PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	pthread_attr_setstack(&attr, ((char*)pth) + pth_obj_size, stack_size - pth_obj_size - 0x1000);

	// std::cout << "Allocated stack at " << pth << ", size " << stack_size << std::endl;
	pth = static_cast<char*>(pth) + stack_size + 0x1000;

	args.pth = pth;
	pthread_create(&nativeLibcThread, &attr, darling_thread_entry, &args);
	pthread_attr_destroy(&attr);
	
	while (args.pth != nullptr)
		sched_yield();

	return pth;
}

static void* darling_thread_entry(void* p)
{
	arg_struct* in_args = static_cast<arg_struct*>(p);
	arg_struct args;
	
	memcpy(&args, in_args, sizeof(args));

	args.port = args.thread_self_trap();
	in_args->pth = nullptr;

#ifdef __x86_64__
	__asm__ __volatile__ (
	"movq %1, %%rdi\n"
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
#elif defined(__i386__) // TODO: args in eax, ebx, ecx, edx, edi, esi
	__asm__ __volatile__ (
	"movl (%0), %%eax\n"
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
	//args->entry_point(args->stack, args->port, args->arg3,
	//		args->arg4, args->arg5, args->arg6);
	
	//memcpy(stack - sizeof(arg_struct) / sizeof(void*),
	//		args, sizeof(arg_struct));
	//delete args;
	

#if 0
#ifdef __x86_64__
	__asm__ (
			"movq %%rsp, -8(%0)\n" // save original stack location
			"movq %0, %%rsp\n" // switch to Darwin libc stack
			"movq -56(%%rsp), %%rax\n"
			"movq %%rsp, %%rdi\n" // stack/pthread_self is 1st argument
			"movl -16(%%rsp), %%esi\n" // port is 2nd argument
			"movq -48(%%rsp), %%rdx\n"
			"movq -40(%%rsp), %%rcx\n"
			"movq -32(%%rsp), %%r8\n"
			"movq -24(%%rsp), %%r9\n"
			"subq $24, %%rsp\n" // protect orig_stack + alignment
			"callq *%%rax\n"
			"ud2\n"
			:: "r" (stack));
#elif defined(__i386__)
#   warning Missing assembly for this architecture!
#else
#   warning Missing assembly for this architecture!
#endif

	__builtin_unreachable();
#endif
	return nullptr;
}

int __darling_thread_terminate(void* stackaddr,
				unsigned long freesize, unsigned long pthobj_size)
{
#if 0
#ifdef __x86_64__
	__asm__ ("movq %0, %%rdi\n"
			"movq %1, %%rsi\n"
			"movq -8(%2), %%rsp\n" // restore original glibc thread stack
			//"call munmap@PLT\n"
			"xorl %%edi, %%edi\n"
			"call pthread_exit@PLT\n"
			:: "a" (stackaddr), "c" (freesize),
			"b" (uintptr_t(stackaddr)+freesize-pthobj_size)
	);
#elif defined(__i386__)
	// TODO: PIC magic probably needed
	__asm__ ("movl (%0), %%esp\n" // restore original glibc thread stack
			"pushl %1\n"
			"pushl %0\n"
			"call munmap@PLT\n"
			"addl $8, %%esp\n"
			"pushl $0\n"
			"call pthread_exit@PLT\n"
			:: "r" (stackaddr), "r" (freesize)
	);
#else
#	warning Missing assembly for this architecture!
#endif
#endif

	pthread_mutex_lock(&reaper_mutex);
	reaper_items.push(reaper_item { pthread_self(), stackaddr, freesize });
	pthread_mutex_unlock(&reaper_mutex);

	sem_post(&reaper_sem);

	pthread_exit(nullptr);

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

static void* reaper_entry(void*)
{
	while (true)
	{
		reaper_item item;

		sem_wait(&reaper_sem);

		pthread_mutex_lock(&reaper_mutex);
		item = reaper_items.front();
		reaper_items.pop();
		pthread_mutex_unlock(&reaper_mutex);
		
		// std::cout << "Reaping thread " << (void*)item.thread << "; Free stack at " << item.stack << ", " << item.stacksize << " bytes\n";

		// Wait for thread to terminate
		pthread_join(item.thread, nullptr);

		// Free its stack in the extended requested by Darwin's libc
		munmap(item.stack, item.stacksize);
	}
}

static void start_reaper()
{
	pthread_attr_t attr;
	pthread_t thread;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	sem_init(&reaper_sem, 0, 0);
	pthread_create(&thread, &attr, reaper_entry, nullptr);
	pthread_attr_destroy(&attr);
}

