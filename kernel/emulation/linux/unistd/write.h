#ifndef LINUX_WRITE_H
#define LINUX_WRITE_H

int sys_write(int fd, const void* mem, int len);
int sys_write_nocancel(int fd, const void* mem, int len);

#endif

