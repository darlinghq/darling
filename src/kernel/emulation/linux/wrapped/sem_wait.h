#ifndef LINUX_SEM_WAIT_H
#define LINUX_SEM_WAIT_H

long sys_sem_wait(int* sem);
long sys_sem_wait_nocancel(int* sem);

#endif

