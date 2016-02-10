#ifndef LINUX_FLISTXATTR_H
#define LINUX_FLISTXATTR_H

long sys_flistxattr(int fd, char* namebuf, unsigned long size, int options);

#endif

