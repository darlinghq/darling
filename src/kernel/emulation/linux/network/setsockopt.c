#include "setsockopt.h"
#include "getsockopt.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include <stddef.h>
#include <sys/errno.h>
#include "../time/gettimeofday.h"
#include "duct.h"

#define LINGER_TICKS_PER_SEC	100 // Is this the right number of ticks per sec?

long sys_setsockopt(int fd, int level, int optname, const void* optval, int optlen)
{
	int ret, linux_level, linux_optname;
	char buf[100];
	
	linux_level = level;
	linux_optname = optname;
	
	ret = sockopt_bsd_to_linux(&linux_level, &linux_optname,
			(void**) &optval, buf);
	
	if (ret != 0 || !linux_optname)
		return ret;

	if (level == BSD_SOL_SOCKET)
	{
		if (optname == BSD_SO_RCVTIMEO)
		{
			const struct bsd_timeval* tv = (const struct bsd_timeval*) optval;
			struct linux_timeval* ltv = (struct linux_timeval*) __builtin_alloca(sizeof(struct linux_timeval));

			ltv->tv_sec = tv->tv_sec;
			ltv->tv_usec = tv->tv_usec;

			optval = ltv;
			optlen = sizeof(*ltv);
		}
	}

#ifdef __NR_socketcall
	ret = LINUX_SYSCALL(__NR_socketcall, LINUX_SYS_SETSOCKOPT,
			((long[6]) { fd, linux_level, linux_optname, optval, optlen, 0 }));
#else
	ret = LINUX_SYSCALL(__NR_setsockopt, fd, linux_level, linux_optname,
			optval, optlen);
#endif

	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}
