#ifndef LINUX_FCHMOD_EXTENDED_H
#define LINUX_FCHMOD_EXTENDED_H

long sys_fchmod_extended(int fd, int uid, int gid, int mode, void* xsec);

#endif

