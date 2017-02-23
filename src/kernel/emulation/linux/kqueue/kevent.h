#ifndef LINUX_KEVENT_H
#define LINUX_KEVENT_H

struct kevent;
struct timespec;

long sys_kevent(int	kq, const struct kevent	*changelist, int nchanges,
			struct	kevent *eventlist, int nevents,
			const struct timespec *timeout);

#endif

