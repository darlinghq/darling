#include "seteuid.h"
#include "../base.h"
#include <darlingserver/rpc.h>
#include "../errno.h"
#include <stddef.h>
#include "getuid.h"

long sys_seteuid(int euid)
{
	return __setuidgid(euid, -1);
}

