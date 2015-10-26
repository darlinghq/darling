#include "readlink.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_readlink(const char* path, char* buf, int count)
{
	int ret;

	// TODO: case translation

	ret = LINUX_SYSCALL(__NR_readlink, path, buf, count);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

