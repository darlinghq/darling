#ifndef LINUX_SETSOCKOPT_H
#define LINUX_SETSOCKOPT_H

long sys_setsockopt(int fd, int level, int optname, const void* optval, int optlen);

#define LINUX_SYS_SETSOCKOPT	14

#endif

