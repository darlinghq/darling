#include "setuid.h"
#include "../base.h"
#include "../mach/lkm.h"
#include "../../../../lkm/api.h"

long sys_setuid(int uid)
{
	return lkm_call(NR_setuid, (void*)(long)uid);
}

