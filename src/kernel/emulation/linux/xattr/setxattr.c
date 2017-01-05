#include "setxattr.h"
#include "../base.h"
#include "../errno.h"
#include "../../../../../platform-include/sys/errno.h"
#include <asm/unistd.h>

#define XATTR_NOFOLLOW	1

long sys_setxattr(const char* path, const char* name, const void* value,
		unsigned long size, unsigned int pos, int options)
{
	int ret;

	if (pos != 0)
		return -ERANGE;

	if (options & XATTR_NOFOLLOW)
	{
		ret = LINUX_SYSCALL(__NR_lsetxattr, path, name, value, size);
	}
	else
	{
		ret = LINUX_SYSCALL(__NR_setxattr, path, name, value, size);
	}

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	return ret;
}
