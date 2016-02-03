#include "socketpair.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include "../../../../../platform-include/sys/socket.h"
#include "../../../../../platform-include/sys/errno.h"
#include "duct.h"

long sys_socketpair(int domain, int type, int protocol, int sv[2])
{
	int ret;
	int linux_domain;

	switch (domain)
	{
		case PF_LOCAL:
			linux_domain = LINUX_PF_LOCAL; break;
		case PF_INET:
			linux_domain = LINUX_PF_INET; break;
		case PF_IPX:
			linux_domain = LINUX_PF_IPX; break;
		case PF_INET6:
			linux_domain = LINUX_PF_INET6; break;
		default:
			return -EINVAL;
	}

#ifdef __NR_socketcall
	ret = LINUX_SYSCALL(__NR_socketcall, LINUX_SYS_SOCKETPAIR,
			((long[6]) { linux_domain, type, protocol, sv }));
#else
	ret = LINUX_SYSCALL(__NR_socketpair, linux_domain, type, protocol, sv);
#endif

	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

