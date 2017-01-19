#include "sys/utsname.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>

extern void cerror(int e);

int __linux_uname(struct linux_utsname* p)
{
	int rv;

	rv = LINUX_SYSCALL(__NR_uname, p);
	if (rv < 0)
	{
		cerror(errno_linux_to_bsd(rv));
		return -1;
	}

	return rv;
}

