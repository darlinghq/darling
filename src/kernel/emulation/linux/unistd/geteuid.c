#include "geteuid.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_geteuid(void)
{
	int ret;

#ifdef __NR_geteuid32
	ret = LINUX_SYSCALL0(__NR_geteuid32);
#else
	ret = LINUX_SYSCALL0(__NR_geteuid);
#endif
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

