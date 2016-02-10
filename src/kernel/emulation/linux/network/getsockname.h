#ifndef LINUX_GETSOCKNAME_H
#define LINUX_GETSOCKNAME_H

long sys_getsockname(int fd, void* asa, int* socklen);

#define LINUX_SYS_GETSOCKNAME	6

#endif

