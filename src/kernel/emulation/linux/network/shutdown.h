#ifndef LINUX_SHUTDOWN_H
#define LINUX_SHUTDOWN_H

long sys_shutdown(int fd, int how);

#define LINUX_SYS_SHUTDOWN	13

#endif

