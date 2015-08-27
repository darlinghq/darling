#include "setuid.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

int sys_setuid(int uid)
{
	int ret;

	ret = LINUX_SYSCALL1(__NR_setuid, uid);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

