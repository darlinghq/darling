#include "rmdir.h"
#include "common.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../bsdthread/per_thread_wd.h"

#define LINUX_AT_REMOVEDIR	0x200

long sys_rmdir(const char* path)
{
	int ret;

	ret = LINUX_SYSCALL(__NR_unlinkat, get_perthread_wd(), path, LINUX_AT_REMOVEDIR);

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	return 0;
}
