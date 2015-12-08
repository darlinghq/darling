#include "setgroups.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_setgroups(unsigned int size, int* gidset)
{
	int ret;

#ifdef __NR_setgroups32
	ret = LINUX_SYSCALL(__NR_setgroups32, size, gidset);
#else
	ret = LINUX_SYSCALL(__NR_setgroups, size, gidset);
#endif
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

