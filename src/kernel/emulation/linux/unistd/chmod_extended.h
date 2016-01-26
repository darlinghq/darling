#ifndef LINUX_CHMOD_EXTENDED_H
#define LINUX_CHMOD_EXTENDED_H

long sys_chmod_extended(const char* path, int uid, int gid, int mode, void* xsec);

#endif

