#ifndef LINUX_SYSV_SEMOP_H
#define LINUX_SYSV_SEMOP_H
#include <stdint.h>

struct sembuf;
long sys_semop(int semid, struct sembuf *sops, int nsops);

#endif
