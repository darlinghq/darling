#include <stdatomic.h>
#include <limits.h>
#include <stdbool.h>

#include <libsimple/lock.h>

#ifndef LIBSIMPLE_LOCK_DEBUG
	#define LIBSIMPLE_LOCK_DEBUG 0
#endif

#if LIBSIMPLE_LOCK_DEBUG
	#define libsimple_lock_debug_internal(x, ...) libsimple_log(x "\n", ## __VA_ARGS__)
	#undef LIBSIMPLE_INLINE
	#define LIBSIMPLE_INLINE
#else
	#define libsimple_lock_debug_internal(x, ...)
#endif

#define libsimple_lock_debug(x, ...) libsimple_lock_debug_internal("lock %p: " x, lock, ## __VA_ARGS__)
#define libsimple_once_debug(x, ...) libsimple_lock_debug_internal("once %p: " x, once, ## __VA_ARGS__)

#define FUTEX_WAIT 0
#define FUTEX_WAKE 1

struct timespec;

#if LIBSIMPLE_LINUX
	#include <sys/syscall.h>
	#include <unistd.h>
#endif

static int linux_futex(int* uaddr, int op, int val, const struct timespec* timeout, int* uaddr2, int val3) {
#if LIBSIMPLE_LINUX
	return syscall(SYS_futex, uaddr, op, val, timeout, uaddr2, val3);
#else
	#error linux_futex not implemented for this platform
#endif
};

//
// libsimple_lock
//
// simple lock implementation on top of futexes, based on https://github.com/eliben/code-for-blog/blob/master/2018/futex-basics/mutex-using-futex.cpp
//

// used to hide the use of C11 atomics from external code (e.g. C++ code, which can't use C11 atomics).
// it's *technically* undefined behavior to cast the external one to this one, but that's okay;
// we know our exact environment and it works in it
typedef struct libsimple_lock_internal {
	_Atomic uint32_t state;
} libsimple_lock_internal_t;

enum libsimple_lock_state {
	// the lock is unlocked (duh)
	libsimple_lock_state_unlocked = 0,

	// the lock is locked, but no one is waiting for it
	libsimple_lock_state_locked_uncontended = 1,

	// the locked is locked and there are waiters
	libsimple_lock_state_locked_contended = 2,
};

LIBSIMPLE_INLINE
uint32_t cmpxchg_wrapper_u32(_Atomic uint32_t* atom, uint32_t expected, uint32_t desired) {
	atomic_compare_exchange_strong_explicit(atom, &expected, desired, memory_order_acq_rel, memory_order_acquire);
	return expected;
};

void libsimple_lock_lock(libsimple_lock_t* _lock) {
	libsimple_lock_internal_t* lock = (libsimple_lock_internal_t*)_lock;

	libsimple_lock_debug("locking");

	// we do the initial exchange using `libsimple_lock_state_locked_uncontended` because
	// if it was previously unlocked, it is now locked but nobody is waiting for it
	uint32_t prev = cmpxchg_wrapper_u32(&lock->state, libsimple_lock_state_unlocked, libsimple_lock_state_locked_uncontended);

	libsimple_lock_debug("previous state was %u", prev);

	// if it was not unlocked, we need to do some waiting
	if (prev != libsimple_lock_state_unlocked) {
		do {
			// we can skip the cmpxchg if the lock was already contended
			// also, when we do the cmpxchg, we need to make sure it's still locked
			// we use `libsimple_lock_state_locked_contended` because it was either uncontended (but it is now) or it was already contended
			if (prev == libsimple_lock_state_locked_contended || cmpxchg_wrapper_u32(&lock->state, libsimple_lock_state_locked_uncontended, libsimple_lock_state_locked_contended) != libsimple_lock_state_unlocked) {
				libsimple_lock_debug("going to wait");
				// do the actual sleeping
				// we expect `libsimple_lock_state_locked_contended` because we don't want to sleep if it's not contended
				linux_futex((int*)&lock->state, FUTEX_WAIT, libsimple_lock_state_locked_contended, NULL, 0, 0);
				libsimple_lock_debug("awoken");
			}

			// loop as long as the lock was not unlocked
			// we use `libsimple_lock_state_locked_contended` for the same reason as before
		} while ((prev = cmpxchg_wrapper_u32(&lock->state, libsimple_lock_state_unlocked, libsimple_lock_state_locked_contended)) != libsimple_lock_state_unlocked);
		libsimple_lock_debug("lock acquired after sleeping");
	}

	libsimple_lock_debug("lock acquired");
};

