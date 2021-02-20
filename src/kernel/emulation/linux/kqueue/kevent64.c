#include "kevent64.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include <stddef.h>
#include <sys/errno.h>
#include "../mach/lkm.h"
#include <lkm/api.h>

long sys_kevent64(int kq, const struct kevent64_s *changelist, int nchanges,
			struct kevent64_s *eventlist, int nevents, unsigned int flags,
			const struct timespec *timeout)
{
	struct kevent64_args args = {
		.fd = kq,
		.changelist = changelist,
		.nchanges = nchanges,
		.eventlist = eventlist,
		.nevents = nevents,
		.flags = flags,
		.timeout = timeout,
	};
	return lkm_call(NR_kevent64, &args);
}

