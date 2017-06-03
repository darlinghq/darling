#ifndef LINUX_SEM_OPEN_H
#define LINUX_SEM_OPEN_H

long sys_sem_open(const char* name, int oflag, int mode, int value);

#endif

