#include "symlinkat.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../../../../../platform-include/sys/errno.h"
#include "../common_at.h"

extern char* strcpy(char* dst, const char* src);

long sys_symlinkat(const char* path, int fd, const char* link)
{
	int ret;

	ret = LINUX_SYSCALL(__NR_symlinkat, path, atfd(fd), link);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}
