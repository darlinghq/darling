#ifndef LINUX_SYSV_SEMGET_H
#define LINUX_SYSV_SEMGET_H
#include <stdint.h>

long sys_semget(int32_t key, int nsems, int semflg);

#endif
