#ifndef MACH_SEMAPHORE_H
#define MACH_SEMAPHORE_H
#include <semaphore.h>
#include "task.h"
#include <mach/kern_return.h>

struct semaphore
{
	sem_t sem;
};

typedef semaphore* esemaphore_t;

extern "C"
{

kern_return_t semaphore_create(darwin_task_t task, esemaphore_t *semaphore, int policy, int value);

kern_return_t semaphore_destroy(darwin_task_t task, esemaphore_t semaphore);

kern_return_t semaphore_signal(esemaphore_t semaphore);

kern_return_t semaphore_signal_all(esemaphore_t semaphore);

kern_return_t semaphore_wait(esemaphore_t semaphore);

}

#endif

