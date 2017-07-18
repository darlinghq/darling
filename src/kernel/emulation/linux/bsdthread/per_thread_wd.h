#ifndef _PER_THREAD_WD_H
#define _PER_THREAD_WD_H
#include <pthread/tsd_private.h>
#include "../common_at.h"

// We use the following static TSD keys (see tsd_private.h):
// 100 = is 101 set? (Because 0 could be a valid FD, theoretically.)
// 101 = per thread wd file descriptor
//
// NOTE: The file descriptor in 101 will get leaked unless __pthread_fchdir(-1)
// is called, but this is such a rare scenario that we disregard it.
// It is be too hard to fix, because static TSD keys (such as the ones we use)
// don't have destructors.

static inline int get_perthread_wd(void)
{
	if (_pthread_getspecific_direct(100))
		return _pthread_getspecific_direct(101);
	else
		return LINUX_AT_FDCWD;
}

static inline void set_perthread_wd(int fd)
{
	_pthread_setspecific_direct(100, 1);
	_pthread_setspecific_direct(101, fd);
}

#endif

