#include "getgid.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_getgid(void)
{
	int ret;

#ifdef __NR_getgid32
	ret = LINUX_SYSCALL0(__NR_getgid32);
#else
	ret = LINUX_SYSCALL0(__NR_getgid);
#endif
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

