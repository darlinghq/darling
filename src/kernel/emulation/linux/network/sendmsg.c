#include "sendmsg.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include <stddef.h>
#include "duct.h"

extern void *malloc(__SIZE_TYPE__ size);
extern void free(void* ptr);
extern void* memcpy(void* dest, const void* src, __SIZE_TYPE__ len);

long sys_sendmsg(int socket, const struct bsd_msghdr* msg, int flags)
{
	return sys_sendmsg_nocancel(socket, msg, flags);
}

long sys_sendmsg_nocancel(int socket, const struct bsd_msghdr* msg, int flags)
{
	int ret, linux_flags;
	struct linux_msghdr lmsg;
	struct linux_cmsghdr* lchdr = NULL;

	lmsg.msg_name = msg->msg_name;
	lmsg.msg_namelen = msg->msg_namelen;
	lmsg.msg_iov = msg->msg_iov;
	lmsg.msg_iovlen = msg->msg_iovlen;

	if (sizeof(unsigned long) == 4)
	{
		lmsg.msg_control = msg->msg_control;
		lmsg.msg_controllen = msg->msg_controllen;
	}
	else if (msg->msg_control != NULL)
	{
		struct bsd_cmsghdr* bchdr;

		bchdr = (struct bsd_cmsghdr*) msg->msg_control;
		lchdr = (struct linux_cmsghdr*) malloc(msg->msg_controllen + 4);

		lmsg.msg_control = lchdr;
		lmsg.msg_controllen = msg->msg_controllen + 4;

		lchdr->cmsg_len = bchdr->cmsg_len;
		lchdr->cmsg_level = bchdr->cmsg_level;
		lchdr->cmsg_type = bchdr->cmsg_type;

		memcpy(lchdr->cmsg_data, bchdr->cmsg_data,
				lchdr->cmsg_len - offsetof(struct bsd_cmsghdr, cmsg_data));
	}

	lmsg.msg_flags = 0; // ignored

	linux_flags = msgflags_bsd_to_linux(flags);

#ifdef __NR_socketcall
	ret = LINUX_SYSCALL(__NR_socketcall, LINUX_SYS_SENDMSG,
			((long[6]) { socket, &lmsg, linux_flags }));
#else
	ret = LINUX_SYSCALL(__NR_recvmsg, socket, &lmsg, linux_flags);
#endif

	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	if (lchdr != NULL)
		free(lchdr);

	return ret;
}

