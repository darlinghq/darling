#include "settid.h"
#include "../base.h"
#include <darlingserver/rpc.h>
#include "../errno.h"
#include <stddef.h>

long sys_settid(int uid, int gid)
{
	return errno_linux_to_bsd(dserver_rpc_uidgid(uid, gid, NULL, NULL));
}

