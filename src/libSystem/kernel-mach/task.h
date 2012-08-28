#ifndef MACH_TASK_H
#define MACH_TASK_H
#include <sys/types.h>

struct darwin_task
{
	darwin_task(pid_t p) : pid(p) {}
	pid_t pid;
};

typedef darwin_task* darwin_task_t;

extern "C"
{
	extern darwin_task_t mach_task_self_;
	inline darwin_task_t mach_task_self() { return mach_task_self_; }
}

#endif
