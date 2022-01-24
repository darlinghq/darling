#include "gettid.h"
#include "../base.h"
#include <stddef.h>
#include <darlingserver/rpc.h>
#include "../simple.h"

long sys_gettid(int* uid, int* gid)
{
	if (dserver_rpc_uidgid(-1, -1, uid, gid) < 0) {
		__simple_abort();
	}

	return 0;
}

