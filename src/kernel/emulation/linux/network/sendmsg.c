#include "sendmsg.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include <stddef.h>
#include "duct.h"
#include "../bsdthread/cancelable.h"

extern void *malloc(__SIZE_TYPE__ size);
extern void free(void* ptr);
extern void* memcpy(void* dest, const void* src, __SIZE_TYPE__ len);

long sys_sendmsg(int socket, const struct bsd_msghdr* msg, int flags)
{
	CANCELATION_POINT();
	return sys_sendmsg_nocancel(socket, msg, flags);
}

long sys_sendmsg_nocancel(int socket, const struct bsd_msghdr* msg, int flags)
{
	int ret, linux_flags;
	struct linux_msghdr lmsg;
	struct linux_cmsghdr* lchdr = NULL;
	struct bsd_sockaddr* xname;

	if (msg->msg_name != NULL && msg->msg_namelen > 0)
	{
		struct sockaddr_fixup* saddr = __builtin_alloca(msg->msg_namelen);
		memcpy(saddr, msg->msg_name, msg->msg_namelen);
		saddr->linux_family = sfamily_bsd_to_linux(saddr->bsd_family);
		lmsg.msg_name = saddr;
	}

	lmsg.msg_namelen = msg->msg_namelen;
	lmsg.msg_iov = msg->msg_iov;
	lmsg.msg_iovlen = msg->msg_iovlen;

	if (sizeof(unsigned long) == 4)
	{
		lmsg.msg_control = msg->msg_control;
		lmsg.msg_controllen = msg->msg_controllen;
	}
	else if (msg->msg_control != NULL && msg->msg_controllen > 0)
	{
		struct bsd_cmsghdr* bchdr;

		bchdr = (struct bsd_cmsghdr*) msg->msg_control;
		lchdr = (struct linux_cmsghdr*) malloc(msg->msg_controllen + 4);

		lmsg.msg_control = lchdr;
		lmsg.msg_controllen = msg->msg_controllen + 4;

		lchdr->cmsg_len = bchdr->cmsg_len;
		lchdr->cmsg_level = socket_level_bsd_to_linux(bchdr->cmsg_level);
		lchdr->cmsg_type = bchdr->cmsg_type;

		memcpy(lchdr->cmsg_data, bchdr->cmsg_data,
				lchdr->cmsg_len - sizeof(struct bsd_cmsghdr));
	}
	else
	{
		lmsg.msg_control = NULL;
		lmsg.msg_controllen = 0;
	}

	lmsg.msg_flags = 0; // ignored

	linux_flags = msgflags_bsd_to_linux(flags);

#ifdef __NR_socketcall
	ret = LINUX_SYSCALL(__NR_socketcall, LINUX_SYS_SENDMSG,
			((long[6]) { socket, &lmsg, linux_flags }));
#else
	ret = LINUX_SYSCALL(__NR_sendmsg, socket, &lmsg, linux_flags);
#endif

	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	if (lchdr != NULL)
		free(lchdr);

	return ret;
}

