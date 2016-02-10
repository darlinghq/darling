#ifndef LINUX_PREAD_H
#define LINUX_PREAD_H

long sys_pread(int fd, void* mem, int len, long long ofs);
long sys_pread_nocancel(int fd, void* mem, int len, long long ofs);

#endif

