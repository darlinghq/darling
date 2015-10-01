#ifndef _TASK_H
#define _TASK_H
#include "ipc_types.h"
#include "ipc_space.h"
#include "ipc_port.h"

struct mach_task
{
	pid_t pid;
	ipc_namespace_t namespace;
	
	// TODO: add host and bootstrap ports
	darling_mach_port_t* task_self;
};

typedef struct mach_task mach_task_t;

mach_task_t*
darling_task_get_current(void);

void
darling_task_set_current(mach_task_t* task);

#endif
