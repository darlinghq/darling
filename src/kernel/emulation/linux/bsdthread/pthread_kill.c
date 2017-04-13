#include "pthread_kill.h"
#include "../base.h"
#include "../errno.h"
#include "../signal/duct_signals.h"
#include "../../../../../platform-include/sys/errno.h"
#include <stddef.h>
#include "../mach/lkm.h"
#include "../../../../lkm/api.h"
#include "../simple.h"

long sys_pthread_kill(int thread_port, int sig)
{
	struct pthread_kill_args args;

	args.thread_port = thread_port;
	args.sig = signum_bsd_to_linux(sig);

	return lkm_call(NR_pthread_kill_trap, &args);

}

