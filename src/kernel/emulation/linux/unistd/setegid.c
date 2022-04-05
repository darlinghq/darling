#include "setegid.h"
#include "../base.h"
#include <darlingserver/rpc.h>
#include "../errno.h"
#include <stddef.h>
#include "getuid.h"

long sys_setegid(int egid)
{
	return __setuidgid(-1, egid);
}

