#include "setgid.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

int sys_setgid(int gid)
{
	int ret;

	ret = LINUX_SYSCALL1(__NR_setgid, gid);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

