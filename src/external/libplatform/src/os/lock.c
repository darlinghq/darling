/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

#include "lock_internal.h"
#include "libkern/OSAtomic.h"
#include "os/lock.h"
#include "os/lock_private.h"
#include "os/once_private.h"
#include "resolver.h"

#include <mach/mach_init.h>
#include <mach/mach_traps.h>
#include <mach/thread_switch.h>
#include <os/tsd.h>

#pragma mark -
#pragma mark _os_lock_base_t

#if !OS_VARIANT_ONLY

OS_LOCK_STRUCT_DECL_INTERNAL(base);
OS_USED static OS_LOCK_TYPE_STRUCT_DECL(base);

void
os_lock_lock(os_lock_t l)
{
	return l._osl_base->osl_type->osl_lock(l);
}

bool
os_lock_trylock(os_lock_t l)
{
	return l._osl_base->osl_type->osl_trylock(l);
}

void
os_lock_unlock(os_lock_t l)
{
	return l._osl_base->osl_type->osl_unlock(l);
}

#endif //!OS_VARIANT_ONLY

OS_NOINLINE OS_NORETURN OS_COLD
static void
_os_lock_corruption_abort(void *lock_ptr OS_UNUSED, uintptr_t lock_value)
{
	__LIBPLATFORM_CLIENT_CRASH__(lock_value, "os_lock is corrupt");
}

#pragma mark -
#pragma mark OSSpinLock

#ifdef OS_LOCK_VARIANT_SELECTOR
void _OSSpinLockLockSlow(volatile OSSpinLock *l);
#else
OS_NOINLINE OS_USED static void _OSSpinLockLockSlow(volatile OSSpinLock *l);
#endif // OS_LOCK_VARIANT_SELECTOR

OS_ATOMIC_EXPORT void OSSpinLockLock(volatile OSSpinLock *l);
OS_ATOMIC_EXPORT bool OSSpinLockTry(volatile OSSpinLock *l);
OS_ATOMIC_EXPORT int spin_lock_try(volatile OSSpinLock *l);
OS_ATOMIC_EXPORT void OSSpinLockUnlock(volatile OSSpinLock *l);

#if OS_ATOMIC_UP
// Don't spin on UP
#elif OS_ATOMIC_WFE
#define OS_LOCK_SPIN_SPIN_TRIES 100
#define OS_LOCK_SPIN_PAUSE() os_hardware_wfe()
#else
#define OS_LOCK_SPIN_SPIN_TRIES 1000
#define OS_LOCK_SPIN_PAUSE() os_hardware_pause()
#endif
#define OS_LOCK_SPIN_YIELD_TRIES 100

static const OSSpinLock _OSSpinLockLocked = TARGET_OS_EMBEDDED ? 1 : -1;

OS_NOINLINE
static void
_OSSpinLockLockYield(volatile OSSpinLock *l)
{
	int option = SWITCH_OPTION_DEPRESS;
	mach_msg_timeout_t timeout = 1;
	uint32_t tries = OS_LOCK_SPIN_YIELD_TRIES;
	OSSpinLock lock;
	while (unlikely(lock = *l)) {
_yield:
		if (unlikely(lock != _OSSpinLockLocked)) {
			_os_lock_corruption_abort((void *)l, (uintptr_t)lock);
		}
		// Yield until tries first hits zero, then permanently switch to wait
		if (unlikely(!tries--)) option = SWITCH_OPTION_WAIT;
		thread_switch(MACH_PORT_NULL, option, timeout);
	}
	bool r = os_atomic_cmpxchgv(l, 0, _OSSpinLockLocked, &lock, acquire);
	if (likely(r)) return;
	goto _yield;
}

#if OS_ATOMIC_UP
void
_OSSpinLockLockSlow(volatile OSSpinLock *l)
{
	return _OSSpinLockLockYield(l); // Don't spin on UP
}
#else
void
_OSSpinLockLockSlow(volatile OSSpinLock *l)
{
	uint32_t tries = OS_LOCK_SPIN_SPIN_TRIES;
	OSSpinLock lock;
	while (unlikely(lock = *l)) {
_spin:
		if (unlikely(lock != _OSSpinLockLocked)) {
			return _os_lock_corruption_abort((void *)l, (uintptr_t)lock);
		}
		if (unlikely(!tries--)) return _OSSpinLockLockYield(l);
		OS_LOCK_SPIN_PAUSE();
	}
	bool r = os_atomic_cmpxchgv(l, 0, _OSSpinLockLocked, &lock, acquire);
	if (likely(r)) return;
	goto _spin;
}
#endif

#ifdef OS_LOCK_VARIANT_SELECTOR
#undef _OSSpinLockLockSlow
extern void _OSSpinLockLockSlow(volatile OSSpinLock *l);
#endif

#if !OS_LOCK_VARIANT_ONLY

#if OS_LOCK_OSSPINLOCK_IS_NOSPINLOCK && !TARGET_OS_SIMULATOR

typedef struct _os_nospin_lock_s *_os_nospin_lock_t;
void _os_nospin_lock_lock(_os_nospin_lock_t lock);
bool _os_nospin_lock_trylock(_os_nospin_lock_t lock);
void _os_nospin_lock_unlock(_os_nospin_lock_t lock);

void
OSSpinLockLock(volatile OSSpinLock *l)
{
	OS_ATOMIC_ALIAS(spin_lock, OSSpinLockLock);
	OS_ATOMIC_ALIAS(_spin_lock, OSSpinLockLock);
	return _os_nospin_lock_lock((_os_nospin_lock_t)l);
}

