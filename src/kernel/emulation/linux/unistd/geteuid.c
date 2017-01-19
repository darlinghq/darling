#include "geteuid.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include <stddef.h>

extern char* getenv(const char* p);

long sys_geteuid(void)
{
	int ret;

#ifndef VARIANT_DYLD
	if (getenv("__FAKE_SETUID_ROOT") != NULL)
		return 0;
#endif

#ifdef __NR_geteuid32
	ret = LINUX_SYSCALL0(__NR_geteuid32);
#else
	ret = LINUX_SYSCALL0(__NR_geteuid);
#endif
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

