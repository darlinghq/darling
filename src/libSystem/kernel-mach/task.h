#ifndef MACH_TASK_H
#define MACH_TASK_H
#include <sys/types.h>
#include <mach/kern_return.h>

struct darwin_task
{
	darwin_task(pid_t p) : pid(p) {}
	pid_t pid;
};

typedef darwin_task* darwin_task_t;

#ifdef __cplusplus
extern "C"
{
#endif
	extern darwin_task_t mach_task_self_;
	inline darwin_task_t mach_task_self() { return mach_task_self_; }
	kern_return_t mach_port_deallocate(darwin_task_t task, void* port);
#ifdef __cplusplus
}
#endif

#endif
