#include "chmod.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_chmod(const char* path, int mode)
{
	int ret;

	// TODO: case translation

	ret = LINUX_SYSCALL(__NR_chmod, path, mode);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

