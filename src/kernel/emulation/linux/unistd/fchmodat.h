#ifndef LINUX_FCHMODAT_H
#define LINUX_FCHMODAT_H

long sys_fchmodat(int fd, const char* path, int mode, int flag);

#endif

