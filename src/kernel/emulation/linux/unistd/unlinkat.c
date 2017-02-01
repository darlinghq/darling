#include "unlinkat.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../common_at.h"

long sys_unlinkat(int fd, const char* path, int flag)
{
	int ret;

	ret = LINUX_SYSCALL(__NR_unlinkat, atfd(fd), path, atflags_bsd_to_linux(flag));
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}
