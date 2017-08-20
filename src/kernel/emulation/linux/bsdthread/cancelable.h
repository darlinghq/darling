#ifndef _BSDTHREAD_CANCELLABLE_H
#define _BSDTHREAD_CANCELLABLE_H
#include <sys/errno.h>
#include "pthread_canceled.h"

#define CANCELATION_POINT() \
	if (sys_pthread_canceled(0) == 0) \
		return -EINTR;

#endif

