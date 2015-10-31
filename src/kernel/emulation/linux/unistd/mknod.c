#include "mknod.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_mknod(const char* path, int mode, int dev)
{
	int ret;

	// TODO: case translation

	ret = LINUX_SYSCALL(__NR_mknod, path, mode, dev);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

