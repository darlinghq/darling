#include "getegid.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_getegid(void)
{
	int ret;

	ret = LINUX_SYSCALL0(__NR_getegid);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

