#ifndef _TASK_H
#define _TASK_H
#include "ipc_types.h"
#include "ipc_space.h"
#include "ipc_port.h"
#include <linux/thread_info.h>

struct mach_task
{
	pid_t pid;
	ipc_namespace_t namespace;
	
	// TODO: add bootstrap port
	darling_mach_port_t* task_self;
};

typedef struct mach_task mach_task_t;

/* Global initialization */
void
darling_task_init(void);

mach_task_t*
darling_task_get_current(void);

void
darling_task_set_current(mach_task_t* task);

static inline bool
task_is_64bit(void)
{
#if defined(__i386__)
	return false;
#elif defined(__x86_64__)
	return !test_thread_flag(TIF_IA32);
#endif
}

#endif
