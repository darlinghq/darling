#include "config.h"
#include "lockset.h"
#include <cstring>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include "mach-stub.h"
#include "trace.h"
#include "Futex.h"

struct lock_set
{
	int lock_count;
	Darling::Futex* futexes;
};

kern_return_t lock_acquire(lock_set_t lock_set, int lock_id)
{
	TRACE2(lock_set, lock_id);
	if (!lock_set || lock_id >= lock_set->lock_count)
		return KERN_INVALID_ARGUMENT;
	
	lock_set->futexes[lock_id].acquire();
	
	return KERN_SUCCESS;
}

kern_return_t lock_handoff(lock_set_t lock_set, int lock_id)
{
	if (!lock_set || lock_id >= lock_set->lock_count)
		return KERN_INVALID_ARGUMENT;
	
	lock_set->futexes[lock_id].handoff();
	
	return KERN_SUCCESS;
}

kern_return_t lock_handoff_accept(lock_set_t lock_set, int lock_id)
{
	if (!lock_set || lock_id >= lock_set->lock_count)
		return KERN_INVALID_ARGUMENT;
	
	if (!lock_set->futexes[lock_id].handoff_accept())
		return KERN_ALREADY_WAITING;
	else
		return KERN_SUCCESS;
}

kern_return_t lock_make_stable(lock_set_t lock_set, int lock_id)
{
	// Whatever the hell is an unstable lock
	return KERN_SUCCESS;
}

kern_return_t lock_release(lock_set_t lock_set, int lock_id)
{
	TRACE2(lock_set, lock_id);
	
	if (!lock_set || lock_id >= lock_set->lock_count)
		return KERN_INVALID_ARGUMENT;
	
	lock_set->futexes[lock_id].release();
	
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
	
	(*lockset)->futexes = new Darling::Futex[locks];
	if (!(*lockset)->futexes)
		return KERN_RESOURCE_SHORTAGE;
	
	return KERN_SUCCESS;
}

kern_return_t lock_set_destroy (darwin_task_t task, lock_set_t lockset)
{
	TRACE2(task, lockset);
	CHECK_TASK_SELF(task);
	
	if (!lockset)
		return KERN_INVALID_ARGUMENT;
	
	delete [] lockset->futexes;
	delete lockset;
	
	return KERN_SUCCESS;
}

kern_return_t lock_try (lock_set_t lock_set, int lock_id)
{
	TRACE2(lock_set, lock_id);
	if (!lock_set || lock_id >= lock_set->lock_count)
		return KERN_INVALID_ARGUMENT;
	
	Darling::Futex::TryAcquireResult result = lock_set->futexes[lock_id].try_acquire();
	if (result == Darling::Futex::ResultAlreadyOwned)
		return KERN_LOCK_OWNED_SELF;
	else if (result == Darling::Futex::ResultLocked)
		return KERN_LOCK_OWNED;
	else
		return KERN_SUCCESS;
}

