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

int socket_level_bsd_to_linux(int level);
int socket_level_linux_to_bsd(int level);

#define LINUX_SYS_RECVMSG	17

#define BSD_CMSG_ALIGN(len) (((len) + sizeof(uint32_t) - 1) & (size_t)~(sizeof(uint32_t) - 1))
#define BSD_CMSG_SPACE(len) (BSD_CMSG_ALIGN(sizeof(struct bsd_cmsghdr)) + BSD_CMSG_ALIGN(len))
#define BSD_CMSG_LEN(len) (BSD_CMSG_ALIGN(sizeof(struct bsd_cmsghdr)) + (len))

#define LINUX_CMSG_ALIGN(len) (((len) + sizeof(unsigned long) - 1) & (size_t)~(sizeof(unsigned long) - 1))
#define LINUX_CMSG_SPACE(len) (LINUX_CMSG_ALIGN(sizeof(struct linux_cmsghdr)) + LINUX_CMSG_ALIGN(len))
#define LINUX_CMSG_LEN(len) (LINUX_CMSG_ALIGN(sizeof(struct linux_cmsghdr)) + (len))

#define LINUX_BSD_CMSGHDR_SIZE_DIFFERENCE (sizeof(struct linux_cmsghdr) - sizeof(struct bsd_cmsghdr))

#endif

