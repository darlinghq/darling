#include "geteuid.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

int sys_geteuid(void)
{
	int ret;

	ret = LINUX_SYSCALL0(__NR_geteuid);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

