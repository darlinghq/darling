#ifndef LINUX_KEVENT_QOS_H
#define LINUX_KEVENT_QOS_H
#define PRIVATE 1
#include <sys/event.h>

long sys_kevent_qos(int	kq, const struct kevent_qos_s *changelist, int nchanges,
			struct	kevent_qos_s *eventlist, int nevents,
			void* data_out, unsigned long* data_available, unsigned int flags);

#endif

