#include "truncate.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_truncate(const char* path, long long length)
{
	int ret;

	// TODO: translate path

#ifdef __NR_truncate64
	ret = LINUX_SYSCALL(__NR_truncate64, path, LL_ARG(length));
#else
	ret = LINUX_SYSCALL(__NR_truncate, path, LL_ARG(length));
#endif
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

