#ifndef LINUX_LISTXATTR_H
#define LINUX_LISTXATTR_H

long sys_listxattr(const char* path, char* namebuf, unsigned long size, int options);

#endif

