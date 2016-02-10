#ifndef LINUX_SOCKETPAIR_H
#define LINUX_SOCKETPAIR_H

long sys_socketpair(int domain, int type, int protocol, int sv[2]);

#define LINUX_SYS_SOCKETPAIR	8

#endif

