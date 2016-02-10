#include "gettimeofday.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_gettimeofday(struct bsd_timeval* tv, struct timezone* tz)
{
	int ret;
	struct linux_timeval ltv;

	ret = LINUX_SYSCALL(__NR_gettimeofday, &ltv, tz);
	if (ret < 0)
	{
		ret = errno_linux_to_bsd(ret);
	}
	else
	{
		tv->tv_sec = ltv.tv_sec;
		tv->tv_usec = ltv.tv_usec;
	}

	return ret;
}

