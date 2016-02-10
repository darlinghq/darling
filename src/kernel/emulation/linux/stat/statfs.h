#ifndef LINUX_STATFS_H
#define LINUX_STATFS_H
#include "common.h"

long sys_statfs(const char* path, struct bsd_statfs* buf);
long sys_statfs64(const char* path, struct bsd_statfs64* buf);

#endif

