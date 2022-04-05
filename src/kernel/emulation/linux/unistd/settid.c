#include "settid.h"
#include "../base.h"
#include <darlingserver/rpc.h>
#include "../errno.h"
#include <stddef.h>
#include "getuid.h"

long sys_settid(int uid, int gid)
{
	return __setuidgid(uid, gid);
}

