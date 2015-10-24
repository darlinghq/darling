#ifndef LINUX_MMAN_H
#define LINUX_MMAN_H

void* sys_mmap(void* start, unsigned long len, int prot, int flags, int fd, long pos);
long sys_munmap(void* addr, unsigned long len);
long sys_mprotect(void* addr, unsigned long len, int prot);
long sys_mincore(void* addr, unsigned long len, unsigned char* vec);

#endif

