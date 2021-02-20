#include "kqueue.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include <stddef.h>
#include <sys/errno.h>
#include "../simple.h"
#include "../mach/lkm.h"
#include <lkm/api.h>

long sys_kqueue(void)
{
	return lkm_call(NR_kqueue_create, NULL);
}

