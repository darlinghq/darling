#include "shutdown.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "socket.h"
#include "duct.h"

long sys_shutdown(int fd, int how)
{
	int ret;

#ifdef __NR_socketcall
	ret = LINUX_SYSCALL(__NR_socketcall, LINUX_SYS_SHUTDOWN,
			((long[6]) { fd, how }));
#else
	ret = LINUX_SYSCALL(__NR_shutdown, fd, how);
#endif

	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

