#include "pthread_markcancel.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include <stddef.h>

#include <darlingserver/rpc.h>

long sys_pthread_markcancel(unsigned int thread_port)
{
	int ret = dserver_rpc_pthread_markcancel(thread_port);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

