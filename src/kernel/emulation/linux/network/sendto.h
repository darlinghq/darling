#ifndef LINUX_SENDTO_H
#define LINUX_SENDTO_H

long sys_sendto(int fd, const void* buffer, unsigned long length,
		int flags, const void* sockaddr, int socklen);
long sys_sendto_nocancel(int fd, const void* buffer, unsigned long length,
		int flags, const void* sockaddr, int socklen);

#define LINUX_SYS_SENDTO	11

#endif

