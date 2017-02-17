#include "ulock_wait.h"
#include "ulock_wake.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../../../../../platform-include/sys/errno.h"
#include <stdbool.h>
#include <limits.h>

long sys_ulock_wake(uint32_t operation, void* addr, uint64_t wake_value)
{
	int ret, op;
	bool no_errno = operation & ULF_NO_ERRNO;

	op = operation & UL_OPCODE_MASK;
	if (op == UL_COMPARE_AND_WAIT)
	{
		int value;

		if (operation & ULF_WAKE_ALL)
			value = INT_MAX;
		else if (operation & ULF_WAKE_THREAD)
			value = INT_MAX; // incorrect
		else
			value = 1;

		ret = LINUX_SYSCALL(__NR_futex, addr, FUTEX_WAKE | FUTEX_PRIVATE_FLAG,
			value);
	}
	else if (op == UL_UNFAIR_LOCK)
	{
		ret = LINUX_SYSCALL(__NR_futex, addr, FUTEX_UNLOCK_PI | FUTEX_PRIVATE_FLAG);
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

