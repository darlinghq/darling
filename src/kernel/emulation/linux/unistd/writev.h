#ifndef LINUX_WRITEV_H
#define LINUX_WRITEV_H

struct iovec;
long sys_writev(int fd, struct iovec* iovp, unsigned int len);
long sys_writev_nocancel(int fd, struct iovec* iovp, unsigned int len);

#endif