bool
OSSpinLockTry(volatile OSSpinLock *l)
{
	return _os_nospin_lock_trylock((_os_nospin_lock_t)l);
}

int
spin_lock_try(volatile OSSpinLock *l)
{
	OS_ATOMIC_ALIAS(_spin_lock_try, spin_lock_try);
	return _os_nospin_lock_trylock((_os_nospin_lock_t)l);
}

void
OSSpinLockUnlock(volatile OSSpinLock *l)
{
	OS_ATOMIC_ALIAS(spin_unlock, OSSpinLockUnlock);
	OS_ATOMIC_ALIAS(_spin_unlock, OSSpinLockUnlock);
	return _os_nospin_lock_unlock((_os_nospin_lock_t)l);
}

#undef OS_ATOMIC_ALIAS
#define OS_ATOMIC_ALIAS(n, o)
static void _OSSpinLockLock(volatile OSSpinLock *l);
#undef OSSpinLockLock
#define OSSpinLockLock _OSSpinLockLock
static bool _OSSpinLockTry(volatile OSSpinLock *l);
#undef OSSpinLockTry
#define OSSpinLockTry _OSSpinLockTry
static __unused int __spin_lock_try(volatile OSSpinLock *l);
#undef spin_lock_try
#define spin_lock_try __spin_lock_try
static void _OSSpinLockUnlock(volatile OSSpinLock *l);
#undef OSSpinLockUnlock
#define OSSpinLockUnlock _OSSpinLockUnlock

#endif // OS_LOCK_OSSPINLOCK_IS_NOSPINLOCK

void
OSSpinLockLock(volatile OSSpinLock *l)
{
	OS_ATOMIC_ALIAS(spin_lock, OSSpinLockLock);
	OS_ATOMIC_ALIAS(_spin_lock, OSSpinLockLock);
	bool r = os_atomic_cmpxchg(l, 0, _OSSpinLockLocked, acquire);
	if (likely(r)) return;
	return _OSSpinLockLockSlow(l);
}

bool
OSSpinLockTry(volatile OSSpinLock *l)
{
	bool r = os_atomic_cmpxchg(l, 0, _OSSpinLockLocked, acquire);
	return r;
}

int
spin_lock_try(volatile OSSpinLock *l) // <rdar://problem/13316060>
{
	OS_ATOMIC_ALIAS(_spin_lock_try, spin_lock_try);
	return OSSpinLockTry(l);
}

void
OSSpinLockUnlock(volatile OSSpinLock *l)
{
	OS_ATOMIC_ALIAS(spin_unlock, OSSpinLockUnlock);
	OS_ATOMIC_ALIAS(_spin_unlock, OSSpinLockUnlock);
	os_atomic_store(l, 0, release);
}

#pragma mark -
#pragma mark os_lock_spin_t

OS_LOCK_STRUCT_DECL_INTERNAL(spin,
	OSSpinLock volatile osl_spinlock;
);
#if !OS_VARIANT_ONLY
OS_LOCK_METHODS_DECL(spin);
OS_LOCK_TYPE_INSTANCE(spin);
#endif // !OS_VARIANT_ONLY

#ifdef OS_VARIANT_SELECTOR
#define _os_lock_spin_lock \
		OS_VARIANT(_os_lock_spin_lock, OS_VARIANT_SELECTOR)
#define _os_lock_spin_trylock \
		OS_VARIANT(_os_lock_spin_trylock, OS_VARIANT_SELECTOR)
#define _os_lock_spin_unlock \
		OS_VARIANT(_os_lock_spin_unlock, OS_VARIANT_SELECTOR)
OS_LOCK_METHODS_DECL(spin);
#endif // OS_VARIANT_SELECTOR

void
_os_lock_spin_lock(_os_lock_spin_t l)
{
	return OSSpinLockLock(&l->osl_spinlock);
}

bool
_os_lock_spin_trylock(_os_lock_spin_t l)
{
	return OSSpinLockTry(&l->osl_spinlock);
}

void
_os_lock_spin_unlock(_os_lock_spin_t l)
{
	return OSSpinLockUnlock(&l->osl_spinlock);
}

#pragma mark -
#pragma mark os_lock_owner_t

#ifndef __TSD_MACH_THREAD_SELF
#define __TSD_MACH_THREAD_SELF 3
#endif

typedef mach_port_name_t os_lock_owner_t;

OS_ALWAYS_INLINE
static inline os_lock_owner_t
_os_lock_owner_get_self(void)
{
	os_lock_owner_t self;
	self = (os_lock_owner_t)_os_tsd_get_direct(__TSD_MACH_THREAD_SELF);
	return self;
}

#define OS_LOCK_NO_OWNER MACH_PORT_NULL

#if !OS_LOCK_VARIANT_ONLY

OS_NOINLINE OS_NORETURN OS_COLD
static void
_os_lock_recursive_abort(os_lock_owner_t owner)
{
	__LIBPLATFORM_CLIENT_CRASH__(owner, "Trying to recursively lock an "
			"os_lock");
}

#endif //!OS_LOCK_VARIANT_ONLY

#pragma mark -
#pragma mark os_lock_handoff_t

OS_LOCK_STRUCT_DECL_INTERNAL(handoff,
	os_lock_owner_t volatile osl_owner;
);
#if !OS_VARIANT_ONLY
OS_LOCK_METHODS_DECL(handoff);
OS_LOCK_TYPE_INSTANCE(handoff);
#endif // !OS_VARIANT_ONLY

