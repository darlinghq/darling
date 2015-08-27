#ifndef LINUX_FSYNC_H
#define LINUX_FSYNC_H

int sys_fsync(int fd);
int sys_fsync_nocancel(int fd);

#endif

