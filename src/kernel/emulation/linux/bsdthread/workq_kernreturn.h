#ifndef WORKQ_KERNRETURN_H
#define WORKQ_KERNRETURN_H
#define PRIVATE 1
#include <sys/event.h>

// Darling specific
#define WQOPS_QUEUE_REQTHREAD_FOR_KEVENT  0x100020

struct wq_kevent_data
{
	struct kevent_qos_s* events;
	int nevents;
	int sem;
};

struct timespec;

long sys_workq_kernreturn(int options, void* item, int affinity, int prio);

int sem_down(int* sem, int timeout);
void sem_up(int* sem);

#endif

