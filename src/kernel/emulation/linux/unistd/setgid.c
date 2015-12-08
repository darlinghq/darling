#include "setgid.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_setgid(int gid)
{
	int ret;

#ifdef __NR_setgid32
	ret = LINUX_SYSCALL1(__NR_setgid32, gid);
#else
	ret = LINUX_SYSCALL1(__NR_setgid, gid);
#endif
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

