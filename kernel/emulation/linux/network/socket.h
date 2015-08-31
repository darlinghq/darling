#ifndef LINUX_SOCKET_H
#define LINUX_SOCKET_H

int sys_socket(int domain, int type, int protocol);

#define LINUX_SYS_SOCKET	1

#define LINUX_PF_LOCAL	1
#define LINUX_PF_INET	2
#define LINUX_PF_IPX	4
#define LINUX_PF_INET6	10

#endif

