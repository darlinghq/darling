#ifndef LINUX_FACCESSAT_H
#define LINUX_FACCESSAT_H

long sys_faccessat(int fd, const char* filename, int amode, int flag);

#endif

