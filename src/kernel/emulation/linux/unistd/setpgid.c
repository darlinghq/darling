#include "setpgid.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_setpgid(int pid, int pgid)
{
	int ret;

	ret = LINUX_SYSCALL(__NR_setpgid, pid, pgid);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

