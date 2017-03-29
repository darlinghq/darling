#ifndef WORKQ_KERNRETURN_H
#define WORKQ_KERNRETURN_H

struct parked_thread
{
	struct parked_thread *prev, *next;
	int sem, prio;
};
struct timespec;

long sys_workq_kernreturn(int options, void* item, int affinity, int prio);

#endif

