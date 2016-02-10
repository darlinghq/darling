#ifndef LINUX_READLINK_H
#define LINUX_READLINK_H

long sys_readlink(const char* path, char* buf, int count);

#endif

