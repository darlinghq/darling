#ifndef SEMAPHORE_H
#define SEMAPHORE_H
#include "task.h"
#include <semaphore.h>

struct semaphore
{
	sem_t sem;
};

typedef semaphore* temaphore_t;

extern "C"
{

kern_return_t semaphore_create(task_t task, semaphore_t *semaphore, int policy, int value);

kern_return_t semaphore_destroy(task_t task, semaphore_t semaphore);

kern_return_t semaphore_signal(semaphore_t semaphore);

kern_return_t semaphore_signal_all(semaphore_t semaphore);

kern_return_t semaphore_wait(semaphore_t semaphore);

}

#endif

