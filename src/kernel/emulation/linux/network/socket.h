#ifndef LINUX_SOCKET_H
#define LINUX_SOCKET_H

long sys_socket(int domain, int type, int protocol);

int sfamily_linux_to_bsd(int family);
int sfamily_bsd_to_linux(int family);

#define LINUX_SYS_SOCKET	1

#endif

