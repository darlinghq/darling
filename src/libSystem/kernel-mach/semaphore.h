#ifndef MACH_SEMAPHORE_H
#define MACH_SEMAPHORE_H
#include <semaphore.h>
#include "task.h"
#include <mach/kern_return.h>
#include <mach/clock_types.h>

namespace Darling
{
	class FutexSemaphore;
}

struct semaphore
{
	Darling::FutexSemaphore* sem;
};

typedef semaphore* esemaphore_t;

#ifdef __cplusplus
extern "C"
{
#endif

kern_return_t semaphore_create(darwin_task_t task, esemaphore_t *semaphore, int policy, int value);

kern_return_t semaphore_destroy(darwin_task_t task, esemaphore_t semaphore);

kern_return_t semaphore_signal(esemaphore_t semaphore);

kern_return_t semaphore_signal_all(esemaphore_t semaphore);

kern_return_t semaphore_wait(esemaphore_t semaphore);

kern_return_t semaphore_timedwait(esemaphore_t semaphore, mach_timespec_t ts);

#ifdef __cplusplus
}
#endif

#endif

