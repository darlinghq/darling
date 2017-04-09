#include "guarded_kqueue_np.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../kqueue/kqueue.h"

long sys_guarded_kqueue_np(uint64_t* guardid, unsigned guardflags)
{
	*guardid = 0;
	return sys_kqueue();
}

