#include "rename.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_rename(const char* oldpath, const char* newpath)
{
	int ret;

	// TODO: handle case conversion

	ret = LINUX_SYSCALL(__NR_rename, oldpath, newpath);

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	return 0;
}

