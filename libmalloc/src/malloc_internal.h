/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef __MALLOC_INTERNAL_H
#define __MALLOC_INTERNAL_H

#include <TargetConditionals.h>
#include <os/lock_private.h>

#if TARGET_OS_EMBEDDED && !defined(MALLOC_USE_OS_LOCK_HANDOFF)
#define MALLOC_USE_OS_LOCK_HANDOFF 1 // <rdar://problem/13807682>
#endif

#if !TARGET_IPHONE_SIMULATOR || MALLOC_USE_OS_LOCK_HANDOFF
#define MALLOC_USE_OS_LOCK 1 // <rdar://problem/16104834> <rdar://problem/16230083>
#endif

#if MALLOC_USE_OS_LOCK_HANDOFF
typedef os_lock_handoff_s _malloc_lock_s;
#define _MALLOC_LOCK_INIT OS_LOCK_HANDOFF_INIT
#elif MALLOC_USE_OS_LOCK
typedef os_lock_spin_s _malloc_lock_s;
#define _MALLOC_LOCK_INIT OS_LOCK_SPIN_INIT
#else
#include <libkern/OSAtomic.h>
typedef OSSpinLock _malloc_lock_s;
#define _MALLOC_LOCK_INIT OS_SPINLOCK_INIT
#endif

__attribute__((always_inline))
static inline void
_malloc_lock_init(_malloc_lock_s *lock) {
	const _malloc_lock_s _malloc_lock_init = _MALLOC_LOCK_INIT;
	*lock = _malloc_lock_init;
}

#if MALLOC_USE_OS_LOCK
__attribute__((always_inline))
static inline void
_malloc_lock_lock(_malloc_lock_s *lock) {
	return os_lock_lock(lock);
}

__attribute__((always_inline))
static inline bool
_malloc_lock_trylock(_malloc_lock_s *lock) {
	return os_lock_trylock(lock);
}

__attribute__((always_inline))
static inline void
_malloc_lock_unlock(_malloc_lock_s *lock) {
	return os_lock_unlock(lock);
}

#else // MALLOC_USE_OS_LOCK

__attribute__((always_inline))
static inline void
_malloc_lock_lock(_malloc_lock_s *lock) {
	return OSSpinLockLock(lock);
}

__attribute__((always_inline))
static inline bool
_malloc_lock_trylock(_malloc_lock_s *lock) {
	return OSSpinLockTry(lock);
}

__attribute__((always_inline))
static inline void
_malloc_lock_unlock(_malloc_lock_s *lock) {
	return OSSpinLockUnlock(lock);
}

#endif // MALLOC_USE_OS_LOCK


#endif // __MALLOC_INTERNAL_H
