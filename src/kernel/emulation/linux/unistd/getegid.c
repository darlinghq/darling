#include "getegid.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>

long sys_getegid(void)
{
	int ret;

#ifdef __NR_getegid32
	ret = LINUX_SYSCALL0(__NR_getegid32);
#else
	ret = LINUX_SYSCALL0(__NR_getegid);
#endif
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

