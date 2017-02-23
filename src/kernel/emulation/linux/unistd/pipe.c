#include "pipe.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>

long sys_pipe(int fd[2])
{
	int ret;

	ret = LINUX_SYSCALL(__NR_pipe, fd);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

#if defined(__i386__) || defined(__x86_64__)
	__asm__ __volatile__("movl %0, %%edx" :: "m"(fd[1]) : "edx");
#else
#	warning Missing assembly!
#endif
	return fd[0];
}

