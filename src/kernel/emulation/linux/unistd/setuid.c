#include "setuid.h"
#include "../base.h"
#include <darlingserver/rpc.h>
#include "../errno.h"
#include <stddef.h>

long sys_setuid(int uid)
{
	return errno_linux_to_bsd(dserver_rpc_uidgid(uid, -1, NULL, NULL));
}

