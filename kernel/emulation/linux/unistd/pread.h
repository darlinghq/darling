#ifndef LINUX_PREAD_H
#define LINUX_PREAD_H

int sys_pread(int fd, void* mem, int len, long long ofs);
int sys_pread_nocancel(int fd, void* mem, int len, long long ofs);

#endif

