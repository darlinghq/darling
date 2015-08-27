#ifndef LINUX_READ_H
#define LINUX_READ_H

int sys_read(int fd, void* mem, int len);
int sys_read_nocancel(int fd, void* mem, int len);

#endif

