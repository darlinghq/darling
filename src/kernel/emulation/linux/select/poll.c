#include "poll.h"
#include "../base.h"
#include "../errno.h"
#include <stddef.h>
#include <linux-syscalls/linux.h>
#include "../bsdthread/cancelable.h"

struct ppoll_timespec {
    long    tv_sec;         /* seconds */
    long    tv_nsec;        /* nanoseconds */
};

long sys_poll(struct pollfd* fds, unsigned int nfds, int timeout)
{
	CANCELATION_POINT();
	return sys_poll_nocancel(fds, nfds, timeout);
}

long sys_poll_nocancel(struct pollfd* fds, unsigned int nfds, int timeout)
{
	int ret;

	#if defined(__NR_poll)
		ret = LINUX_SYSCALL(__NR_poll, fds, nfds, timeout);
	#else
		struct ppoll_timespec timeout_ts = {
			.tv_sec = (timeout % 1000) * 1000000,
			.tv_nsec = timeout / 1000
		};
		
		ret = LINUX_SYSCALL(__NR_ppoll, fds, nfds, (timeout < 0) ? NULL : &timeout_ts, NULL);
	#endif
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

