#include "geteuid.h"
#include "../base.h"
#include <stddef.h>
#include "../mach/lkm.h"
#include "../../../../external/lkm/api.h"

long sys_geteuid(void)
{
	struct uidgid ug;
	lkm_call(NR_getuidgid, &ug);
	return ug.uid;
}

