#ifndef LINUX_SYMLINKAT_H
#define LINUX_SYMLINKAT_H

long sys_symlinkat(const char* path, int fd, const char* link);

#endif