#ifdef OS_VARIANT_SELECTOR
#define _os_lock_handoff_lock \
		OS_VARIANT(_os_lock_handoff_lock, OS_VARIANT_SELECTOR)
#define _os_lock_handoff_trylock \
		OS_VARIANT(_os_lock_handoff_trylock, OS_VARIANT_SELECTOR)
#define _os_lock_handoff_unlock \
		OS_VARIANT(_os_lock_handoff_unlock, OS_VARIANT_SELECTOR)
OS_LOCK_METHODS_DECL(handoff);
#endif // OS_VARIANT_SELECTOR

#define OS_LOCK_HANDOFF_YIELD_TRIES 100

OS_NOINLINE
static void
_os_lock_handoff_lock_slow(_os_lock_handoff_t l)
{
	int option = SWITCH_OPTION_OSLOCK_DEPRESS;
	mach_msg_timeout_t timeout = 1;
	uint32_t tries = OS_LOCK_HANDOFF_YIELD_TRIES;
	os_lock_owner_t self = _os_lock_owner_get_self(), owner;
	while (unlikely(owner = l->osl_owner)) {
_handoff:
		if (unlikely(owner == self)) return _os_lock_recursive_abort(self);
		// Yield until tries first hits zero, then permanently switch to wait
		if (unlikely(!tries--)) option = SWITCH_OPTION_OSLOCK_WAIT;
		thread_switch(owner, option, timeout);
		// Redrive the handoff every 1ms until switching to wait
		if (option == SWITCH_OPTION_OSLOCK_WAIT) timeout++;
	}
	bool r = os_atomic_cmpxchgv2o(l, osl_owner, MACH_PORT_NULL, self, &owner,
			acquire);
	if (likely(r)) return;
	goto _handoff;
}

void
_os_lock_handoff_lock(_os_lock_handoff_t l)
{
	os_lock_owner_t self = _os_lock_owner_get_self();
	bool r = os_atomic_cmpxchg2o(l, osl_owner, MACH_PORT_NULL, self, acquire);
	if (likely(r)) return;
	return _os_lock_handoff_lock_slow(l);
}

bool
_os_lock_handoff_trylock(_os_lock_handoff_t l)
{
	os_lock_owner_t self = _os_lock_owner_get_self();
	bool r = os_atomic_cmpxchg2o(l, osl_owner, MACH_PORT_NULL, self, acquire);
	return r;
}

void
_os_lock_handoff_unlock(_os_lock_handoff_t l)
{
	os_atomic_store2o(l, osl_owner, MACH_PORT_NULL, release);
}

#pragma mark -
#pragma mark os_ulock_value_t

#include <sys/errno.h>
#include <sys/ulock.h>

typedef os_lock_owner_t os_ulock_value_t;

// This assumes that all thread mach port values always have the low bit set!
// Clearing this bit is used to communicate the existence of waiters to unlock.
#define OS_ULOCK_NOWAITERS_BIT ((os_ulock_value_t)1u)
#define OS_ULOCK_OWNER(value) ((value) | OS_ULOCK_NOWAITERS_BIT)

#define OS_ULOCK_ANONYMOUS_OWNER MACH_PORT_DEAD
#define OS_ULOCK_IS_OWNER(value, self) ({ \
		os_lock_owner_t _owner = OS_ULOCK_OWNER(value); \
		(_owner == (self) && _owner != OS_ULOCK_ANONYMOUS_OWNER); })
#define OS_ULOCK_IS_NOT_OWNER(value, self) ({ \
		os_lock_owner_t _owner = OS_ULOCK_OWNER(value); \
		(_owner != (self) && _owner != OS_ULOCK_ANONYMOUS_OWNER); })


#pragma mark -
#pragma mark os_unfair_lock

typedef struct _os_unfair_lock_s {
	os_ulock_value_t oul_value;
} *_os_unfair_lock_t;

_Static_assert(sizeof(struct os_unfair_lock_s) ==
		sizeof(struct _os_unfair_lock_s), "os_unfair_lock size mismatch");

OS_ATOMIC_EXPORT void os_unfair_lock_lock(os_unfair_lock_t lock);
OS_ATOMIC_EXPORT void os_unfair_lock_lock_with_options(os_unfair_lock_t lock,
		os_unfair_lock_options_t options);
OS_ATOMIC_EXPORT bool os_unfair_lock_trylock(os_unfair_lock_t lock);
OS_ATOMIC_EXPORT void os_unfair_lock_unlock(os_unfair_lock_t lock);

OS_ATOMIC_EXPORT void os_unfair_lock_lock_no_tsd_4libpthread(
		os_unfair_lock_t lock);
OS_ATOMIC_EXPORT void os_unfair_lock_unlock_no_tsd_4libpthread(
		os_unfair_lock_t lock);

_Static_assert(OS_UNFAIR_LOCK_DATA_SYNCHRONIZATION ==
		ULF_WAIT_WORKQ_DATA_CONTENTION,
		"check value for OS_UNFAIR_LOCK_OPTIONS_MASK");
#define OS_UNFAIR_LOCK_OPTIONS_MASK \
		(os_unfair_lock_options_t)(OS_UNFAIR_LOCK_DATA_SYNCHRONIZATION)

