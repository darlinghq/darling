#ifndef LINUX_FSETXATTR_H
#define LINUX_FSETXATTR_H

long sys_fsetxattr(int fd, const char* name, const void* value,
		unsigned long size, unsigned int pos, int options);

#endif

