#ifndef LINUX_REMOVEXATTR_H
#define LINUX_REMOVEXATTR_H

long sys_removexattr(const char* path, const char* name, int options);

#endif

