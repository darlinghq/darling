#include <stdatomic.h>
#include <limits.h>
#include <stdbool.h>

#include "lock.h"
#include <darling/emulation/simple.h>

#ifndef XTRACE_LOCK_DEBUG
	#define XTRACE_LOCK_DEBUG 0
#endif

#if XTRACE_LOCK_DEBUG
	#define xtrace_lock_debug_internal(x, ...) xtrace_printf(x "\n", ## __VA_ARGS__)
	#undef XTRACE_INLINE
	#define XTRACE_INLINE
#else
	#define xtrace_lock_debug_internal(x, ...)
#endif

#define xtrace_lock_debug(x, ...) xtrace_lock_debug_internal("lock %p: " x, lock, ## __VA_ARGS__)
#define xtrace_once_debug(x, ...) xtrace_lock_debug_internal("once %p: " x, once, ## __VA_ARGS__)

//
// xtrace_lock
//
// simple lock implementation on top of futexes, based on https://github.com/eliben/code-for-blog/blob/master/2018/futex-basics/mutex-using-futex.cpp
//

// used to hide the use of C11 atomics from external code (e.g. C++ code, which can't use C11 atomics).
// it's *technically* undefined behavior to cast the external one to this one, but that's okay;
// we know our exact environment and it works in it
typedef struct xtrace_lock_internal {
	_Atomic uint32_t state;
} xtrace_lock_internal_t;

enum xtrace_lock_state {
	// the lock is unlocked (duh)
	xtrace_lock_state_unlocked = 0,

	// the lock is locked, but no one is waiting for it
	xtrace_lock_state_locked_uncontended = 1,

	// the locked is locked and there are waiters
	xtrace_lock_state_locked_contended = 2,
};

XTRACE_INLINE
uint32_t cmpxchg_wrapper_u32(_Atomic uint32_t* atom, uint32_t expected, uint32_t desired) {
	atomic_compare_exchange_strong_explicit(atom, &expected, desired, memory_order_acq_rel, memory_order_acquire);
	return expected;
};

void xtrace_lock_lock(xtrace_lock_t* _lock) {
	xtrace_lock_internal_t* lock = (xtrace_lock_internal_t*)_lock;

	xtrace_lock_debug("locking");

	// we do the initial exchange using `xtrace_lock_state_locked_uncontended` because
	// if it was previously unlocked, it is now locked but nobody is waiting for it
	uint32_t prev = cmpxchg_wrapper_u32(&lock->state, xtrace_lock_state_unlocked, xtrace_lock_state_locked_uncontended);

	xtrace_lock_debug("previous state was %u", prev);

	// if it was not unlocked, we need to do some waiting
	if (prev != xtrace_lock_state_unlocked) {
		do {
			// we can skip the cmpxchg if the lock was already contended
			// also, when we do the cmpxchg, we need to make sure it's still locked
			// we use `xtrace_lock_state_locked_contended` because it was either uncontended (but it is now) or it was already contended
			if (prev == xtrace_lock_state_locked_contended || cmpxchg_wrapper_u32(&lock->state, xtrace_lock_state_locked_uncontended, xtrace_lock_state_locked_contended) != xtrace_lock_state_unlocked) {
				xtrace_lock_debug("going to wait");
				// do the actual sleeping
				// we expect `xtrace_lock_state_locked_contended` because we don't want to sleep if it's not contended
				__linux_futex(&lock->state, FUTEX_WAIT, xtrace_lock_state_locked_contended, NULL, 0, 0);
				xtrace_lock_debug("awoken");
			}

			// loop as long as the lock was not unlocked
			// we use `xtrace_lock_state_locked_contended` for the same reason as before
		} while ((prev = cmpxchg_wrapper_u32(&lock->state, xtrace_lock_state_unlocked, xtrace_lock_state_locked_contended)) != xtrace_lock_state_unlocked);
		xtrace_lock_debug("lock acquired after sleeping");
	}

	xtrace_lock_debug("lock acquired");
};

