#include "bsdthread_create.h"
#include "bsdthread_register.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include <stddef.h>
#include <stdint.h>
#include "../../../../../platform-include/sys/mman.h"
#include "../mman/mman.h"

extern void *memset(void *s, int c, size_t n);

#define PTHREAD_START_CUSTOM    0x01000000
#define PTHREAD_START_SETSCHED  0x02000000
#define PTHREAD_START_DETACHED  0x04000000
#define PTHREAD_START_POLICY_BITSHIFT 16
#define PTHREAD_START_POLICY_MASK 0xff
#define PTHREAD_START_IMPORTANCE_MASK 0xffff

#define STACK_GUARD_SIZE 4096

// http://www.tldp.org/FAQ/Threads-FAQ/clone.c

long sys_bsdthread_create(void* thread_start, void* arg, void** stack,
		void* pthread, uint32_t flags)
{
	int ret;

	if (!(flags & PTHREAD_START_CUSTOM))
	{
		// We have to allocate stack ourselves
		unsigned long stacksize = (unsigned long) stack;
		unsigned long allocsize;

		// The pthread object is placed above stack area
		allocsize = stacksize + pthread_obj_size;
		allocsize = (allocsize + 4095) & ~4095; // round up to page size

		stack = (void**) sys_mmap(NULL, allocsize, PROT_READ | PROT_WRITE,
				MAP_ANON | MAP_PRIVATE, -1, 0);

		if (((intptr_t)stack) < 0 && ((intptr_t)stack) >= -4095)
			ret = errno_linux_to_bsd((int) stack);

		pthread = (void*) (((uintptr_t)stack) + stacksize);
		stack = (void**) (((uintptr_t)stack) + stacksize);
	}

	// TODO: call pthread_entry_point with the right args

#if defined(__x86_64__)
	__asm__ __volatile__ (
			"syscall\n"
			"testl %0, %0\n"
			"jne 1f\n"
			"call *%2\n"
			"int3\n"
			"1:\n"
			: "=a"(ret)
			: "0" (__NR_clone), "r"(thread_start),
			"D"(LINUX_CLONE_THREAD | LINUX_CLONE_VM | LINUX_CLONE_SIGHAND 
	            | LINUX_CLONE_FILES | LINUX_CLONE_FS | LINUX_CLONE_SYSVSEM),
			"S"(stack));
#elif defined(__i386__)
#	warning Missing clone call assembly!
#else
#	warning Missing clone call assembly!
#endif

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	return pthread;
}


