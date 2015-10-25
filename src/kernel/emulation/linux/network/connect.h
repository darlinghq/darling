#ifndef LINUX_CONNECT_H
#define LINUX_CONNECT_H

long sys_connect(int fd, const void* name, int socklen);

#define LINUX_SYS_CONNECT	2

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
};

#endif

