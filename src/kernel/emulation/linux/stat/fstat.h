#ifndef LINUX_FSTAT_H
#define LINUX_FSTAT_H

struct stat;
struct stat64;

long sys_fstat(int fd, struct stat* stat);
long sys_fstat64(int fd, struct stat64* stat);

#endif

