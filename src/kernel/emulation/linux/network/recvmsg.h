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
int msgflags_bsd_to_linux(int flags);
int msgflags_linux_to_bsd(int flags);

#define LINUX_SYS_RECVMSG	17

#define LINUX_MSG_OOB             0x01
#define LINUX_MSG_PEEK            0x02
#define LINUX_MSG_DONTROUTE       0x04
#define LINUX_MSG_CTRUNC          0x08
#define LINUX_MSG_PROXY           0x10
#define LINUX_MSG_TRUNC           0x20
#define LINUX_MSG_DONTWAIT        0x40
#define LINUX_MSG_EOR             0x80
#define LINUX_MSG_WAITALL         0x100
#define LINUX_MSG_FIN             0x200
#define LINUX_MSG_SYN             0x400
#define LINUX_MSG_CONFIRM         0x800
#define LINUX_MSG_RST             0x1000
#define LINUX_MSG_ERRQUEUE        0x2000
#define LINUX_MSG_NOSIGNAL        0x4000
#define LINUX_MSG_MORE            0x8000
#define LINUX_MSG_WAITFORONE      0x10000
#define LINUX_MSG_FASTOPEN        0x20000000
#define LINUX_MSG_CMSG_CLOEXEC    0x40000000

#define BSD_MSG_OOB         0x1
#define BSD_MSG_PEEK        0x2
#define BSD_MSG_DONTROUTE   0x4
#define BSD_MSG_EOR         0x8
#define BSD_MSG_TRUNC       0x10
#define BSD_MSG_CTRUNC      0x20
#define BSD_MSG_WAITALL     0x40
#define BSD_MSG_DONTWAIT    0x80
#define BSD_MSG_EOF         0x100
#define BSD_MSG_WAITSTREAM  0x200
#define BSD_MSG_FLUSH       0x400
#define BSD_MSG_HOLD        0x800
#define BSD_MSG_SEND        0x1000
#define BSD_MSG_HAVEMORE    0x2000
#define BSD_MSG_RCVMORE     0x4000
#define BSD_MSG_NEEDSA      0x10000

#endif

