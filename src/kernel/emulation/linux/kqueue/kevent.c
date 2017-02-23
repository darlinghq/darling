#include "kevent.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include <stddef.h>
#include "../../../../../platform-include/sys/errno.h"

int __attribute__((weak)) __attribute__((visibility("default"))) kevent_impl(int kq, ...) { return -ENOSYS; }

long sys_kevent(int	kq, const struct kevent	*changelist, int nchanges,
			struct	kevent *eventlist, int nevents,
			const struct timespec *timeout)
{
	return kevent_impl(kq, changelist, nchanges, eventlist, nevents, timeout);
}

