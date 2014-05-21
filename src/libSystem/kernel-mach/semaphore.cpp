#include "config.h"
#include <semaphore.h>
#include "semaphore.h"
#include "mach-stub.h"
#include "trace.h"
#include <unistd.h>
#include <errno.h>
#include "FutexSemaphore.h"

kern_return_t semaphore_create(darwin_task_t task, esemaphore_t *semaphore, int policy, int value)
{
	TRACE4(task, semaphore, policy, value);
	CHECK_TASK_SELF(task);
	
	if (value < 0)
		return KERN_INVALID_ARGUMENT;

	if (!semaphore)
		return KERN_INVALID_ARGUMENT;

	*semaphore = new struct semaphore;
	if (!*semaphore)
		return KERN_RESOURCE_SHORTAGE;
	
	(*semaphore)->sem = new Darling::FutexSemaphore(value);

	return KERN_SUCCESS;
}

kern_return_t semaphore_destroy(darwin_task_t task, esemaphore_t semaphore)
{
	TRACE2(task, semaphore);
	CHECK_TASK_SELF(task);

	if (!semaphore)
		return KERN_INVALID_ARGUMENT;

	delete semaphore->sem;
	delete semaphore;
	return KERN_SUCCESS;
}

kern_return_t semaphore_signal(esemaphore_t semaphore)
{
	TRACE1(semaphore);
	if (!semaphore)
		return KERN_INVALID_ARGUMENT;

	semaphore->sem->signal();

	return KERN_SUCCESS;
}

kern_return_t semaphore_signal_all(esemaphore_t semaphore)
{
	TRACE1(semaphore);
	if (!semaphore)
		return KERN_INVALID_ARGUMENT;
	
	semaphore->sem->signalAll();
	
	return KERN_SUCCESS;
}

kern_return_t semaphore_wait(esemaphore_t semaphore)
{
	TRACE1(semaphore);
	if (!semaphore)
		return KERN_INVALID_ARGUMENT;
	
	semaphore->sem->wait();

	return KERN_SUCCESS;
}

kern_return_t semaphore_timedwait(esemaphore_t semaphore, mach_timespec_t ts)
{
	STUB();
	return semaphore_wait(semaphore);
}

