#include "fileport_makefd.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../mach/lkm.h"
#include "../../../../external/lkm/api.h"

long sys_fileport_makefd(int port)
{
	int ret = lkm_call(NR_fileport_makefd, (void*)(unsigned long) port);

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	return ret;
}
