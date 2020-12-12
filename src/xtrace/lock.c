#include <stdatomic.h>

#include "lock.h"
#include "simple.h"

#ifndef XTRACE_LOCK_DEBUG
	#define XTRACE_LOCK_DEBUG 0
#endif

#if XTRACE_LOCK_DEBUG
	#define xtrace_lock_debug(x, ...) __simple_printf(x "\n", ## __VA_ARGS__)
	#undef XTRACE_INLINE
	#define XTRACE_INLINE
#else
	#define xtrace_lock_debug(x, ...)
#endif

// simple lock implementation on top of futexes, based on https://github.com/eliben/code-for-blog/blob/master/2018/futex-basics/mutex-using-futex.cpp

// used to hide the use of C11 atomics from external code (e.g. C++ code, which can't use C11 atomics).
// it's *technically* undefined behavior to cast the external one to this one, but that's okay;
// we know our exact environment and it works in it
typedef struct xtrace_lock_internal {
	_Atomic uint32_t state;
} xtrace_lock_internal_t;

typedef struct xtrace_once_internal {
	_Atomic uint8_t state;
} xtrace_once_internal_t;

enum xtrace_lock_state {
	// the lock is unlocked (duh)
	xtrace_lock_state_unlocked = 0,

	// the lock is locked, but no one is waiting for it
	xtrace_lock_state_locked_uncontended = 1,

	// the locked is locked and there are waiters
	xtrace_lock_state_locked_contended = 2,
};

enum xtrace_once_state {
	// the once hasn't been run yet
	xtrace_once_state_untriggered = 0,

	// the once has already been run at least once
	xtrace_once_state_triggered = 1,
};

void xtrace_lock_lock(xtrace_lock_t* _lock) {
	xtrace_lock_internal_t* lock = (xtrace_lock_internal_t*)_lock;

	xtrace_lock_debug("lock %p: locking", lock);

	// we do the initial exchange using `xtrace_lock_state_locked_uncontended` because
	// if it was previously unlocked, it is now locked but nobody is waiting for it
	uint32_t prev = atomic_exchange(&lock->state, xtrace_lock_state_locked_uncontended);

	xtrace_lock_debug("lock %p: previous state was %u", lock, prev);

	// if it was not unlocked, we need to do some waiting
	if (prev != xtrace_lock_state_unlocked) {
		do {
			// we can skip the atomic_exchange if the lock was already contended
			// also, when we do the atomic_exchange, we need to make sure it's still locked
			// we use `xtrace_lock_state_locked_contended` because it was either uncontended (but it is now) or it was already contended
			if (prev == xtrace_lock_state_locked_contended || atomic_exchange(&lock->state, xtrace_lock_state_locked_contended) != xtrace_lock_state_unlocked) {
				xtrace_lock_debug("lock %p: going to wait", lock);
				// do the actual sleeping
				// we expect `xtrace_lock_state_locked_contended` because we don't want to sleep if it's not contended
				__linux_futex(&lock->state, FUTEX_WAIT, xtrace_lock_state_locked_contended, NULL, 0, 0);
				xtrace_lock_debug("lock %p: awoken", lock);
			}

			// loop as long as the lock was not unlocked
			// we use `xtrace_lock_state_locked_contended` for the same reason as before
		} while ((prev = atomic_exchange(&lock->state, xtrace_lock_state_locked_contended)) != xtrace_lock_state_unlocked);
		xtrace_lock_debug("lock %p: lock acquired after sleeping", lock);
	}

	xtrace_lock_debug("lock %p: lock acquired", lock);
};

void xtrace_lock_unlock(xtrace_lock_t* _lock) {
	xtrace_lock_internal_t* lock = (xtrace_lock_internal_t*)_lock;

	xtrace_lock_debug("lock %p: unlocking", lock);

	uint32_t prev = atomic_exchange(&lock->state, xtrace_lock_state_unlocked);

	xtrace_lock_debug("lock %p: previous state was %u", prev);

	// if it was previously contended, then we need to wake someone up
	if (prev == xtrace_lock_state_locked_contended) {
		xtrace_lock_debug("lock %p: waking someone up", lock);
		__linux_futex(&lock->state, FUTEX_WAKE, 1, NULL, 0, 0);
	}
};

void xtrace_once(xtrace_once_t* _once, xtrace_once_callback callback) {
	xtrace_lock_internal_t* once = (xtrace_lock_internal_t*)_once;
	if (atomic_exchange(&once->state, xtrace_once_state_triggered) == xtrace_once_state_untriggered) {
		callback();
	}
};
