#include "getuid.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include <stddef.h>

extern char* getenv(const char* p);

long sys_getuid(void)
{
	int ret;
	
#ifndef VARIANT_DYLD // dyld cannot call getenv
	if (getenv("__FAKE_SETUID_ROOT") != NULL)
		return 0;
#endif

#ifdef __NR_getuid32
	ret = LINUX_SYSCALL0(__NR_getuid32);
#else
	ret = LINUX_SYSCALL0(__NR_getuid);
#endif
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

