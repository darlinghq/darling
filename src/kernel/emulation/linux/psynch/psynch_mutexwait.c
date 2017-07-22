#include "psynch_mutexwait.h"
#include "../base.h"
#include "psynch_errno.h"
#include <linux-syscalls/linux.h>
#include "../mach/lkm.h"
#include "../../../../lkm/api.h"
#include "../simple.h"

long sys_psynch_mutexwait(void* mutex, uint32_t mgen, uint32_t ugen, uint64_t tid, uint32_t flags)
{
	struct psynch_mutexwait_args args = {
		.mutex = (unsigned long) mutex,
		.mgen = mgen,
		.ugen = ugen,
		.tid = tid,
		.flags = flags
	};

	// __simple_printf("sys_psynch_mutexwait(mutex=%p, mgen=%x)\n", mutex, mgen);

	int rv = lkm_call(NR_psynch_mutexwait_trap, &args);
	return psynch_errno(rv);
}

