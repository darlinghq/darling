#include "pthread_kill.h"
#include "../base.h"
#include "../errno.h"
#include "../signal/duct_signals.h"
#include <sys/errno.h>
#include <stddef.h>
#include "../simple.h"

#include <darlingserver/rpc.h>

long sys_pthread_kill(int thread_port, int sig)
{
	int ret = dserver_rpc_pthread_kill(thread_port, signum_bsd_to_linux(sig));
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

