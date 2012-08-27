#include "config.h"
#include "task.h"
#include <sys/types.h>
#include <unistd.h>


darwin_task_t mach_task_self_ = new darwin_task(::getpid());