void libsimple_lock_unlock(libsimple_lock_t* _lock) {
	libsimple_lock_internal_t* lock = (libsimple_lock_internal_t*)_lock;

	libsimple_lock_debug("unlocking");

	uint32_t prev = atomic_exchange_explicit(&lock->state, libsimple_lock_state_unlocked, memory_order_acq_rel);

	libsimple_lock_debug("previous state was %u", prev);

	// if it was previously contended, then we need to wake someone up
	if (prev == libsimple_lock_state_locked_contended) {
		libsimple_lock_debug("waking someone up");
		linux_futex((int*)&lock->state, FUTEX_WAKE, 1, NULL, 0, 0);
	}
};

//
// libsimple_once
//
// see https://github.com/bugaevc/serenity/blob/1fc3f2ba84d9c75c6c30e62014dabe4fcd62aae1/Libraries/LibPthread/pthread_once.cpp
//

typedef struct libsimple_once_internal {
	_Atomic uint32_t state;
} libsimple_once_internal_t;

enum libsimple_once_state {
	// the once hasn't been triggered yet
	libsimple_once_state_untriggered = 0,

	// the once has been triggered, but no one is waiting for it
	libsimple_once_state_triggered_uncontended = 1,
	
	// the once has been triggered and there are waiters
	libsimple_once_state_triggered_contended = 2,

	// the once has been triggered and has completed execution
	libsimple_once_state_completed = 3,
};

void libsimple_once(libsimple_once_t* _once, libsimple_once_callback callback) {
	libsimple_once_internal_t* once = (libsimple_once_internal_t*)_once;

	libsimple_once_debug("evaluating...");

	uint32_t prev = libsimple_once_state_untriggered;
	bool exchanged = atomic_compare_exchange_strong_explicit(&once->state, &prev, libsimple_once_state_triggered_uncontended, memory_order_acq_rel, memory_order_acquire);

	libsimple_once_debug("previous state was %u", prev);

	if (exchanged) {
		libsimple_once_debug("performing callback...");

		// we had `libsimple_once_state_untriggered` and now we have `libsimple_once_state_triggered_uncontended`, so let's do the callback
		callback();

		libsimple_once_debug("callback done; updating state...");

		// now let's update the state to tell everyone we're done
		prev = atomic_exchange_explicit(&once->state, libsimple_once_state_completed, memory_order_acq_rel);

		libsimple_once_debug("previous state was %u", prev);

		switch (prev) {
			// no one was waiting, so we have no one to wake up
			case libsimple_once_state_triggered_uncontended: break;
			case libsimple_once_state_triggered_contended: {
				libsimple_once_debug("waking up all waiters...");
				// otherwise, we have to wake someone up
				linux_futex((int*)&once->state, FUTEX_WAKE, INT_MAX, NULL, 0, 0);
			} break;
		}

		return;
	}

	libsimple_once_debug("someone else is performing the callback");

	while (true) {
		switch (prev) {
			case libsimple_once_state_completed: {
				// we're done
				libsimple_once_debug("callback completed by someone else; returning...");
				return;
			}; // not reached
			case libsimple_once_state_triggered_uncontended: {
				// somebody is already perfoming the callback;
				// now we're waiting, so let's update the state for that
				libsimple_once_debug("someone is already performing the callback with no waiters; becoming first waiter...");
				exchanged = atomic_compare_exchange_strong_explicit(&once->state, &prev, libsimple_once_state_triggered_contended, memory_order_acq_rel, memory_order_acquire);
				if (!exchanged) {
					libsimple_once_debug("state changed from uncontended; re-evaluating...");
					// something changed, let's re-evaluate
					continue;
				}
				prev = libsimple_once_state_triggered_contended;
			}; // fall through
			case libsimple_once_state_triggered_contended: {
				// somebody is already performing the callback and there are already waiters;
				// let's wait
				libsimple_once_debug("someone is already performing the callback with waiters; going to wait...");
				linux_futex((int*)&once->state, FUTEX_WAIT, prev, NULL, 0, 0);

				libsimple_once_debug("woken up");

				// we got woken up, but that may have been spurious;
				// let's loop again to re-evaluate
				prev = atomic_load_explicit(&once->state, memory_order_acquire);
				libsimple_once_debug("previous state was %u; going to re-evaluate...", prev);
			} break;
		}
	}
};
