#include "mkdir.h"
#include "common.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>

long sys_mkdir(const char* path, unsigned int mode)
{
	int ret;

	ret = LINUX_SYSCALL(__NR_mkdir, path, mode);

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	return 0;
}
