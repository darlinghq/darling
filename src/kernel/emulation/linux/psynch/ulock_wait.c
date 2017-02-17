#include "ulock_wait.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../../../../../platform-include/sys/errno.h"
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

	if (timeout > 0)
	{
		ts.tv_sec = timeout / 1000*1000;
		ts.tv_nsec = (timeout % (1000*1000)) * 1000;
	}

	op = operation & UL_OPCODE_MASK;
	if (op == UL_COMPARE_AND_WAIT)
	{
		ret = LINUX_SYSCALL(__NR_futex, addr, FUTEX_WAIT | FUTEX_PRIVATE_FLAG,
			value, (timeout != 0) ? & ts : NULL);
	}
	else if (op == UL_UNFAIR_LOCK)
	{
		// FIXME: This is not correct. Linux sets addr to TID, while libplatform expects Mach thread port
		ret = LINUX_SYSCALL(__NR_futex, addr, FUTEX_LOCK_PI | FUTEX_PRIVATE_FLAG,
			value, (timeout != 0) ? & ts : NULL);
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

