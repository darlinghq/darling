#include "seteuid.h"
#include "../base.h"
#include <darlingserver/rpc.h>
#include "../errno.h"
#include <stddef.h>

long sys_seteuid(int euid)
{
	return errno_linux_to_bsd(dserver_rpc_uidgid(euid, -1, NULL, NULL));
}

