#include "ulock_wait.h"
#include "../base.h"
#include "../errno.h"
#include "../duct_errno.h"
#include <linux-syscalls/linux.h>
#include <sys/errno.h>
#include <stdbool.h>
#include <stddef.h>

struct timespec
{
	long tv_sec;
	long tv_nsec;
};

// timeout is in us
long sys_ulock_wait(uint32_t operation, void* addr, uint64_t value, uint32_t timeout)
{
	int ret, op;
	struct timespec ts;
	bool no_errno = operation & ULF_NO_ERRNO;


	// char dbg[100];
	// __simple_sprintf(dbg, "ulock_wait on %p for %d ns", addr, timeout / 1000);
	// lkm_call(0x1028, dbg);

	if (timeout > 0)
	{
		ts.tv_sec = timeout / 1000*1000;
		ts.tv_nsec = (timeout % (1000*1000)) * 1000;
	}

	op = operation & UL_OPCODE_MASK;
	if (op == UL_COMPARE_AND_WAIT || op == UL_UNFAIR_LOCK)
	{
		ret = LINUX_SYSCALL(__NR_futex, addr, FUTEX_WAIT | FUTEX_PRIVATE_FLAG,
			value, (timeout != 0) ? & ts : NULL);

		// unlike ulock_wait(), futex(FUTEX_WAIT) does not return how many
		// other threads are now (still) waiting for the lock.
		//
		// This hack makes userspace believe that there are other pending threads
		// and always take the slow path, which is the safe thing to do if
		// we are unsure.
		if (ret == 0 || ret == -LINUX_EAGAIN)
			ret = 1;
	}
	else
		return no_errno ? -(EINVAL | 0x800) : -EINVAL;

	if (ret < 0)
	{
		ret = errno_linux_to_bsd(ret);
		if (no_errno)
			ret &= ~0x800;
	}

	return ret;
}

