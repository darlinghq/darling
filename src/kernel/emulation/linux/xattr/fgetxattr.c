#include "fgetxattr.h"
#include "../base.h"
#include "../errno.h"
#include "../../../../../platform-include/sys/errno.h"
#include <linux-syscalls/linux.h>

long sys_fgetxattr(int fd, const char* name, char* value,
		unsigned long size, unsigned int pos, int options)
{
	int ret;
	
	if (pos != 0)
		return -ERANGE;

	ret = LINUX_SYSCALL(__NR_fgetxattr, fd, name, value, size);

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	return ret;
}

