#ifndef LINUX_FSGETPATH_H
#define LINUX_FSGETPATH_H

long sys_fsgetpath(const char* buf, unsigned long size, void* fsid, unsigned long long objid);

#endif // LINUX_FSGETPATH_H
