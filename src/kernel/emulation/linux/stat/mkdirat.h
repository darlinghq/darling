#ifndef LINUX_MKDIRAT_H
#define LINUX_MKDIRAT_H

long sys_mkdirat(int fd, const char* path, unsigned int mode);

#endif

