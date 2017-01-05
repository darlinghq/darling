#include "access.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_access(const char* filename, int amode)
{
	int ret;

#ifdef __NR_access
	ret = LINUX_SYSCALL(__NR_access, filename, amode);
#else
	ret = LINUX_SYSCALL(__NR_faccessat, LINUX_AT_FDCWD, filename, amode, 0);
#endif

	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}
