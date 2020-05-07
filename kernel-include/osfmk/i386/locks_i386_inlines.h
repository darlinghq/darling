/*
 * Copyright (c) 201 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#ifndef _I386_LOCKS_I386_INLINES_H_
#define _I386_LOCKS_I386_INLINES_H_

#include <kern/locks.h>
#include <kern/lock_stat.h>
#include <kern/turnstile.h>

// Enforce program order of loads and stores.
#define ordered_load(target) os_atomic_load(target, compiler_acq_rel)
#define ordered_store_release(target, value) ({ \
	        os_atomic_store(target, value, release); \
	        os_compiler_barrier(); \
})

/* Enforce program order of loads and stores. */
#define ordered_load_mtx_state(lock)                    ordered_load(&(lock)->lck_mtx_state)
#define ordered_store_mtx_state_release(lock, value)            ordered_store_release(&(lock)->lck_mtx_state, (value))
#define ordered_store_mtx_owner(lock, value)    os_atomic_store(&(lock)->lck_mtx_owner, (value), compiler_acq_rel)

#if DEVELOPMENT | DEBUG
void lck_mtx_owner_check_panic(lck_mtx_t       *mutex) __abortlike;
#endif

__attribute__((always_inline))
static inline void
lck_mtx_ilk_unlock_inline(
	lck_mtx_t       *mutex,
	uint32_t        state)
{
	state &= ~LCK_MTX_ILOCKED_MSK;
	ordered_store_mtx_state_release(mutex, state);

	enable_preemption();
}

__attribute__((always_inline))
static inline void
lck_mtx_lock_finish_inline(
	lck_mtx_t       *mutex,
	uint32_t        state,
	boolean_t       indirect)
{
	assert(state & LCK_MTX_ILOCKED_MSK);

	/* release the interlock and re-enable preemption */
	lck_mtx_ilk_unlock_inline(mutex, state);

#if     CONFIG_DTRACE
	if (indirect) {
		LOCKSTAT_RECORD(LS_LCK_MTX_EXT_LOCK_ACQUIRE, mutex, 0);
	} else {
		LOCKSTAT_RECORD(LS_LCK_MTX_LOCK_ACQUIRE, mutex, 0);
	}
#endif
}

__attribute__((always_inline))
static inline void
lck_mtx_lock_finish_inline_with_cleanup(
	lck_mtx_t       *mutex,
	uint32_t        state,
	boolean_t       indirect)
{
	assert(state & LCK_MTX_ILOCKED_MSK);

	/* release the interlock and re-enable preemption */
	lck_mtx_ilk_unlock_inline(mutex, state);

	turnstile_cleanup();

#if     CONFIG_DTRACE
	if (indirect) {
		LOCKSTAT_RECORD(LS_LCK_MTX_EXT_LOCK_ACQUIRE, mutex, 0);
	} else {
		LOCKSTAT_RECORD(LS_LCK_MTX_LOCK_ACQUIRE, mutex, 0);
	}
#endif
}

__attribute__((always_inline))
static inline void
lck_mtx_try_lock_finish_inline(
	lck_mtx_t       *mutex,
	uint32_t        state)
{
	/* release the interlock and re-enable preemption */
	lck_mtx_ilk_unlock_inline(mutex, state);

#if     CONFIG_DTRACE
	LOCKSTAT_RECORD(LS_LCK_MTX_TRY_LOCK_ACQUIRE, mutex, 0);
#endif
}

__attribute__((always_inline))
static inline void
lck_mtx_convert_spin_finish_inline(
	lck_mtx_t       *mutex,
	uint32_t        state)
{
	/* release the interlock and acquire it as mutex */
	state &= ~(LCK_MTX_ILOCKED_MSK | LCK_MTX_SPIN_MSK);
	state |= LCK_MTX_MLOCKED_MSK;

	ordered_store_mtx_state_release(mutex, state);
	enable_preemption();
}

__attribute__((always_inline))
static inline void
lck_mtx_unlock_finish_inline(
	lck_mtx_t       *mutex,
	boolean_t       indirect)
{
	enable_preemption();

#if     CONFIG_DTRACE
	if (indirect) {
		LOCKSTAT_RECORD(LS_LCK_MTX_EXT_UNLOCK_RELEASE, mutex, 0);
	} else {
		LOCKSTAT_RECORD(LS_LCK_MTX_UNLOCK_RELEASE, mutex, 0);
	}
#endif  // CONFIG_DTRACE
}

#endif /* _I386_LOCKS_I386_INLINES_H_ */
