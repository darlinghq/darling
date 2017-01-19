#include "fchmod_extended.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>

long sys_fchmod_extended(int fd, int uid, int gid, int mode, void* xsec)
{
	int ret;

	ret = LINUX_SYSCALL2(__NR_fchmod, fd, mode);
	if (ret < 0)
		return errno_linux_to_bsd(ret);
	
	ret = LINUX_SYSCALL(__NR_fchown, fd, uid, gid);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	return ret;
}

