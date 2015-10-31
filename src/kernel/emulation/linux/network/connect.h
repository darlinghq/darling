#ifndef LINUX_CONNECT_H
#define LINUX_CONNECT_H

long sys_connect(int fd, const void* name, int socklen);
long sys_connect_nocancel(int fd, const void* name, int socklen);

#define LINUX_SYS_CONNECT	2

#endif

