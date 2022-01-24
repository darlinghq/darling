#include "getegid.h"
#include "../base.h"
#include <stddef.h>
#include <darlingserver/rpc.h>
#include "../simple.h"

long sys_getegid(void)
{
	int32_t gid;
	if (dserver_rpc_uidgid(-1, -1, NULL, &gid) < 0) {
		__simple_abort();
	}
	return gid;
}

