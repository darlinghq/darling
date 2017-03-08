#include "getdtablesize.h"
#include "../misc/getrlimit.h"
#include "../base.h"
#include "../errno.h"
#include <limits.h>
#include <linux-syscalls/linux.h>

#define min(a,b) ((a) < (b)) ? (a) : (b)

long sys_getdtablesize(void)
{
	int ret;
	struct rlimit lim;

	ret = sys_getrlimit(BSD_RLIMIT_NOFILE, &lim);
	if (ret < 0)
		return ret;

	return min(lim.rlim_cur, INT_MAX);
}

