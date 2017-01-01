#include "chroot.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_chroot(const char* path)
{
	int ret;

	ret = LINUX_SYSCALL(__NR_chroot, path);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}
