#ifndef LINUX_UTIMES_H
#define LINUX_UTIMES_H
#include "gettimeofday.h"

long sys_utimes(const char* path, struct bsd_timeval* tv);

#endif

