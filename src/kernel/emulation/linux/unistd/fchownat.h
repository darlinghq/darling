#ifndef LINUX_FCHOWNAT_H
#define LINUX_FCHOWNAT_H

long sys_fchownat(int fd, const char* path, int uid, int gid, int flag);

#endif

