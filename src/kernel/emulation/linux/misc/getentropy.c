#include "getentropy.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>

#define LINUX_GRND_RANDOM 2

long sys_getentropy(void* buf, unsigned long size)
{
	int ret;

	ret = LINUX_SYSCALL(__NR_getrandom, buf, size, LINUX_GRND_RANDOM);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

