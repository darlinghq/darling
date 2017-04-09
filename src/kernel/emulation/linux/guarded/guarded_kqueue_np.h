#ifndef LINUX_GUARDED_KQUEUE_H
#define LINUX_GUARDED_KQUEUE_H
#include <stdint.h>

long sys_guarded_kqueue_np(uint64_t* guardid, unsigned guardflags);

#endif

