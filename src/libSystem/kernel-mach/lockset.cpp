#include "config.h"
#include "lockset.h"
#include <cstring>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include "mach-stub.h"
#include "trace.h"

struct lock_set
{
	int lock_count;
	pthread_mutex_t* mutexes;
	pthread_t* anonymous_recipients;
};

kern_return_t lock_acquire(lock_set_t lock_set, int lock_id)
{
	TRACE2(lock_set, lock_id);
	if (!lock_set || lock_id >= lock_set->lock_count)
		return KERN_INVALID_ARGUMENT;
	
	if (::pthread_mutex_lock(lock_set->mutexes+lock_id))
		return KERN_FAILURE;
	
	return KERN_SUCCESS;
}

kern_return_t lock_handoff(lock_set_t lock_set, int lock_id)
{
	if (!lock_set || lock_id >= lock_set->lock_count)
		return KERN_INVALID_ARGUMENT;
	
	MACH_STUB();
}

kern_return_t lock_handoff_accept(lock_set_t lock_set, int lock_id)
{
	MACH_STUB();
}

kern_return_t lock_make_stable(lock_set_t lock_set, int lock_id)
{
	MACH_STUB();
}

kern_return_t lock_release(lock_set_t lock_set, int lock_id)
{
	TRACE2(lock_set, lock_id);
	
	if (!lock_set || lock_id >= lock_set->lock_count)
		return KERN_INVALID_ARGUMENT;
	
	if (::pthread_mutex_unlock(lock_set->mutexes+lock_id))
	{
		if (errno == EPERM)
			return KERN_INVALID_RIGHT;
		else
			return KERN_FAILURE;
	}
	
	return KERN_SUCCESS;
}

kern_return_t lock_set_create(darwin_task_t task, lock_set_t* lockset, int locks, int policy)
{
	*lockset = 0;
	
	TRACE4(task, lockset, locks, policy);
	CHECK_TASK_SELF(task);
	
	if (locks <= 0 || !lockset)
		return KERN_INVALID_ARGUMENT;
	
	*lockset = new lock_set;
	if (!*lockset)
		return KERN_RESOURCE_SHORTAGE;
	
	(*lockset)->lock_count = locks;
	(*lockset)->mutexes = 0;
	(*lockset)->anonymous_recipients = 0;
	
	(*lockset)->mutexes = new pthread_mutex_t[locks];
	if (!(*lockset)->mutexes)
		return KERN_RESOURCE_SHORTAGE;
	
	::memset((*lockset)->mutexes, 0, sizeof(pthread_mutex_t)*locks);
	
	(*lockset)->anonymous_recipients = new pthread_t[locks];
	if (!(*lockset)->anonymous_recipients)
	{
		lock_set_destroy(mach_task_self(), *lockset);
		return KERN_RESOURCE_SHORTAGE;
	}
	
	::memset((*lockset)->anonymous_recipients, 0, sizeof(pthread_t) * locks);
	
	for (int i = 0; i < locks; i++)
	{
		if (::pthread_mutex_init((*lockset)->mutexes+i, 0))
		{
			lock_set_destroy(mach_task_self(), *lockset);
			return KERN_RESOURCE_SHORTAGE;
		}
	}
	
	return KERN_SUCCESS;
}

kern_return_t lock_set_destroy (darwin_task_t task, lock_set_t lockset)
{
	TRACE2(task, lockset);
	CHECK_TASK_SELF(task);
	
	if (!lockset)
		return KERN_INVALID_ARGUMENT;
	
	if (lockset->mutexes)
	{
		for (int i = 0; i < lockset->lock_count; i++)
		{
			//if (lockset->mutexes[i])
				::pthread_mutex_destroy(lockset->mutexes+i);
		}
	}
	
	delete [] lockset->mutexes;
	delete [] lockset->anonymous_recipients;
	delete lockset;
	
	return KERN_SUCCESS;
}

kern_return_t lock_try (lock_set_t lock_set, int lock_id)
{
	TRACE2(lock_set, lock_id);
	if (!lock_set || lock_id >= lock_set->lock_count)
		return KERN_INVALID_ARGUMENT;
	
	if (::pthread_mutex_trylock(lock_set->mutexes+lock_id))
	{
		if (errno == EBUSY)
			return KERN_LOCK_OWNED;
		return KERN_FAILURE;
	}
	
	return KERN_SUCCESS;
}