OS_NOINLINE OS_NORETURN OS_COLD
static void
_os_unfair_lock_recursive_abort(os_lock_owner_t owner)
{
	__LIBPLATFORM_CLIENT_CRASH__(owner, "Trying to recursively lock an "
			"os_unfair_lock");
}

OS_NOINLINE OS_NORETURN OS_COLD
static void
_os_unfair_lock_unowned_abort(os_lock_owner_t owner)
{
	__LIBPLATFORM_CLIENT_CRASH__(owner, "Unlock of an os_unfair_lock not "
			"owned by current thread");
}

OS_NOINLINE OS_NORETURN OS_COLD
static void
_os_unfair_lock_corruption_abort(os_ulock_value_t current)
{
	__LIBPLATFORM_CLIENT_CRASH__(current, "os_unfair_lock is corrupt");
}

OS_NOINLINE
static void
_os_unfair_lock_lock_slow(_os_unfair_lock_t l, os_lock_owner_t self,
		os_unfair_lock_options_t options)
{
	os_ulock_value_t current, new, waiters_mask = 0;
	if (unlikely(options & ~OS_UNFAIR_LOCK_OPTIONS_MASK)) {
		__LIBPLATFORM_CLIENT_CRASH__(options, "Invalid options");
	}
	while (unlikely((current = os_atomic_load2o(l, oul_value, relaxed)) !=
			OS_LOCK_NO_OWNER)) {
_retry:
		if (unlikely(OS_ULOCK_IS_OWNER(current, self))) {
			return _os_unfair_lock_recursive_abort(self);
		}
		new = current & ~OS_ULOCK_NOWAITERS_BIT;
		if (current != new) {
			// Clear nowaiters bit in lock value before waiting
			if (!os_atomic_cmpxchgv2o(l, oul_value, current, new, &current,
					relaxed)){
				continue;
			}
			current = new;
		}
		int ret = __ulock_wait(UL_UNFAIR_LOCK | ULF_NO_ERRNO | options,
				l, current, 0);
		if (unlikely(ret < 0)) {
			switch (-ret) {
			case EINTR:
			case EFAULT:
				continue;
			case EOWNERDEAD:
				_os_unfair_lock_corruption_abort(current);
				break;
			default:
				__LIBPLATFORM_INTERNAL_CRASH__(-ret, "ulock_wait failure");
			}
		}
		// If there are more waiters, unset nowaiters bit when acquiring lock
		waiters_mask = (ret > 0) ? OS_ULOCK_NOWAITERS_BIT : 0;
	}
	new = self & ~waiters_mask;
	bool r = os_atomic_cmpxchgv2o(l, oul_value, OS_LOCK_NO_OWNER, new,
			&current, acquire);
	if (unlikely(!r)) goto _retry;
}

OS_NOINLINE
static void
_os_unfair_lock_unlock_slow(_os_unfair_lock_t l, os_ulock_value_t current,
		os_lock_owner_t self)
{
	if (unlikely(OS_ULOCK_IS_NOT_OWNER(current, self))) {
		return _os_unfair_lock_unowned_abort(OS_ULOCK_OWNER(current));
	}
	if (current & OS_ULOCK_NOWAITERS_BIT) {
		__LIBPLATFORM_INTERNAL_CRASH__(current, "unlock_slow with no waiters");
	}
	for (;;) {
		int ret = __ulock_wake(UL_UNFAIR_LOCK | ULF_NO_ERRNO, l, 0);
		if (unlikely(ret < 0)) {
			switch (-ret) {
			case EINTR:
				continue;
			case ENOENT:
				break;
			default:
				__LIBPLATFORM_INTERNAL_CRASH__(-ret, "ulock_wake failure");
			}
		}
		break;
	}
}

void
os_unfair_lock_lock(os_unfair_lock_t lock)
{
	_os_unfair_lock_t l = (_os_unfair_lock_t)lock;
	os_lock_owner_t self = _os_lock_owner_get_self();
	bool r = os_atomic_cmpxchg2o(l, oul_value, OS_LOCK_NO_OWNER, self, acquire);
	if (likely(r)) return;
	return _os_unfair_lock_lock_slow(l, self, OS_UNFAIR_LOCK_NONE);
}

void
os_unfair_lock_lock_with_options(os_unfair_lock_t lock,
		os_unfair_lock_options_t options)
{
	_os_unfair_lock_t l = (_os_unfair_lock_t)lock;
	os_lock_owner_t self = _os_lock_owner_get_self();
	bool r = os_atomic_cmpxchg2o(l, oul_value, OS_LOCK_NO_OWNER, self, acquire);
	if (likely(r)) return;
	return _os_unfair_lock_lock_slow(l, self, options);
}

bool
os_unfair_lock_trylock(os_unfair_lock_t lock)
{
	_os_unfair_lock_t l = (_os_unfair_lock_t)lock;
	os_lock_owner_t self = _os_lock_owner_get_self();
	bool r = os_atomic_cmpxchg2o(l, oul_value, OS_LOCK_NO_OWNER, self, acquire);
	return r;
}

void
os_unfair_lock_unlock(os_unfair_lock_t lock)
{
	_os_unfair_lock_t l = (_os_unfair_lock_t)lock;
	os_lock_owner_t self = _os_lock_owner_get_self();
	os_ulock_value_t current;
	current = os_atomic_xchg2o(l, oul_value, OS_LOCK_NO_OWNER, release);
	if (likely(current == self)) return;
	return _os_unfair_lock_unlock_slow(l, current, self);
}

