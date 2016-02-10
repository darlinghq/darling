#ifndef LINUX_RECVMSG_H
#define LINUX_RECVMSG_H
#include <stdint.h>

struct iovec;
struct bsd_msghdr
{
	void* msg_name;
	uint32_t msg_namelen;
	struct iovec* msg_iov;
	int msg_iovlen;
	void* msg_control;
	uint32_t msg_controllen;
	int msg_flags;
};

struct linux_msghdr
{
	void* msg_name;
	uint32_t msg_namelen;
	struct iovec* msg_iov;
	unsigned long msg_iovlen;
	void* msg_control;
	unsigned long msg_controllen;
	int msg_flags;
};

struct bsd_cmsghdr
{
	uint32_t cmsg_len;
	int cmsg_level;
	int cmsg_type;
	unsigned char cmsg_data[];
};

struct linux_cmsghdr
{
	unsigned long cmsg_len;
	int cmsg_level;
	int cmsg_type;
	unsigned char cmsg_data[];
};

long sys_recvmsg(int socket, struct bsd_msghdr* msg, int flags);
long sys_recvmsg_nocancel(int socket, struct bsd_msghdr* msg, int flags);

#define LINUX_SYS_RECVMSG	17

#endif

