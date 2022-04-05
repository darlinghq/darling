#include "gettid.h"
#include "../base.h"
#include <stddef.h>
#include <darlingserver/rpc.h>
#include "../simple.h"
#include "getuid.h"

long sys_gettid(int* uid, int* gid)
{
	__getuidgid(uid, gid);
	return 0;
}

