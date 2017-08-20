#include "pthread_markcancel.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include <stddef.h>
#include "../mach/lkm.h"
#include "../../../../lkm/api.h"

long sys_pthread_markcancel(int thread_port)
{
	int ret = lkm_call(NR_pthread_markcancel, (void*)(long) thread_port);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

