#ifndef LINUX_KEVENT64_H
#define LINUX_KEVENT64_H

struct kevent64_s;
struct timespec;

long sys_kevent64(int kq, const struct kevent64_s *changelist, int nchanges,
			struct kevent64_s *eventlist, int nevents, unsigned int flags,
			const struct timespec *timeout);

#endif

