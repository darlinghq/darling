#include "getpgrp.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>

long sys_getpgrp(void)
{
	#if defined(__NR_getpgrp)
		int ret;

		ret = LINUX_SYSCALL0(__NR_getpgrp);
		if (ret < 0)
			ret = errno_linux_to_bsd(ret);

		return ret;
	#else
		return sys_getpgid(0);
	#endif
}

