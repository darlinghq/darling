#ifndef LINUX_FUTIMES_H
#define LINUX_FUTIMES_H
#include "gettimeofday.h"

long sys_futimes(int fd, struct bsd_timeval* tv);

#endif

