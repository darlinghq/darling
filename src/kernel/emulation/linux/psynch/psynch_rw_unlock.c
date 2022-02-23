#include "psynch_rw_unlock.h"
#include "../base.h"
#include <linux-syscalls/linux.h>
#include <darlingserver/rpc.h>
#include "../simple.h"

long sys_psynch_rw_unlock(void* rwlock, uint32_t lgenval, uint32_t ugenval, uint32_t rw_wc, int flags)
{
	uint32_t retval;
	int ret = dserver_rpc_psynch_rw_unlock(rwlock, lgenval, ugenval, rw_wc, flags, &retval);

	if (ret < 0) {
		__simple_printf("psynch_rw_unlock failed internally: %d", ret);
		__simple_abort();
	}

	return (ret) ? ret : retval;
}

