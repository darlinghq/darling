#include "pselect.h"
#include "../base.h"
#include "../errno.h"
#include <stddef.h>
#include <linux-syscalls/linux.h>

long sys_pselect(int nfds, void* rfds, void* wfds, void* efds, struct bsd_timeval* timeout, const sigset_t* mask)
{
	return sys_pselect_nocancel(nfds, rfds, wfds, efds, timeout, mask);
}

long sys_pselect_nocancel(int nfds, void* rfds, void* wfds, void* efds, struct bsd_timeval* timeout, const sigset_t* mask)
{
	int ret;
	struct linux_timeval ltv;
	long data[2];

	if (timeout != NULL)
	{
		ltv.tv_sec = timeout->tv_sec;
		ltv.tv_usec = timeout->tv_usec;
	}
	if (mask != NULL)
	{
		linux_sigset_t lmask;

		sigset_bsd_to_linux(mask, &lmask);

		data[0] = (long)lmask;
		data[1] = 65/8; // _NSIG / 8
	}

	ret = LINUX_SYSCALL(__NR_pselect6, nfds, rfds, wfds, efds,
			(timeout != NULL) ? &ltv : NULL,
			(mask != NULL) ? data : NULL);

	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

