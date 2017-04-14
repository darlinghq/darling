#ifndef LINUX_SETPRIORITY_H
#define LINUX_SETPRIORITY_H

#include "priority.h"

int sys_setpriority(int which, int who, int prio);

#endif
