#include "setuid.h"
#include "../base.h"
#include <darlingserver/rpc.h>
#include "../errno.h"
#include <stddef.h>
#include "getuid.h"

long sys_setuid(int uid)
{
	return __setuidgid(uid, -1);
}

