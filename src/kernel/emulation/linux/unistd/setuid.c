#include "setuid.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_setuid(int uid)
{
	int ret;

#ifdef __NR_setuid32
	ret = LINUX_SYSCALL1(__NR_setuid32, uid);
#else
	ret = LINUX_SYSCALL1(__NR_setuid, uid);
#endif
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

