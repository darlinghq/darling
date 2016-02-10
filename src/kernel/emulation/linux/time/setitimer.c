#include "setitimer.h"
#include "../base.h"
#include "../errno.h"
#include "../../../../../platform-include/sys/errno.h"
#include <asm/unistd.h>
#include <stddef.h>

long sys_setitimer(int which, struct bsd_itimerval* itv,
		struct bsd_itimerval* oitv)
{
	int ret;
	struct linux_itimerval linux_itv, linux_oitv;
	
	if (itv == NULL)
		return -EFAULT;
	
	linux_itv.it_interval.tv_sec = itv->it_interval.tv_sec;
	linux_itv.it_interval.tv_usec = itv->it_interval.tv_usec;
	linux_itv.it_value.tv_sec = itv->it_value.tv_sec;
	linux_itv.it_value.tv_usec = itv->it_value.tv_usec;

	ret = LINUX_SYSCALL(__NR_setitimer, which, &linux_itv, &linux_oitv);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);
	else if (oitv != NULL)
	{
		oitv->it_interval.tv_sec = linux_oitv.it_interval.tv_sec;
		oitv->it_interval.tv_usec = linux_oitv.it_interval.tv_usec;
		oitv->it_value.tv_sec = linux_oitv.it_value.tv_sec;
		oitv->it_value.tv_usec = linux_oitv.it_value.tv_usec;
	}

	return ret;
}

