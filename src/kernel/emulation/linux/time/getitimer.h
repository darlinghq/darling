#ifndef LINUX_GETITIMER_H
#define LINUX_GETITIMER_H
#include "setitimer.h"

long sys_getitimer(int which, struct bsd_itimerval* oitv);

#endif

