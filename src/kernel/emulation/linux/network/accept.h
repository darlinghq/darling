#ifndef LINUX_ACCEPT_H
#define LINUX_ACCEPT_H

long sys_accept(int fd, void* from, int* socklen);
long sys_accept_nocancel(int fd, void* from, int* socklen);

#define LINUX_SYS_ACCEPT	5

#endif

