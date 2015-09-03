#ifndef LINUX_ACCESS_H
#define LINUX_ACCESS_H

long sys_access(const char* filename, int amode);

#define LINUX_AT_FDCWD -100

#endif

