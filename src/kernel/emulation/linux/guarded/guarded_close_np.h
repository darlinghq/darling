#ifndef LINUX_GUARDED_CLOSE_H
#define LINUX_GUARDED_CLOSE_H
#include <stdint.h>

long sys_guarded_close_np(int fd, uint64_t* guardid);

#endif

