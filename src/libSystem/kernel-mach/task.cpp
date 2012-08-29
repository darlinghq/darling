#include "config.h"
#include "task.h"
#include <sys/types.h>
#include <unistd.h>
#include <cstdlib>

darwin_task_t mach_task_self_ = new darwin_task(::getpid());

kern_return_t mach_port_deallocate(darwin_task_t task, void* port)
{
	free(port);
	return KERN_SUCCESS;
}
