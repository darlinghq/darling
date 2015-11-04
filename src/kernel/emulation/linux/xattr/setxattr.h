#ifndef LINUX_SETXATTR_H
#define LINUX_SETXATTR_H

long sys_setxattr(const char* path, const char* name, const void* value,
		unsigned long size, unsigned int pos, int options);

#endif

