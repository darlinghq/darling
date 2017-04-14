#ifndef LINUX_GETPRIORITY_H
#define LINUX_GETPRIORITY_H

#include "priority.h"

int sys_getpriority(int which, id_t who);

#endif