void
os_unfair_lock_lock_no_tsd_4libpthread(os_unfair_lock_t lock)
{
	_os_unfair_lock_t l = (_os_unfair_lock_t)lock;
	os_lock_owner_t self = OS_ULOCK_ANONYMOUS_OWNER;
	bool r = os_atomic_cmpxchg2o(l, oul_value, OS_LOCK_NO_OWNER, self, acquire);
	if (likely(r)) return;
	return _os_unfair_lock_lock_slow(l, self,
			OS_UNFAIR_LOCK_DATA_SYNCHRONIZATION);
}

void
os_unfair_lock_unlock_no_tsd_4libpthread(os_unfair_lock_t lock)
{
	_os_unfair_lock_t l = (_os_unfair_lock_t)lock;
	os_lock_owner_t self = OS_ULOCK_ANONYMOUS_OWNER;
	os_ulock_value_t current;
	current = os_atomic_xchg2o(l, oul_value, OS_LOCK_NO_OWNER, release);
	if (likely(current == self)) return;
	return _os_unfair_lock_unlock_slow(l, current, self);
}

#pragma mark -
#pragma mark _os_lock_unfair_t 4Libc // <rdar://problem/27138264>

OS_ATOMIC_EXPORT void os_unfair_lock_lock_with_options_4Libc(
		os_unfair_lock_t lock, os_unfair_lock_options_t options);
OS_ATOMIC_EXPORT void os_unfair_lock_unlock_4Libc(os_unfair_lock_t lock);

OS_NOINLINE
static void
_os_unfair_lock_lock_slow_4Libc(_os_unfair_lock_t l, os_lock_owner_t self,
		os_unfair_lock_options_t options)
{
	os_ulock_value_t current, new, waiters_mask = 0;
	if (unlikely(options & ~OS_UNFAIR_LOCK_OPTIONS_MASK)) {
		__LIBPLATFORM_CLIENT_CRASH__(options, "Invalid options");
	}
	while (unlikely((current = os_atomic_load2o(l, oul_value, relaxed)) !=
			OS_LOCK_NO_OWNER)) {
_retry:
		if (unlikely(OS_ULOCK_IS_OWNER(current, self))) {
			return _os_unfair_lock_recursive_abort(self);
		}
		new = current & ~OS_ULOCK_NOWAITERS_BIT;
		if (current != new) {
			// Clear nowaiters bit in lock value before waiting
			if (!os_atomic_cmpxchgv2o(l, oul_value, current, new, &current,
					relaxed)){
				continue;
			}
			current = new;
		}
		int ret = __ulock_wait(UL_UNFAIR_LOCK | ULF_NO_ERRNO | options,
				l, current, 0);
		if (unlikely(ret < 0)) {
			switch (-ret) {
			case EINTR:
			case EFAULT:
				continue;
			case EOWNERDEAD:
				// if we get an `EOWNERDEAD` it could be corruption of the lock
				// so for the Libc locks, if we can steal the lock, assume
				// it is corruption and pretend we got the lock with contention
				new = self & ~OS_ULOCK_NOWAITERS_BIT;
				if (os_atomic_cmpxchgv2o(l, oul_value, current, new, &current,
								acquire)) {
					return;
				}
				break;
			default:
				__LIBPLATFORM_INTERNAL_CRASH__(-ret, "ulock_wait failure");
			}
		}
		// If there are more waiters, unset nowaiters bit when acquiring lock
		waiters_mask = (ret > 0) ? OS_ULOCK_NOWAITERS_BIT : 0;
	}
	new = self & ~waiters_mask;
	bool r = os_atomic_cmpxchgv2o(l, oul_value, OS_LOCK_NO_OWNER, new,
			&current, acquire);
	if (unlikely(!r)) goto _retry;
}

OS_NOINLINE
static void
_os_unfair_lock_unlock_slow_4Libc(_os_unfair_lock_t l)
{
	for (;;) {
		int ret = __ulock_wake(UL_UNFAIR_LOCK | ULF_NO_ERRNO, l, 0);
		if (unlikely(ret < 0)) {
			switch (-ret) {
			case EINTR:
				continue;
			case ENOENT:
				break;
			default:
				__LIBPLATFORM_INTERNAL_CRASH__(-ret, "ulock_wake failure");
			}
		}
		break;
	}
}

void
os_unfair_lock_lock_with_options_4Libc(os_unfair_lock_t lock,
		os_unfair_lock_options_t options)
{
	_os_unfair_lock_t l = (_os_unfair_lock_t)lock;
	os_lock_owner_t self = _os_lock_owner_get_self();
	bool r = os_atomic_cmpxchg2o(l, oul_value, OS_LOCK_NO_OWNER, self, acquire);
	if (likely(r)) return;
	return _os_unfair_lock_lock_slow_4Libc(l, self, options);
}

void
os_unfair_lock_unlock_4Libc(os_unfair_lock_t lock)
{
	_os_unfair_lock_t l = (_os_unfair_lock_t)lock;
	os_lock_owner_t self = _os_lock_owner_get_self();
	os_ulock_value_t current;
	current = os_atomic_xchg2o(l, oul_value, OS_LOCK_NO_OWNER, release);
	if (likely(current == self)) return;
	return _os_unfair_lock_unlock_slow_4Libc(l);
}

