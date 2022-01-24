#include "vfork.h"
#include "../base.h"
#include "../errno.h"
#include "../signal/duct_signals.h"
#include <linux-syscalls/linux.h>

#include "fork.h"

long sys_vfork(void)
{
	// this used to be a separate implementation, but it did everything exactly the same as sys_fork,
	// so now it just calls sys_fork to avoid duplicating code
	return sys_fork();
}

