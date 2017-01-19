#include "lchown.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>

long sys_lchown(const char* path, int uid, int gid)
{
	int ret;

#ifdef __NR_chown32
	ret = LINUX_SYSCALL(__NR_lchown32, path, uid, gid);
#else
	ret = LINUX_SYSCALL(__NR_lchown, path, uid, gid);
#endif
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}
