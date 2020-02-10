#ifndef NETWORK_DUCT_H
#define NETWORK_DUCT_H

int sfamily_linux_to_bsd(int family);
int sfamily_bsd_to_linux(int family);
int msgflags_bsd_to_linux(int flags);
int msgflags_linux_to_bsd(int flags);

struct sockaddr_fixup
{
	union
	{
		unsigned short linux_family;
		struct
		{
			unsigned char bsd_length;
			unsigned char bsd_family;
		};
	};
	union
	{
		char sun_path[104];
	};
};

#define LINUX_PF_LOCAL	1
#define LINUX_PF_INET	2
#define LINUX_PF_IPX	4
#define LINUX_PF_INET6	10

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