void xtrace_lock_unlock(xtrace_lock_t* _lock) {
	xtrace_lock_internal_t* lock = (xtrace_lock_internal_t*)_lock;

	xtrace_lock_debug("unlocking");

	uint32_t prev = atomic_exchange_explicit(&lock->state, xtrace_lock_state_unlocked, memory_order_acq_rel);

	xtrace_lock_debug("previous state was %u", prev);

	// if it was previously contended, then we need to wake someone up
	if (prev == xtrace_lock_state_locked_contended) {
		xtrace_lock_debug("waking someone up");
		__linux_futex(&lock->state, FUTEX_WAKE, 1, NULL, 0, 0);
	}
};

//
// xtrace_once
//
// see https://github.com/bugaevc/serenity/blob/1fc3f2ba84d9c75c6c30e62014dabe4fcd62aae1/Libraries/LibPthread/pthread_once.cpp
//

typedef struct xtrace_once_internal {
	_Atomic uint32_t state;
} xtrace_once_internal_t;

enum xtrace_once_state {
	// the once hasn't been triggered yet
	xtrace_once_state_untriggered = 0,

	// the once has been triggered, but no one is waiting for it
	xtrace_once_state_triggered_uncontended = 1,
	
	// the once has been triggered and there are waiters
	xtrace_once_state_triggered_contended = 2,

	// the once has been triggered and has completed execution
	xtrace_once_state_completed = 3,
};

void xtrace_once(xtrace_once_t* _once, xtrace_once_callback callback) {
	xtrace_once_internal_t* once = (xtrace_once_internal_t*)_once;

	xtrace_once_debug("evaluating...");

	uint32_t prev = xtrace_once_state_untriggered;
	bool exchanged = atomic_compare_exchange_strong_explicit(&once->state, &prev, xtrace_once_state_triggered_uncontended, memory_order_acq_rel, memory_order_acquire);

	xtrace_once_debug("previous state was %u", prev);

	if (exchanged) {
		xtrace_once_debug("performing callback...");

		// we had `xtrace_once_state_untriggered` and now we have `xtrace_once_state_triggered_uncontended`, so let's do the callback
		callback();

		xtrace_once_debug("callback done; updating state...");

		// now let's update the state to tell everyone we're done
		prev = atomic_exchange_explicit(&once->state, xtrace_once_state_completed, memory_order_acq_rel);

		xtrace_once_debug("previous state was %u", prev);

		switch (prev) {
			// no one was waiting, so we have no one to wake up
			case xtrace_once_state_triggered_uncontended: break;
			case xtrace_once_state_triggered_contended: {
				xtrace_once_debug("waking up all waiters...");
				// otherwise, we have to wake someone up
				__linux_futex(&once->state, FUTEX_WAKE, INT_MAX, NULL, 0, 0);
			} break;
		}

		return;
	}

	xtrace_once_debug("someone else is performing the callback");

	while (true) {
		switch (prev) {
			case xtrace_once_state_completed: {
				// we're done
				xtrace_once_debug("callback completed by someone else; returning...");
				return;
			}; // not reached
			case xtrace_once_state_triggered_uncontended: {
				// somebody is already perfoming the callback;
				// now we're waiting, so let's update the state for that
				xtrace_once_debug("someone is already performing the callback with no waiters; becoming first waiter...");
				exchanged = atomic_compare_exchange_strong_explicit(&once->state, &prev, xtrace_once_state_triggered_contended, memory_order_acq_rel, memory_order_acquire);
				if (!exchanged) {
					xtrace_once_debug("state changed from uncontended; re-evaluating...");
					// something changed, let's re-evaluate
					continue;
				}
				prev = xtrace_once_state_triggered_contended;
			}; // fall through
			case xtrace_once_state_triggered_contended: {
				// somebody is already performing the callback and there are already waiters;
				// let's wait
				xtrace_once_debug("someone is already performing the callback with waiters; going to wait...");
				__linux_futex(&once->state, FUTEX_WAIT, prev, NULL, 0, 0);

				xtrace_once_debug("woken up");

				// we got woken up, but that may have been spurious;
				// let's loop again to re-evaluate
				prev = atomic_load_explicit(&once->state, memory_order_acquire);
				xtrace_once_debug("previous state was %u; going to re-evaluate...", prev);
			} break;
		}
	}
};
