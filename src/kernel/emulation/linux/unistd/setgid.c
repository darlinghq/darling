#include "setgid.h"
#include "../base.h"
#include "../mach/lkm.h"
#include "../../../../lkm/api.h"

long sys_setgid(int gid)
{
	return lkm_call(NR_setgid, (void*)(long)gid);
}

