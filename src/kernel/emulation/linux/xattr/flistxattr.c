#include "flistxattr.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_flistxattr(int fd, char* namebuf, unsigned long size, int options)
{
	int ret;

	// TODO: handle case conversion

	ret = LINUX_SYSCALL(__NR_flistxattr, fd, namebuf, size);

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	return 0;
}

