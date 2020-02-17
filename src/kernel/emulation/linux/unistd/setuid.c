#include "setuid.h"
#include "../base.h"
#include "../mach/lkm.h"
#include "../../../../external/lkm/api.h"

long sys_setuid(int uid)
{
	struct uidgid ug = {
		.uid = uid,
		.gid = -1
	};

	return lkm_call(NR_setuidgid, &ug);
}

