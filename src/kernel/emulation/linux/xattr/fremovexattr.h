#ifndef LINUX_FREMOVEXATTR_H
#define LINUX_FREMOVEXATTR_H

long sys_fremovexattr(int fd, const char* name, int options);

#endif

