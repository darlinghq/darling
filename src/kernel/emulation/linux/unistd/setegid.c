#include "setegid.h"
#include "../base.h"
#include <darlingserver/rpc.h>
#include "../errno.h"
#include <stddef.h>

long sys_setegid(int egid)
{
	return errno_linux_to_bsd(dserver_rpc_uidgid(-1, egid, NULL, NULL));
}

