#include "rename.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../../../../../platform-include/sys/errno.h"

extern char* strcpy(char* dst, const char* src);

long sys_rename(const char* oldpath, const char* newpath)
{
	int ret;

	ret = LINUX_SYSCALL(__NR_rename, oldpath, newpath);

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	return 0;
}
