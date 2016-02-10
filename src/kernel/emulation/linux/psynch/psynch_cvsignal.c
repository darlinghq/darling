#include "psynch_cvsignal.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include "../../../mach_server/client/lkm.h"
#include "../../../../lkm/api.h"
#include "../simple.h"

long sys_psynch_cvsignal(void* cv, uint32_t cvgen, uint32_t cvugen, void* mutex, uint32_t mgen,
		uint32_t ugen, int thread_port, uint32_t flags)
{
	struct psynch_cvsignal_args args = {
		.cv = cv,
		.cvgen = cvgen,
		.cvugen = cvugen,
		.mutex = mutex,
		.mgen = mgen,
		.ugen = ugen,
		.thread_port = thread_port,
		.flags = flags,
	};

	// __simple_printf("sys_psynch_mutexwait(mutex=%p, mgen=%x)\n", mutex, mgen);

	return lkm_call(NR_psynch_cvsignal_trap, &args);
}

