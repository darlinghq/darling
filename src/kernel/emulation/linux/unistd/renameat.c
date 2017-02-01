#include "renameat.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../../../../../platform-include/sys/errno.h"
#include "../common_at.h"

extern char* strcpy(char* dst, const char* src);

long sys_renameat(int oldfd, const char* oldpath, int newfd, const char* newpath)
{
	int ret;

	ret = LINUX_SYSCALL(__NR_renameat, atfd(oldfd), oldpath, atfd(newfd), newpath);

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	return 0;
}
