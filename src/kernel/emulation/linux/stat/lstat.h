#ifndef LINUX_LSTAT_H
#define LINUX_LSTAT_H

struct stat;
struct stat64;

long sys_lstat(const char* path, struct stat* stat);
long sys_lstat64(const char* path, struct stat64* stat);

#endif

