#include "config.h"
#include "task.h"
#include <sys/types.h>
#include <unistd.h>


static darwin_task_t mach_task_self_static = { ::getpid() };
darwin_task_t* mach_task_self_ = &mach_task_self_static;
