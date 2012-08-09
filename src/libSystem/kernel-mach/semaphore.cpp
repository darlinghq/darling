#include "config.h"
#include <semaphore.h>
#include "semaphore.h"
#include "mach-stub.h"
#include "trace.h"
#include <unistd.h>
#include <errno.h>

kern_return_t semaphore_create(darwin_task_t* task, esemaphore_t *semaphore, int policy, int value)
{
	TRACE4(task, semaphore, policy, value);
	CHECK_TASK_SELF(task);

	if (!semaphore)
		return KERN_INVALID_ARGUMENT;

	*semaphore = new struct semaphore;
	if (!*semaphore)
		return KERN_RESOURCE_SHORTAGE;

	if (::sem_init(&(*semaphore)->sem, 0, value) == -1)
	{
		if (errno == EINVAL)
			return KERN_INVALID_ARGUMENT;
		else if (errno == ENOMEM)
			return KERN_RESOURCE_SHORTAGE;
		else
			return KERN_FAILURE;
	}
	return KERN_SUCCESS;
}

kern_return_t semaphore_destroy(darwin_task_t* task, esemaphore_t semaphore)
{
	TRACE2(task, semaphore);
	CHECK_TASK_SELF(task);

	if (!semaphore)
		return KERN_INVALID_ARGUMENT;

	if (::sem_destroy(&semaphore->sem) == -1)
		return KERN_INVALID_ARGUMENT;

	delete semaphore;
	return KERN_SUCCESS;
}

kern_return_t semaphore_signal(esemaphore_t semaphore)
{
	TRACE1(semaphore);
	if (!semaphore)
		return KERN_INVALID_ARGUMENT;

	if (::sem_post(&semaphore->sem) == -1)
		return KERN_INVALID_ARGUMENT;

	return KERN_SUCCESS;
}

kern_return_t semaphore_signal_all(esemaphore_t semaphore)
{
	MACH_STUB();
}

kern_return_t semaphore_wait(esemaphore_t semaphore)
{
	TRACE1(semaphore);
	if (!semaphore)
		return KERN_INVALID_ARGUMENT;

	if (::sem_wait(&semaphore->sem) == -1)
	{
		if (errno == EINVAL)
			return KERN_INVALID_ARGUMENT;
		else if (errno == EDEADLK)
			return KERN_OPERATION_TIMED_OUT; // the closest we can get
		else
			return KERN_FAILURE;
	}
	return KERN_SUCCESS;
}

