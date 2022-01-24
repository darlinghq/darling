#include "setgid.h"
#include "../base.h"
#include <darlingserver/rpc.h>
#include "../errno.h"
#include <stddef.h>

long sys_setgid(int gid)
{
	return errno_linux_to_bsd(dserver_rpc_uidgid(-1, gid, NULL, NULL));
}

