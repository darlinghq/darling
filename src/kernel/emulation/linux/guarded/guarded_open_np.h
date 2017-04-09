#ifndef LINUX_GUARDED_OPEN_H
#define LINUX_GUARDED_OPEN_H
#include <stdint.h>

long sys_guarded_open_np(const char* filename, uint64_t* guardid, unsigned int guardflags, int flags, unsigned int mode);

#endif

