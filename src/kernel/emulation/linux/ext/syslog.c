#include "syslog.h"
#include "../errno.h"
#include "../base.h"
#include <asm/unistd.h>

extern long cerror(int __err);

VISIBLE
int __linux_syslog (int type, char* bufp, int len)
{
	int rv;

	rv = LINUX_SYSCALL(__NR_syslog, type, bufp, len);
	if (rv < 0)
	{
		cerror(errno_linux_to_bsd(rv));
		return -1;
	}

	return rv;
}

