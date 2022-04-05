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
#define FUTEX_WAIT_BITSET 9
#define FUTEX_WAKE_BITSET 10
#define FUTEX_PRIVATE_FLAG 128

struct timespec;

#if LIBSIMPLE_LINUX
	#include <sys/syscall.h>
	#include <unistd.h>
#elif LIBSIMPLE_DARLING
	extern int __linux_futex(int* uaddr, int op, int val, const struct timespec* timeout, int* uaddr2, int val3);
#endif

#if LIBSIMPLE_DARLING
	#define linux_futex __linux_futex
#else // !LIBSIMPLE_DARLING
static int linux_futex(int* uaddr, int op, int val, const struct timespec* timeout, int* uaddr2, int val3) {
#if LIBSIMPLE_LINUX
	return syscall(SYS_futex, uaddr, op, val, timeout, uaddr2, val3);
#else
	#error linux_futex not implemented for this platform
#endif
};
#endif // !LIBSIMPLE_DARLING

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
				linux_futex((int*)&lock->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, libsimple_lock_state_locked_contended, NULL, 0, 0);
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
		linux_futex((int*)&lock->state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 1, NULL, 0, 0);
	}
};

bool libsimple_lock_try_lock(libsimple_lock_t* _lock) {
	libsimple_lock_internal_t* lock = (libsimple_lock_internal_t*)_lock;
	uint32_t prev = cmpxchg_wrapper_u32(&lock->state, libsimple_lock_state_unlocked, libsimple_lock_state_locked_uncontended);

	if (prev == libsimple_lock_state_unlocked) {
		libsimple_lock_debug("(try) lock acquired");
	} else {
		libsimple_lock_debug("(try) lock not acquired");
	}

	return prev == libsimple_lock_state_unlocked;
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

void libsimple_once(libsimple_once_t* _once, libsimple_once_callback callback, void* context) {
	libsimple_once_internal_t* once = (libsimple_once_internal_t*)_once;

	libsimple_once_debug("evaluating...");

	uint32_t prev = libsimple_once_state_untriggered;
	bool exchanged = atomic_compare_exchange_strong_explicit(&once->state, &prev, libsimple_once_state_triggered_uncontended, memory_order_acq_rel, memory_order_acquire);

	libsimple_once_debug("previous state was %u", prev);

	if (exchanged) {
		libsimple_once_debug("performing callback...");

		// we had `libsimple_once_state_untriggered` and now we have `libsimple_once_state_triggered_uncontended`, so let's do the callback
		callback(context);

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
				linux_futex((int*)&once->state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, INT_MAX, NULL, 0, 0);
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
				linux_futex((int*)&once->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, prev, NULL, 0, 0);

				libsimple_once_debug("woken up");

				// we got woken up, but that may have been spurious;
				// let's loop again to re-evaluate
				prev = atomic_load_explicit(&once->state, memory_order_acquire);
				libsimple_once_debug("previous state was %u; going to re-evaluate...", prev);
			} break;
		}
	}
};

//
// libsimple_rwlock
//
// based on https://github.com/bugaevc/lets-write-sync-primitives/blob/master/src/rwlock.cpp
//

typedef struct libsimple_rwlock_internal {
	_Atomic uint32_t state;
} libsimple_rwlock_internal_t;

enum libsimple_rwlock_bits {
	libsimple_rwlock_bit_reader       = 1U <<  0,
	libsimple_rwlock_bit_writer       = 1U <<  1,
	libsimple_rwlock_bit_locked_write = 1U << 30,
	libsimple_rwlock_bit_need_to_wake = 1U << 31,
};

void libsimple_rwlock_lock_read(libsimple_rwlock_t* _rwlock) {
	libsimple_rwlock_internal_t* rwlock = (void*)_rwlock;
	uint32_t state2 = atomic_load_explicit(&rwlock->state, memory_order_relaxed);

	while (true) {
		// this check ensures that readers sleep when writers are pending (we favor writers)
		if ((state2 & (libsimple_rwlock_bit_locked_write | libsimple_rwlock_bit_need_to_wake)) == 0) {
			// no writers or waiting writers
			if (!atomic_compare_exchange_strong_explicit(&rwlock->state, &state2, state2 + 1, memory_order_acquire, memory_order_relaxed)) {
				// failed to exchange; try again
				continue;
			}

			// successfully exchanged (and acquired lock)
			return;
		}

		// we're going to wait, so let's record that (if it's not already set)
		if ((state2 & libsimple_rwlock_bit_need_to_wake) == 0) {
			if (!atomic_compare_exchange_weak_explicit(&rwlock->state, &state2, libsimple_rwlock_bit_locked_write | libsimple_rwlock_bit_need_to_wake, memory_order_relaxed, memory_order_relaxed)) {
				// try again
				continue;
			}

			state2 = libsimple_rwlock_bit_locked_write | libsimple_rwlock_bit_need_to_wake;
		}

		// now let's wait
		linux_futex((int*)&rwlock->state, FUTEX_WAIT_BITSET | FUTEX_PRIVATE_FLAG, state2, NULL, NULL, libsimple_rwlock_bit_reader);

		// when readers are woken up, we expect to see `0`
		state2 = 0;
	}
};

