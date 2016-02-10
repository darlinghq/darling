#include "select.h"
#include "../base.h"
#include "../errno.h"
#include <stddef.h>
#include <asm/unistd.h>

long sys_select(int nfds, void* rfds, void* wfds, void* efds, struct bsd_timeval* timeout)
{
	return sys_select_nocancel(nfds, rfds, wfds, efds, timeout);
}

long sys_select_nocancel(int nfds, void* rfds, void* wfds, void* efds, struct bsd_timeval* timeout)
{
	int ret;
	struct linux_timeval ltv;

	if (timeout != NULL)
	{
		ltv.tv_sec = timeout->tv_sec;
		ltv.tv_usec = timeout->tv_usec;
	}

#ifdef __NR__newselect
	ret = LINUX_SYSCALL(__NR__newselect, nfds, rfds, wfds, efds,
			(timeout != NULL) ? &ltv : NULL);
#else
	ret = LINUX_SYSCALL(__NR_select, nfds, rfds, wfds, efds,
			(timeout != NULL) ? &ltv : NULL);
#endif
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

