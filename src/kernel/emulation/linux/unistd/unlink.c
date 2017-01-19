#include "unlink.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>

long sys_unlink(const char* path)
{
	int ret;

	ret = LINUX_SYSCALL(__NR_unlink, path);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}
