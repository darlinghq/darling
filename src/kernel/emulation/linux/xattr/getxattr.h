#ifndef LINUX_GETXATTR_H
#define LINUX_GETXATTR_H

#define XATTR_NOFOLLOW	1

long sys_getxattr(const char* path, const char* name, char* value,
		unsigned long size, unsigned int pos, int options);

#endif

