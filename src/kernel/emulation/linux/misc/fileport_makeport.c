#include "fileport_makeport.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../mach/lkm.h"
#include "../../../../external/lkm/api.h"

long sys_fileport_makeport(int fd, int* port)
{
	struct fileport_makeport_args args;

	args.fd = fd;
	args.port_out = 0;

	int ret = lkm_call(NR_fileport_makeport, &args);

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	*port = args.port_out;
	return 0;
}