#if !OS_VARIANT_ONLY
void
os_unfair_lock_assert_owner(os_unfair_lock_t lock)
{
	_os_unfair_lock_t l = (_os_unfair_lock_t)lock;
	os_lock_owner_t self = _os_lock_owner_get_self();
	os_ulock_value_t current = os_atomic_load2o(l, oul_value, relaxed);
	if (unlikely(OS_ULOCK_IS_NOT_OWNER(current, self))) {
		__LIBPLATFORM_CLIENT_CRASH__(current, "Assertion failed: "
				"Lock unexpectedly not owned by current thread");
	}
}

void
os_unfair_lock_assert_not_owner(os_unfair_lock_t lock)
{
	_os_unfair_lock_t l = (_os_unfair_lock_t)lock;
	os_lock_owner_t self = _os_lock_owner_get_self();
	os_ulock_value_t current = os_atomic_load2o(l, oul_value, relaxed);
	if (unlikely(OS_ULOCK_IS_OWNER(current, self))) {
		__LIBPLATFORM_CLIENT_CRASH__(current, "Assertion failed: "
				"Lock unexpectedly owned by current thread");
	}
}
#endif

#pragma mark -
#pragma mark _os_lock_unfair_t

OS_LOCK_STRUCT_DECL_INTERNAL(unfair,
	os_unfair_lock osl_unfair_lock;
);
#if !OS_VARIANT_ONLY
OS_LOCK_METHODS_DECL(unfair);
OS_LOCK_TYPE_INSTANCE(unfair);
#endif // !OS_VARIANT_ONLY

#ifdef OS_VARIANT_SELECTOR
#define _os_lock_unfair_lock \
		OS_VARIANT(_os_lock_unfair_lock, OS_VARIANT_SELECTOR)
#define _os_lock_unfair_trylock \
		OS_VARIANT(_os_lock_unfair_trylock, OS_VARIANT_SELECTOR)
#define _os_lock_unfair_unlock \
		OS_VARIANT(_os_lock_unfair_unlock, OS_VARIANT_SELECTOR)
OS_LOCK_METHODS_DECL(unfair);
#endif // OS_VARIANT_SELECTOR

void
_os_lock_unfair_lock(_os_lock_unfair_t l)
{
	return os_unfair_lock_lock(&l->osl_unfair_lock);
}

bool
_os_lock_unfair_trylock(_os_lock_unfair_t l)
{
	return os_unfair_lock_trylock(&l->osl_unfair_lock);
}

void
_os_lock_unfair_unlock(_os_lock_unfair_t l)
{
	return os_unfair_lock_unlock(&l->osl_unfair_lock);
}

#pragma mark -
#pragma mark _os_nospin_lock

typedef struct _os_nospin_lock_s {
	os_ulock_value_t oul_value;
} _os_nospin_lock, *_os_nospin_lock_t;

_Static_assert(sizeof(OSSpinLock) ==
		sizeof(struct _os_nospin_lock_s), "os_nospin_lock size mismatch");

OS_ATOMIC_EXPORT void _os_nospin_lock_lock(_os_nospin_lock_t lock);
OS_ATOMIC_EXPORT bool _os_nospin_lock_trylock(_os_nospin_lock_t lock);
OS_ATOMIC_EXPORT void _os_nospin_lock_unlock(_os_nospin_lock_t lock);

OS_NOINLINE
static void
_os_nospin_lock_lock_slow(_os_nospin_lock_t l)
{
	os_lock_owner_t self = _os_lock_owner_get_self();
	os_ulock_value_t current, new, waiters_mask = 0;
	uint32_t timeout = 1;
	while (unlikely((current = os_atomic_load2o(l, oul_value, relaxed)) !=
			OS_LOCK_NO_OWNER)) {
_retry:
		new = current & ~OS_ULOCK_NOWAITERS_BIT;
		// For safer compatibility with OSSpinLock where _OSSpinLockLocked may
		// be 1, check that new didn't become 0 (unlocked) by clearing this bit
		if (current != new && new) {
			// Clear nowaiters bit in lock value before waiting
			if (!os_atomic_cmpxchgv2o(l, oul_value, current, new, &current,
					relaxed)){
				continue;
			}
			current = new;
		}
		int ret = __ulock_wait(UL_COMPARE_AND_WAIT | ULF_NO_ERRNO, l, current,
				timeout * 1000);
		if (unlikely(ret < 0)) {
			switch (-ret) {
			case ETIMEDOUT:
				timeout++;
				continue;
			case EINTR:
			case EFAULT:
				continue;
			default:
				__LIBPLATFORM_INTERNAL_CRASH__(-ret, "ulock_wait failure");
			}
		}
		// If there are more waiters, unset nowaiters bit when acquiring lock
		waiters_mask = (ret > 0) ? OS_ULOCK_NOWAITERS_BIT : 0;
	}
	new = self & ~waiters_mask;
	bool r = os_atomic_cmpxchgv2o(l, oul_value, OS_LOCK_NO_OWNER, new,
			&current, acquire);
	if (unlikely(!r)) goto _retry;
}

OS_NOINLINE
static void
_os_nospin_lock_unlock_slow(_os_nospin_lock_t l, os_ulock_value_t current)
{
	os_lock_owner_t self = _os_lock_owner_get_self();
	if (unlikely(OS_ULOCK_OWNER(current) != self)) {
		return; // no unowned_abort for drop-in compatibility with OSSpinLock
	}
	if (current & OS_ULOCK_NOWAITERS_BIT) {
		__LIBPLATFORM_INTERNAL_CRASH__(current, "unlock_slow with no waiters");
	}
	for (;;) {
		int ret = __ulock_wake(UL_COMPARE_AND_WAIT | ULF_NO_ERRNO, l, 0);
		if (unlikely(ret < 0)) {
			switch (-ret) {
			case EINTR:
				continue;
			case ENOENT:
				break;
			default:
				__LIBPLATFORM_INTERNAL_CRASH__(-ret, "ulock_wake failure");
			}
		}
		break;
	}
}

