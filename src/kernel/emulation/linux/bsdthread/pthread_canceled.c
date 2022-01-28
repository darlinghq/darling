#include "pthread_canceled.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include <stddef.h>
#include "bsdthread_create.h"
#include <sys/errno.h>

#include <darlingserver/rpc.h>

long sys_pthread_canceled(int action)
{
	if (action == 0 && !uses_threads())
		return -EINVAL;

	int ret = dserver_rpc_pthread_canceled(action);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

