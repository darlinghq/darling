#ifndef LINUX_MADVISE_H
#define LINUX_MADVISE_H

long sys_madvise(void* addr, unsigned long len, int advice);

#endif

