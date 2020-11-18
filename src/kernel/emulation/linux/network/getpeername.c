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
		fixed->bsd_family = sfamily_linux_to_bsd(fixed->linux_family);

		if (fixed->bsd_family == PF_LOCAL) {
			struct vchroot_unexpand_args vc;
			strcpy(vc.path, fixed->sun_path);

			ret = vchroot_unexpand(&vc);
			if (ret < 0)
				return errno_linux_to_bsd(ret);

			strncpy(fixed->sun_path, vc.path, sizeof(fixed->sun_path) - 1);
			fixed->sun_path[sizeof(fixed->sun_path) - 1] = '\0';
			*socklen = sizeof(*fixed) - sizeof(fixed->sun_path) + strlen(fixed->sun_path) + 1;
		}

		fixed->bsd_length = *socklen;
	}

	return ret;
}

