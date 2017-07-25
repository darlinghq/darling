#include "setegid.h"
#include "../base.h"
#include "../mach/lkm.h"
#include "../../../../lkm/api.h"

long sys_setegid(int egid)
{
	return lkm_call(NR_setgid, (void*)(long)egid);
}

