#include "kevent.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include <stddef.h>
#include <sys/errno.h>
#include "../mach/lkm.h"
#include <lkm/api.h>

long sys_kevent(int	kq, const struct kevent	*changelist, int nchanges,
			struct	kevent *eventlist, int nevents,
			const struct timespec *timeout)
{
	struct kevent_args args = {
		.fd = kq,
		.changelist = changelist,
		.nchanges = nchanges,
		.eventlist = eventlist,
		.nevents = nevents,
		.timeout = timeout,
	};
	return lkm_call(NR_kevent, &args);
}

