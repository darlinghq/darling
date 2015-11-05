#include "bsdthread_terminate.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include <stddef.h>
#include <stdint.h>

long sys_bsdthread_terminate(void* stackaddr, unsigned long freesize, int port,
		int join_sem)
{
	long ret;

	ret = LINUX_SYSCALL(__NR_exit, 0);
	if (ret < 0)
		return errno_linux_to_bsd(ret);
	else
	{
		// TODO: signal join_sem
	}

	return ret;
}


