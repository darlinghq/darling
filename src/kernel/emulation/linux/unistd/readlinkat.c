#include "readlink.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../common_at.h"

extern __SIZE_TYPE__ strlen(const char* str);

long sys_readlinkat(int fd, const char* path, char* buf, int count)
{
	int ret;

	ret = LINUX_SYSCALL(__NR_readlinkat, atfd(fd), path, buf, count);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}
