#include "recvmsg.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include <stddef.h>

extern void *malloc(unsigned long size);
extern void free(void* ptr);
extern void* memcpy(void* dest, const void* src, unsigned long len);

long sys_recvmsg(int socket, struct bsd_msghdr* msg, int flags)
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
	ret = LINUX_SYSCALL(__NR_socketcall, LINUX_SYS_RECVMSG, socket, &lmsg, linux_flags);
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

int msgflags_bsd_to_linux(int flags)
{
	int oflags = 0;

	if (flags & BSD_MSG_OOB)
		oflags |= LINUX_MSG_OOB;
	if (flags & BSD_MSG_PEEK)
		oflags |= LINUX_MSG_PEEK;
	if (flags & BSD_MSG_DONTROUTE)
		oflags |= LINUX_MSG_DONTROUTE;
	if (flags & BSD_MSG_EOR)
		oflags |= LINUX_MSG_EOR;
	if (flags & BSD_MSG_TRUNC)
		oflags |= LINUX_MSG_TRUNC;
	if (flags & BSD_MSG_CTRUNC)
		oflags |= LINUX_MSG_CTRUNC;
	if (flags & BSD_MSG_WAITALL)
		oflags |= LINUX_MSG_WAITALL;
	if (flags & BSD_MSG_DONTWAIT)
		oflags |= LINUX_MSG_DONTWAIT;

	return oflags;
}

int msgflags_linux_to_bsd(int flags)
{
	int oflags = 0;

	if (flags & LINUX_MSG_OOB)
		oflags |= BSD_MSG_OOB;
	if (flags & LINUX_MSG_PEEK)
		oflags |= BSD_MSG_PEEK;
	if (flags & LINUX_MSG_DONTROUTE)
		oflags |= BSD_MSG_DONTROUTE;
	if (flags & LINUX_MSG_EOR)
		oflags |= BSD_MSG_EOR;
	if (flags & LINUX_MSG_TRUNC)
		oflags |= BSD_MSG_TRUNC;
	if (flags & LINUX_MSG_CTRUNC)
		oflags |= BSD_MSG_CTRUNC;
	if (flags & LINUX_MSG_WAITALL)
		oflags |= BSD_MSG_WAITALL;
	if (flags & LINUX_MSG_DONTWAIT)
		oflags |= BSD_MSG_DONTWAIT;

	return oflags;
}

