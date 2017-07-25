#include "getegid.h"
#include "../base.h"
#include <stddef.h>
#include "../mach/lkm.h"
#include "../../../../lkm/api.h"

long sys_getegid(void)
{
	return lkm_call(NR_getgid, NULL);
}

