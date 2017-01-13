#include "bsdthread_terminate.h"
#include "bsdthread_register.h"
#include "../base.h"
#include "../errno.h"
#include "../../../../../platform-include/sys/errno.h"
#include <asm/unistd.h>
#include <stddef.h>
#include <stdint.h>

int bsdthread_terminate_trap(
                uintptr_t stackaddr,
                unsigned long freesize,
                int thread,
                int sem);
int semaphore_signal_trap(int signal_name);

long sys_bsdthread_terminate(void* stackaddr, unsigned long freesize, int port,
		int join_sem)
{
#ifndef VARIANT_DYLD
#ifdef BSDTHREAD_WRAP_LINUX_PTHREAD
	// Implemented in libdyld
	extern int __darling_thread_terminate(void* stackaddr,
			unsigned long freesize, unsigned long pthobj_size);

	semaphore_signal_trap(join_sem);

	return __darling_thread_terminate(stackaddr, freesize, pthread_obj_size);
#else
	return bsdthread_terminate_trap((uintptr_t) stackaddr, freesize,
			port, join_sem);
#endif
#else
	return -ENOSYS;
#endif
}


