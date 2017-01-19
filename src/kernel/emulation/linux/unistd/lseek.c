#include "lseek.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>

long long sys_lseek(int fd, long long offset, int whence)
{
	long long ret;

#ifdef SYS__llseek
	long long roff;
	ret = LINUX_SYSCALL(__NR__llseek, fd, (unsigned int)offset >> 32,
			(unsigned int)offset, &roff, whence);

	if (ret >= 0)
		ret = roff;
#else
	ret = LINUX_SYSCALL(__NR_lseek, fd, offset, whence);
#endif
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

