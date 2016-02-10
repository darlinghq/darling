#ifndef BSDTHREAD_TERMINATE_H
#define BSDTHREAD_TERMINATE_H
#include <stdint.h>

long sys_bsdthread_terminate(void* stackaddr, unsigned long freesize, int port,
		int join_sem);

#endif

