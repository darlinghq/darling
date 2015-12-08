#include "getgroups.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_getgroups(unsigned int size, int* gidset)
{
	int ret;

#ifdef __NR_getgroups32
	ret = LINUX_SYSCALL(__NR_getgroups32, size, gidset);
#else
	ret = LINUX_SYSCALL(__NR_getgroups, size, gidset);
#endif
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

