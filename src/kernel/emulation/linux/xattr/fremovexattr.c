#include "fremovexattr.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_fremovexattr(int fd, const char* name, int options)
{
	int ret;

	// TODO: handle case conversion

	ret = LINUX_SYSCALL(__NR_fremovexattr, fd, name);

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	return 0;
}

