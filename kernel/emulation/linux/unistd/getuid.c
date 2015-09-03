#include "getuid.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_getuid(void)
{
	int ret;

	ret = LINUX_SYSCALL0(__NR_getuid);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

