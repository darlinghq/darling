#include "psynch_cvbroad.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../mach/lkm.h"
#include "../../../../lkm/api.h"
#include "../simple.h"

long sys_psynch_cvbroad(void* cv, uint32_t cvgen, uint32_t diffgen, void* mutex, uint32_t mgen,
		uint32_t ugen, uint64_t tid, uint32_t flags)
{
	struct psynch_cvbroad_args args = {
		.cv = cv,
		.cvgen = cvgen,
		.diffgen = diffgen,
		.mutex = mutex,
		.mgen = mgen,
		.ugen = ugen,
		.tid = tid,
		.flags = flags,
	};

	// __simple_printf("sys_psynch_mutexwait(mutex=%p, mgen=%x)\n", mutex, mgen);

	return lkm_call(NR_psynch_cvbroad_trap, &args);
}

