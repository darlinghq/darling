#ifndef LINUX_BIND_H
#define LINUX_BIND_H

long sys_bind(int fd, const void* name, int socklen);

#define LINUX_SYS_BIND	2

#endif

