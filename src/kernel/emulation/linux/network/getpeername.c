#include "getpeername.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include <sys/socket.h>
#include "duct.h"

#include "../vchroot_expand.h"

extern __SIZE_TYPE__ strlen(const char* src);
extern char* strcpy(char* dest, const char* src);
extern char *strncpy(char *dest, const char *src, __SIZE_TYPE__ n);

long sys_getpeername(int fd, void* asa, int* socklen)
{
	int ret;
	struct sockaddr_fixup* fixed;

#ifdef __NR_socketcall
	ret = LINUX_SYSCALL(__NR_socketcall, LINUX_SYS_GETPEERNAME,
		((long[6]) { fd, asa, socklen }));
#else
	ret = LINUX_SYSCALL(__NR_getpeername, fd, asa, socklen);
#endif

	if (ret < 0)
		ret = errno_linux_to_bsd(ret);
	else
	{
		fixed = (struct sockaddr_fixup*) asa;
		ret = *socklen = sockaddr_fixup_from_linux(fixed, asa, *socklen);
	}

	return ret;
}

