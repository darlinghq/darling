#ifndef LINUX_CLOSE_H
#define LINUX_CLOSE_H

int sys_close(int fd);
int sys_close_nocancel(int fd);

#endif

