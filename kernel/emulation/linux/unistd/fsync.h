#ifndef LINUX_FSYNC_H
#define LINUX_FSYNC_H

long sys_fsync(int fd);
long sys_fsync_nocancel(int fd);

#endif

