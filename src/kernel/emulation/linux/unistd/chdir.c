#include "chdir.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_chdir(const char* path)
{
	int ret;

	// TODO: case translation

	ret = LINUX_SYSCALL(__NR_chdir, path);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

