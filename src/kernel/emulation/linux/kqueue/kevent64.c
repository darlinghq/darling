#include "kevent.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include <stddef.h>
#include "../../../../../platform-include/sys/errno.h"

int __attribute__((weak)) __attribute__((visibility("default"))) kevent64_impl(int kq, ...) { return -ENOSYS; }

long sys_kevent64(int kq, const struct kevent64 *changelist, int nchanges,
			struct kevent64 *eventlist, int nevents, unsigned int flags,
			const struct timespec *timeout);
{
	return kevent64_impl(kq, changelist, nchanges, eventlist, nevents, flags, timeout);
}

