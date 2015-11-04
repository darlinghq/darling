#ifndef LINUX_FGETXATTR_H
#define LINUX_FGETXATTR_H

long sys_fgetxattr(int fd, const char* name, char* value,
		unsigned long size, unsigned int pos, int options);

#endif

