#ifndef LINUX_READLINKAT_H
#define LINUX_READLINKAT_H

long sys_readlinkat(int fd, const char* path, char* buf, int count);

#endif