void
_os_nospin_lock_lock(_os_nospin_lock_t l)
{
	os_lock_owner_t self = _os_lock_owner_get_self();
	bool r = os_atomic_cmpxchg2o(l, oul_value, OS_LOCK_NO_OWNER, self, acquire);
	if (likely(r)) return;
	return _os_nospin_lock_lock_slow(l);
}

bool
_os_nospin_lock_trylock(_os_nospin_lock_t l)
{
	os_lock_owner_t self = _os_lock_owner_get_self();
	bool r = os_atomic_cmpxchg2o(l, oul_value, OS_LOCK_NO_OWNER, self, acquire);
	return r;
}

void
_os_nospin_lock_unlock(_os_nospin_lock_t l)
{
	os_lock_owner_t self = _os_lock_owner_get_self();
	os_ulock_value_t current;
	current = os_atomic_xchg2o(l, oul_value, OS_LOCK_NO_OWNER, release);
	if (likely(current == self)) return;
	return _os_nospin_lock_unlock_slow(l, current);
}

#pragma mark -
#pragma mark _os_lock_nospin_t

OS_LOCK_STRUCT_DECL_INTERNAL(nospin,
	_os_nospin_lock osl_nospin_lock;
);
#if !OS_VARIANT_ONLY
OS_LOCK_METHODS_DECL(nospin);
OS_LOCK_TYPE_INSTANCE(nospin);
#endif // !OS_VARIANT_ONLY

#ifdef OS_VARIANT_SELECTOR
#define _os_lock_nospin_lock \
		OS_VARIANT(_os_lock_nospin_lock, OS_VARIANT_SELECTOR)
#define _os_lock_nospin_trylock \
		OS_VARIANT(_os_lock_nospin_trylock, OS_VARIANT_SELECTOR)
#define _os_lock_nospin_unlock \
		OS_VARIANT(_os_lock_nospin_unlock, OS_VARIANT_SELECTOR)
OS_LOCK_METHODS_DECL(nospin);
#endif // OS_VARIANT_SELECTOR

void
_os_lock_nospin_lock(_os_lock_nospin_t l)
{
	return _os_nospin_lock_lock(&l->osl_nospin_lock);
}

bool
_os_lock_nospin_trylock(_os_lock_nospin_t l)
{
	return _os_nospin_lock_trylock(&l->osl_nospin_lock);
}

void
_os_lock_nospin_unlock(_os_lock_nospin_t l)
{
	return _os_nospin_lock_unlock(&l->osl_nospin_lock);
}

#pragma mark -
#pragma mark os_once_t

typedef struct os_once_gate_s {
	union {
		os_ulock_value_t ogo_lock;
		os_once_t ogo_once;
	};
} os_once_gate_s, *os_once_gate_t;

#define OS_ONCE_INIT ((os_once_t)0l)
#define OS_ONCE_DONE (~(os_once_t)0l)

OS_ATOMIC_EXPORT void _os_once(os_once_t *val, void *ctxt, os_function_t func);
OS_ATOMIC_EXPORT void __os_once_reset(os_once_t *val);

OS_NOINLINE OS_NORETURN OS_COLD
static void
_os_once_gate_recursive_abort(os_lock_owner_t owner)
{
	__LIBPLATFORM_CLIENT_CRASH__(owner, "Trying to recursively lock an "
			"os_once_t");
}

OS_NOINLINE OS_NORETURN OS_COLD
static void
_os_once_gate_unowned_abort(os_lock_owner_t owner)
{
	__LIBPLATFORM_CLIENT_CRASH__(owner, "Unlock of an os_once_t not "
			"owned by current thread");
}

OS_NOINLINE OS_NORETURN OS_COLD
static void
_os_once_gate_corruption_abort(os_ulock_value_t current)
{
	__LIBPLATFORM_CLIENT_CRASH__(current, "os_once_t is corrupt");
}

OS_NOINLINE
static void
_os_once_gate_wait_slow(os_ulock_value_t *gate, os_lock_owner_t self)
{
	os_ulock_value_t tid_old, tid_new;

	for (;;) {
		os_atomic_rmw_loop(gate, tid_old, tid_new, relaxed, {
			switch (tid_old) {
			case (os_ulock_value_t)OS_ONCE_INIT: // raced with __os_once_reset()
			case (os_ulock_value_t)OS_ONCE_DONE: // raced with _os_once()
				os_atomic_rmw_loop_give_up(return);
			}
			tid_new = tid_old & ~OS_ULOCK_NOWAITERS_BIT;
			if (tid_new == tid_old) os_atomic_rmw_loop_give_up(break);
		});
		if (unlikely(OS_ULOCK_IS_OWNER(tid_old, self))) {
			return _os_once_gate_recursive_abort(self);
		}
		int ret = __ulock_wait(UL_UNFAIR_LOCK | ULF_NO_ERRNO,
				gate, tid_new, 0);
		if (unlikely(ret < 0)) {
			switch (-ret) {
			case EINTR:
			case EFAULT:
				continue;
			case EOWNERDEAD:
				_os_once_gate_corruption_abort(tid_old);
				break;
			default:
				__LIBPLATFORM_INTERNAL_CRASH__(-ret, "ulock_wait failure");
			}
		}
	}
}

