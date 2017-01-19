#include "chdir.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>

long sys_chdir(const char* path)
{
	int ret;
	ret = LINUX_SYSCALL(__NR_chdir, path);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}
