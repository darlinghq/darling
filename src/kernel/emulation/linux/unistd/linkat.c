#include "link.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../../../../../platform-include/sys/errno.h"
#include "../common_at.h"

extern char* strcpy(char* dst, const char* src);

long sys_linkat(int fd, const char* path, int fdlink, const char* link, int flag)
{
	int ret;

	ret = LINUX_SYSCALL(__NR_linkat, atfd(fd), path, atfd(fdlink), link, atflags_bsd_to_linux(flag));
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}
