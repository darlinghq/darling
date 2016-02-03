#ifndef LINUX_LISTEN_H
#define LINUX_LISTEN_H

long sys_listen(int fd, int backlog);

#define LINUX_SYS_LISTEN	4

#endif

