#include "settid.h"
#include "../base.h"
#include "../mach/lkm.h"
#include "../../../../external/lkm/api.h"

long sys_settid(int uid, int gid)
{
	struct uidgid ug = {
		.uid = uid,
		.gid = gid
	};

	return lkm_call(NR_setuidgid, &ug);
}

