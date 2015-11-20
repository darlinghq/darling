#include "threads.h"
#include <pthread.h>
#include <sys/mman.h>
#include <cstring>

struct arg_struct
{
	void** stack;
	void* entry_point;
	uintptr_t arg3;
	uintptr_t arg4;
	uintptr_t arg5;
	uintptr_t arg6;
	union
	{
		int (*thread_self_trap)();
		int port;
	};
	void* orig_stack;
};

static void* darling_thread_entry(void* p);

#ifndef PTHREAD_STACK_MIN
#	define PTHREAD_STACK_MIN 16384
#endif

int __darling_thread_create(void** stack, void* entry_point, uintptr_t arg3,
		        uintptr_t arg4, uintptr_t arg5, uintptr_t arg6,
				int (*thread_self_trap)())
{

	arg_struct* args = new arg_struct { stack, entry_point, arg3,
		arg4, arg5, arg6, thread_self_trap, nullptr };
	pthread_attr_t attr;
	pthread_t nativeLibcThread;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN); // we don't need a large Glibc stack

	pthread_create(&nativeLibcThread, &attr, darling_thread_entry, args);

	return 0;
}

static void* darling_thread_entry(void* p)
{
	arg_struct* args = static_cast<arg_struct*>(p);
	void** stack = args->stack;

	args->port = args->thread_self_trap();
	
	memcpy(stack - sizeof(arg_struct) / sizeof(void*),
			args, sizeof(arg_struct));
	delete args;

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
}

int __darling_thread_terminate(void* stackaddr,
				unsigned long freesize, unsigned long pthobj_size)
{
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

	__builtin_unreachable();
}

