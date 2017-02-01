#ifndef LINUX_OPENAT_H
#define LINUX_OPENAT_H

long sys_openat(int fd, const char* filename, int flags, unsigned int mode);
long sys_openat_nocancel(int fd, const char* filename, int flags, unsigned int mode);

#endif

