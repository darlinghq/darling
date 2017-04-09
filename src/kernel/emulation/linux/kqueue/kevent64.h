#ifndef LINUX_KEVENT64_H
#define LINUX_KEVENT64_H

struct kevent64;
struct timespec;

long sys_kevent64(int kq, const struct kevent64 *changelist, int nchanges,
			struct kevent64 *eventlist, int nevents, unsigned int flags,
			const struct timespec *timeout);

#endif

