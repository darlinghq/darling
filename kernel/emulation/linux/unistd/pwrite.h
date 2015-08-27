#ifndef LINUX_PWRITE_H
#define LINUX_PWRITE_H

int sys_pwrite(int fd, const void* mem, int len, long long ofs);
int sys_pwrite_nocancel(int fd, const void* mem, int len, long long ofs);

#endif

