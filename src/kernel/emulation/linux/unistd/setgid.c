#include "setgid.h"
#include "../base.h"
#include <darlingserver/rpc.h>
#include "../errno.h"
#include <stddef.h>
#include "getuid.h"

long sys_setgid(int gid)
{
	return __setuidgid(-1, gid);
}

