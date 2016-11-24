#include "psynch_cvwait.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include "../../../mach_server/client/lkm.h"
#include "../../../../lkm/api.h"
#include "../simple.h"

long sys_psynch_cvwait(void* cv, uint32_t cvgen, uint32_t cvugen, void* mutex, uint32_t mgen,
		uint32_t ugen, uint64_t sec, uint32_t usec)
{
	struct psynch_cvwait_args args = {
		.cv = cv,
		.cvgen = cvgen,
		.cvugen = cvugen,
		.mutex = mutex,
		.mgen = mgen,
		.ugen = ugen,
		.sec = sec,
		.usec = usec,
	};

	// __simple_printf("sys_psynch_mutexwait(mutex=%p, mgen=%x)\n", mutex, mgen);

	return lkm_call(NR_psynch_cvwait_trap, &args);
}

