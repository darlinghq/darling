#include "fsetxattr.h"
#include "../base.h"
#include "../errno.h"
#include "../../../../../platform-include/sys/errno.h"
#include <asm/unistd.h>

long sys_fsetxattr(int fd, const char* name, const void* value,
		unsigned long size, unsigned int pos, int options)
{
	int ret;

	if (pos != 0)
		return -ERANGE;

	ret = LINUX_SYSCALL(__NR_fsetxattr, fd, name, value, size);

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	return 0;
}

