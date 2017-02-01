#include "faccessat.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../common_at.h"

long sys_faccessat(int fd, const char* filename, int amode, int flag)
{
	int ret;

	ret = LINUX_SYSCALL(__NR_faccessat, atfd(fd), filename, amode, atflags_bsd_to_linux(flag));

	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}