bool libsimple_rwlock_try_lock_read(libsimple_rwlock_t* _rwlock) {
	libsimple_rwlock_internal_t* rwlock = (void*)_rwlock;
	uint32_t state2 = atomic_load_explicit(&rwlock->state, memory_order_relaxed);

	// if there's a writer waiting, we can't lock for reading.
	// likewise, if there's anyone waiting, then obviously someone else is holding the lock.
	if ((state2 & (libsimple_rwlock_bit_locked_write | libsimple_rwlock_bit_need_to_wake)) != 0) {
		return false;
	}

	return atomic_compare_exchange_strong_explicit(&rwlock->state, &state2, state2 + 1, memory_order_acquire, memory_order_relaxed);
};

void libsimple_rwlock_unlock_read(libsimple_rwlock_t* _rwlock) {
	libsimple_rwlock_internal_t* rwlock = (void*)_rwlock;
	uint32_t state2 = atomic_fetch_sub_explicit(&rwlock->state, 1, memory_order_release);
	// note that `count` is the count before we decremented it
	uint32_t count = state2 & ~libsimple_rwlock_bit_need_to_wake;

	if (count == 1 && (state2 & libsimple_rwlock_bit_need_to_wake) != 0) {
		// wake 1 writer
		state2 = libsimple_rwlock_bit_need_to_wake;
		atomic_compare_exchange_weak_explicit(&rwlock->state, &state2, 0, memory_order_relaxed, memory_order_relaxed);
		linux_futex((int*)&rwlock->state, FUTEX_WAKE_BITSET | FUTEX_PRIVATE_FLAG, 1, NULL, NULL, libsimple_rwlock_bit_writer);
	}
};

void libsimple_rwlock_lock_write(libsimple_rwlock_t* _rwlock) {
	libsimple_rwlock_internal_t* rwlock = (void*)_rwlock;
	uint32_t state2 = 0;

	// try the fast path
	if (atomic_compare_exchange_strong_explicit(&rwlock->state, &state2, libsimple_rwlock_bit_locked_write, memory_order_acquire, memory_order_relaxed)) {
		return;
	}

	while (true) {
		// if there are no readers or writers (but it's okay for there to be waiters for either),
		// then try to acquire the lock
		if ((state2 & ~libsimple_rwlock_bit_need_to_wake) == 0) {
			if (!atomic_compare_exchange_strong_explicit(&rwlock->state, &state2, libsimple_rwlock_bit_locked_write | libsimple_rwlock_bit_need_to_wake, memory_order_acquire, memory_order_relaxed)) {
				// check again
				continue;
			}

			// successfully acquired
			return;
		}

		// we're going to wait, so let's record that (if it's not already set)
		if ((state2 & libsimple_rwlock_bit_need_to_wake) == 0) {
			if (!atomic_compare_exchange_strong_explicit(&rwlock->state, &state2, state2 | libsimple_rwlock_bit_need_to_wake, memory_order_relaxed, memory_order_relaxed)) {
				// try again
				continue;
			}

			state2 |= libsimple_rwlock_bit_need_to_wake;
		}

		// now let's wait
		linux_futex((int*)&rwlock->state, FUTEX_WAIT_BITSET | FUTEX_PRIVATE_FLAG, state2, NULL, NULL, libsimple_rwlock_bit_writer);

		// when writers are woken up, we expect to see either `0` or the need-to-wake bit
		state2 = 0;
	}
};

bool libsimple_rwlock_try_lock_write(libsimple_rwlock_t* _rwlock) {
	libsimple_rwlock_internal_t* rwlock = (void*)_rwlock;
	uint32_t expected = 0;
	return atomic_compare_exchange_strong_explicit(&rwlock->state, &expected, libsimple_rwlock_bit_locked_write, memory_order_acquire, memory_order_relaxed);
};

void libsimple_rwlock_unlock_write(libsimple_rwlock_t* _rwlock) {
	libsimple_rwlock_internal_t* rwlock = (void*)_rwlock;
	uint32_t state2 = atomic_exchange_explicit(&rwlock->state, 0, memory_order_release);

	if ((state2 & libsimple_rwlock_bit_need_to_wake) != 0) {
		// wake all the readers and 1 writer
		linux_futex((int*)&rwlock->state, FUTEX_WAKE_BITSET | FUTEX_PRIVATE_FLAG, INT_MAX, NULL, NULL, libsimple_rwlock_bit_reader);
		linux_futex((int*)&rwlock->state, FUTEX_WAKE_BITSET | FUTEX_PRIVATE_FLAG, 1, NULL, NULL, libsimple_rwlock_bit_writer);
	}
};