OS_NOINLINE
static void
_os_once_gate_broadcast_slow(os_ulock_value_t *gate, os_ulock_value_t current,
		os_lock_owner_t self)
{
	if (unlikely(OS_ULOCK_IS_NOT_OWNER(current, self))) {
		return _os_once_gate_unowned_abort(OS_ULOCK_OWNER(current));
	}
	if (current & OS_ULOCK_NOWAITERS_BIT) {
		__LIBPLATFORM_INTERNAL_CRASH__(current, "unlock_slow with no waiters");
	}
	for (;;) {
		int ret = __ulock_wake(UL_UNFAIR_LOCK | ULF_NO_ERRNO | ULF_WAKE_ALL,
				gate, 0);
		if (unlikely(ret < 0)) {
			switch (-ret) {
			case EINTR:
				continue;
			case ENOENT:
				break;
			default:
				__LIBPLATFORM_INTERNAL_CRASH__(-ret, "ulock_wake failure");
			}
		}
		break;
	}
}

OS_ALWAYS_INLINE
static void
_os_once_gate_set_value_and_broadcast(os_once_gate_t og, os_lock_owner_t self,
		os_once_t value)
{
	// The next barrier must be long and strong.
	//
	// The scenario: SMP systems with weakly ordered memory models
	// and aggressive out-of-order instruction execution.
	//
	// The problem:
	//
	// The os_once*() wrapper macro causes the callee's
	// instruction stream to look like this (pseudo-RISC):
	//
	//      load r5, pred-addr
	//      cmpi r5, -1
	//      beq  1f
	//      call os_once*()
	//      1f:
	//      load r6, data-addr
	//
	// May be re-ordered like so:
	//
	//      load r6, data-addr
	//      load r5, pred-addr
	//      cmpi r5, -1
	//      beq  1f
	//      call os_once*()
	//      1f:
	//
	// Normally, a barrier on the read side is used to workaround
	// the weakly ordered memory model. But barriers are expensive
	// and we only need to synchronize once! After func(ctxt)
	// completes, the predicate will be marked as "done" and the
	// branch predictor will correctly skip the call to
	// os_once*().
	//
	// A far faster alternative solution: Defeat the speculative
	// read-ahead of peer CPUs.
	//
	// Modern architectures will throw away speculative results
	// once a branch mis-prediction occurs. Therefore, if we can
	// ensure that the predicate is not marked as being complete
	// until long after the last store by func(ctxt), then we have
	// defeated the read-ahead of peer CPUs.
	//
	// In other words, the last "store" by func(ctxt) must complete
	// and then N cycles must elapse before ~0l is stored to *val.
	// The value of N is whatever is sufficient to defeat the
	// read-ahead mechanism of peer CPUs.
	//
	// On some CPUs, the most fully synchronizing instruction might
	// need to be issued.
	os_atomic_maximally_synchronizing_barrier();

	// above assumed to contain release barrier
	os_ulock_value_t current =
			(os_ulock_value_t)os_atomic_xchg(&og->ogo_once, value, relaxed);
	if (likely(current == self)) return;
	_os_once_gate_broadcast_slow(&og->ogo_lock, current, self);
}

// Atomically resets the once value to zero and then signals all
// pending waiters to return from their _os_once_gate_wait_slow()
void
__os_once_reset(os_once_t *val)
{
	os_once_gate_t og = (os_once_gate_t)val;
	os_lock_owner_t self = _os_lock_owner_get_self();
	_os_once_gate_set_value_and_broadcast(og, self, OS_ONCE_INIT);
}

void
_os_once(os_once_t *val, void *ctxt, os_function_t func)
{
	os_once_gate_t og = (os_once_gate_t)val;
	os_lock_owner_t self = _os_lock_owner_get_self();
	os_once_t v = (os_once_t)self;

	if (likely(os_atomic_cmpxchg(&og->ogo_once, OS_ONCE_INIT, v, relaxed))) {
		func(ctxt);
		_os_once_gate_set_value_and_broadcast(og, self, OS_ONCE_DONE);
	} else {
		_os_once_gate_wait_slow(&og->ogo_lock, self);
	}
}

#if !OS_VARIANT_ONLY

#pragma mark -
#pragma mark os_lock_eliding_t

#if !TARGET_OS_IPHONE

#define _os_lock_eliding_t _os_lock_spin_t
#define _os_lock_eliding_lock _os_lock_spin_lock
#define _os_lock_eliding_trylock _os_lock_spin_trylock
#define _os_lock_eliding_unlock _os_lock_spin_unlock
OS_LOCK_METHODS_DECL(eliding);
OS_LOCK_TYPE_INSTANCE(eliding);

#pragma mark -
#pragma mark os_lock_transactional_t

OS_LOCK_STRUCT_DECL_INTERNAL(transactional,
	uintptr_t volatile osl_lock;
);

#define _os_lock_transactional_t _os_lock_eliding_t
#define _os_lock_transactional_lock _os_lock_eliding_lock
#define _os_lock_transactional_trylock _os_lock_eliding_trylock
#define _os_lock_transactional_unlock _os_lock_eliding_unlock
OS_LOCK_METHODS_DECL(transactional);
OS_LOCK_TYPE_INSTANCE(transactional);

#endif // !TARGET_OS_IPHONE
#endif // !OS_VARIANT_ONLY
#endif // !OS_LOCK_VARIANT_ONLY
