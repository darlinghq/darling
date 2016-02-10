#include "psynch_mutexdrop.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include "../../../mach_server/client/lkm.h"
#include "../../../../lkm/api.h"
#include "../simple.h"

long sys_psynch_mutexdrop(void* mutex, uint32_t mgen, uint32_t ugen, uint64_t tid, uint32_t flags)
{
	struct psynch_mutexdrop_args args = {
		.mutex = (unsigned long) mutex,
		.mgen = mgen,
		.ugen = ugen,
		.tid = tid,
		.flags = flags
	};

	// __simple_printf("sys_psynch_mutexdrop(mutex=%p, mgen=%x)\n", mutex, mgen);

	return lkm_call(NR_psynch_mutexdrop_trap, &args);
}

