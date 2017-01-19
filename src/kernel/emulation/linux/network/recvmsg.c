#include "recvmsg.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include <stddef.h>
#include "duct.h"

extern void *malloc(__SIZE_TYPE__ size);
extern void free(void* ptr);
extern void* memcpy(void* dest, const void* src, __SIZE_TYPE__ len);

long sys_recvmsg(int socket, struct bsd_msghdr* msg, int flags)
{
	return sys_recvmsg_nocancel(socket, msg, flags);
}

long sys_recvmsg_nocancel(int socket, struct bsd_msghdr* msg, int flags)
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
		lchdr = (struct linux_cmsghdr*) malloc(msg->msg_controllen + 4);
		lmsg.msg_control = lchdr;
		lmsg.msg_controllen = msg->msg_controllen + 4;
	}

	lmsg.msg_flags = 0; // set on return

	linux_flags = msgflags_bsd_to_linux(flags);

#ifdef __NR_socketcall
	ret = LINUX_SYSCALL(__NR_socketcall, LINUX_SYS_RECVMSG,
			((long[6]) { socket, &lmsg, linux_flags }));
#else
	ret = LINUX_SYSCALL(__NR_recvmsg, socket, &lmsg, linux_flags);
#endif

	if (ret < 0)
		ret = errno_linux_to_bsd(ret);
	else
	{
		struct bsd_cmsghdr* bchdr;
		msg->msg_flags = msgflags_linux_to_bsd(lmsg.msg_flags);

		bchdr = (struct bsd_cmsghdr*) msg->msg_control;

		if (bchdr != NULL)
		{
			bchdr->cmsg_len = lchdr->cmsg_len;
			bchdr->cmsg_level = lchdr->cmsg_level;
			bchdr->cmsg_type = lchdr->cmsg_type;

			memcpy(bchdr->cmsg_data, lchdr->cmsg_data,
					lchdr->cmsg_len - offsetof(struct linux_cmsghdr, cmsg_data));
		}
	}

	if (lchdr != NULL)
		free(lchdr);

	return ret;
}


