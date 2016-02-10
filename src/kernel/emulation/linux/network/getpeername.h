#ifndef LINUX_GETPEERNAME_H
#define LINUX_GETPEERNAME_H

long sys_getpeername(int fd, void* asa, int* socklen);

#define LINUX_SYS_GETPEERNAME	7

#endif

