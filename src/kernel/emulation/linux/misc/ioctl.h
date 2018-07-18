#ifndef LINUX_IOCTL_H
#define LINUX_IOCTL_H

long __real_ioctl(int fd, int req, void* arg);
long __real_ioctl_raw(int fd, int req, void* arg);

#endif

