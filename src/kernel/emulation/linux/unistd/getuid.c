#include "getuid.h"
#include "../base.h"
#include <stddef.h>
#include <darlingserver/rpc.h>
#include "../simple.h"

long sys_getuid(void)
{
	int32_t uid;
	if (dserver_rpc_uidgid(-1, -1, &uid, NULL) < 0) {
		__simple_abort();
	}
	return uid;
}

