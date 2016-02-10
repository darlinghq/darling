#include "fchown.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_fchown(int fd, int uid, int gid)
{
	int ret;

#ifdef __NR_fchown32
	ret = LINUX_SYSCALL3(__NR_fchown32, fd, uid, gid);
#else
	ret = LINUX_SYSCALL3(__NR_fchown, fd, uid, gid);
#endif
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

