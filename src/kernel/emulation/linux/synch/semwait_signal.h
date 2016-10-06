#ifndef LINUX_SEMWAIT_SIGNAL_H
#define LINUX_SEMWAIT_SIGNAL_H
#include <stdint.h>

long sys_semwait_signal(int cond_sem, int mutex_sem, int timeout, int relative, int64_t tv_sec, int32_t tv_nsec);
long sys_semwait_signal_nocancel(int cond_sem, int mutex_sem, int timeout, int relative, int64_t tv_sec, int32_t tv_nsec);

#endif

