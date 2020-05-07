#include "recvmsg.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include <stddef.h>
#include "duct.h"
#include "getsockopt.h"
#include "../bsdthread/cancelable.h"

extern void *malloc(__SIZE_TYPE__ size);
extern void free(void* ptr);
extern void* memcpy(void* dest, const void* src, __SIZE_TYPE__ len);

#define ALIGN(what, how) ( ((what) + (how) - 1) & ~((how) - 1) )

long sys_recvmsg(int socket, struct bsd_msghdr* msg, int flags)
{
	CANCELATION_POINT();
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
	else if (msg->msg_control != NULL && msg->msg_controllen > 0)
	{
		// __simple_printf("controllen=%d\n", msg->msg_controllen);
		lmsg.msg_controllen = LINUX_CMSG_ALIGN(msg->msg_controllen + LINUX_BSD_CMSGHDR_SIZE_DIFFERENCE); // FIXME: there could be multiple control messages in a message
		lchdr = (struct linux_cmsghdr*) malloc(lmsg.msg_controllen);
		lmsg.msg_control = lchdr;
	}
	else
	{
		lmsg.msg_control = NULL;
		lmsg.msg_controllen = 0;
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
			if (sizeof(unsigned long) != 4)
			{
				int lpos = 0, bpos = 0;
				struct linux_cmsghdr* lnext;
				struct bsd_cmsghdr* bnext;

				msg->msg_controllen = lmsg.msg_controllen;

				// __simple_printf("Processing %d bytes of control data\n", msg->msg_controllen);
				while (lpos < lmsg.msg_controllen)
				{
					lnext = (struct linux_cmsghdr*) (((char*) lmsg.msg_control) + lpos);
					bnext = (struct bsd_cmsghdr*) (((char*) msg->msg_control) + bpos);

					// __simple_printf("Linux msg at %d -> BSD at %d\n", lpos, bpos);

					bnext->cmsg_len = lnext->cmsg_len - LINUX_BSD_CMSGHDR_SIZE_DIFFERENCE;
					bnext->cmsg_level = socket_level_linux_to_bsd(lnext->cmsg_level);
					bnext->cmsg_type = lnext->cmsg_type;

					// __simple_printf("About to copy %d bytes\n", lnext->cmsg_len - sizeof(struct linux_cmsghdr));

					size_t actual_data_size = lnext->cmsg_len - sizeof(struct linux_cmsghdr);
					memcpy(bnext->cmsg_data, lnext->cmsg_data, actual_data_size);
					msg->msg_controllen -= LINUX_CMSG_SPACE(actual_data_size) - BSD_CMSG_SPACE(actual_data_size);

					lpos += ALIGN(lnext->cmsg_len, sizeof(unsigned long));
					bpos += ALIGN(bnext->cmsg_len, 4);
				}
			}
			else
			{
				msg->msg_controllen = lmsg.msg_controllen;
			}
		}

		struct sockaddr_fixup* saddr = (struct sockaddr_fixup*) lmsg.msg_name;
		if (saddr != NULL && lmsg.msg_namelen > 0)
			saddr->bsd_family = sfamily_linux_to_bsd(saddr->linux_family);
		msg->msg_namelen = lmsg.msg_namelen;
	}

	if (lchdr != NULL)
		free(lchdr);

	// __simple_printf("Returning %d\n", ret);
	return ret;
}


int socket_level_bsd_to_linux(int level)
{
	switch (level)
	{
		case BSD_SOL_SOCKET:
			return LINUX_SOL_SOCKET;
		default:
			return level;
	}
}

int socket_level_linux_to_bsd(int level)
{
	switch (level)
	{
		case LINUX_SOL_SOCKET:
			return BSD_SOL_SOCKET;
		default:
			return level;
	}
}

