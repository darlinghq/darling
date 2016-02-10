#ifndef LINUX_READV_H
#define LINUX_READV_H

struct iovec;
long sys_readv(int fd, struct iovec* iovp, unsigned int len);
long sys_readv_nocancel(int fd, struct iovec* iovp, unsigned int len);

#endif

