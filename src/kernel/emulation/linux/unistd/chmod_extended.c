#include "chmod_extended.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_chmod_extended(const char* path, int uid, int gid, int mode, void* xsec)
{
	int ret;

	ret = LINUX_SYSCALL(__NR_chmod, path, mode);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	ret = LINUX_SYSCALL(__NR_chown, path, uid, gid);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	return ret;
}
