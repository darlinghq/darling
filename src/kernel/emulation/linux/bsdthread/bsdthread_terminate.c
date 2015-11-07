#include "bsdthread_terminate.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include <stddef.h>
#include <stdint.h>

int bsdthread_terminate_trap(
                uintptr_t stackaddr,
                unsigned long freesize,
                int thread,
                int sem);

long sys_bsdthread_terminate(void* stackaddr, unsigned long freesize, int port,
		int join_sem)
{
	return bsdthread_terminate_trap((uintptr_t) stackaddr, freesize,
			port, join_sem);
}


