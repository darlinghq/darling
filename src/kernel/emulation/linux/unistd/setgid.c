#include "setgid.h"
#include "../base.h"
#include "../mach/lkm.h"
#include "../../../../external/lkm/api.h"

long sys_setgid(int gid)
{
	struct uidgid ug = {
		.uid = -1,
		.gid = gid
	};

	return lkm_call(NR_setuidgid, &ug);
}

