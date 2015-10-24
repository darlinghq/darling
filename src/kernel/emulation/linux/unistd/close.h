#ifndef LINUX_CLOSE_H
#define LINUX_CLOSE_H

long sys_close(int fd);
long sys_close_nocancel(int fd);

#endif

