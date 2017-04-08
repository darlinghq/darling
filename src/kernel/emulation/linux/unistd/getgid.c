#include "getgid.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include <stddef.h>
#include <stdlib.h>

long sys_getgid(void)
{
	int ret;
#ifndef VARIANT_DYLD // dyld cannot call getenv
	if (getenv("__FAKE_SETGID_ROOT") != NULL)
		return 0;
#endif

#ifdef __NR_getgid32
	ret = LINUX_SYSCALL0(__NR_getgid32);
#else
	ret = LINUX_SYSCALL0(__NR_getgid);
#endif
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

