#include "proc_info.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include "../../../../../platform-include/sys/errno.h"
#include "../ext/syslog.h"
#include "../simple.h"

long sys_proc_info(uint32_t callnum, int32_t pid, uint32_t flavor,
		uint64_t arg, void* buffer, int32_t bufsize)
{
	switch (callnum)
	{
		case 4: // proc_kmsgbuf
		{
			int ret;

			ret = __linux_syslog(SYSLOG_ACTION_READ_ALL, (char*) buffer,
					bufsize);
			if (ret < 0)
				return errno_linux_to_bsd(ret);

			return ret;
		}
		case 1: // proc_listpids
		case 2: // proc_pidinfo
		case 3: // proc_pidfdinfo
		default:
			__simple_printf("sys_proc_info(): Unsupported callnum: %d\n",
					callnum);
			return -ENOTSUP;
	}
}

