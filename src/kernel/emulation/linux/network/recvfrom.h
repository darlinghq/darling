#ifndef LINUX_RECVFROM_H
#define LINUX_RECVFROM_H

long sys_recvfrom(int fd, void* buf, unsigned long len,
		int flags, void* from, int* socklen);
long sys_recvfrom_nocancel(int fd, void* buf, unsigned long len,
		int flags, void* from, int* socklen);

#define LINUX_SYS_RECVFROM	12

#endif

