#ifndef LINUX_PWRITE_H
#define LINUX_PWRITE_H

long sys_pwrite(int fd, const void* mem, int len, long long ofs);
long sys_pwrite_nocancel(int fd, const void* mem, int len, long long ofs);

#endif

