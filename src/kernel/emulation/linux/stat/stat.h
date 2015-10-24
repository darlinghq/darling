#ifndef LINUX_STAT_H
#define LINUX_STAT_H

struct stat;
struct stat64;

long sys_stat(const char* path, struct stat* stat);
long sys_stat64(const char* path, struct stat64* stat);

#endif

