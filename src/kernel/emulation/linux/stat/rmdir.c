#include "rmdir.h"
#include "common.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_rmdir(const char* path)
{
	int ret;

	ret = LINUX_SYSCALL(__NR_rmdir, path);

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	return 0;
}
