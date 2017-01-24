/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

#ifndef __LOCKING_H
#define __LOCKING_H

#if CONFIG_OS_LOCK_UNFAIR

typedef os_unfair_lock _malloc_lock_s;
#define _MALLOC_LOCK_INIT OS_UNFAIR_LOCK_INIT

__attribute__((always_inline))
static inline void
_malloc_lock_init(_malloc_lock_s *lock) {
    *lock = OS_UNFAIR_LOCK_INIT;
}

MALLOC_ALWAYS_INLINE
static inline void
_malloc_lock_lock(_malloc_lock_s *lock) {
	os_unfair_lock_lock_with_options_inline(lock,
			OS_UNFAIR_LOCK_DATA_SYNCHRONIZATION);
}

MALLOC_ALWAYS_INLINE
static inline bool
_malloc_lock_trylock(_malloc_lock_s *lock) {
    return os_unfair_lock_trylock_inline(lock);
}

MALLOC_ALWAYS_INLINE
static inline void
_malloc_lock_unlock(_malloc_lock_s *lock) {
    os_unfair_lock_unlock_inline(lock);
}

#else // CONFIG_OS_LOCK_UNFAIR
#if CONFIG_OS_LOCK_HANDOFF

typedef os_lock_handoff_s _malloc_lock_s;
#define _MALLOC_LOCK_INIT OS_LOCK_HANDOFF_INIT

__attribute__((always_inline))
static inline void
_malloc_lock_init(_malloc_lock_s *lock) {
    const os_lock_handoff_s _os_lock_handoff_init = OS_LOCK_HANDOFF_INIT;
    *lock = _os_lock_handoff_init;
}

#else //CONFIG_OS_LOCK_HANDOFF

typedef os_lock_spin_s _malloc_lock_s;
#define _MALLOC_LOCK_INIT OS_LOCK_SPIN_INIT

MALLOC_ALWAYS_INLINE
static inline void
_malloc_lock_init(_malloc_lock_s *lock) {
    const os_lock_spin_s _os_lock_spin_init = OS_LOCK_SPIN_INIT;
    *lock = _os_lock_spin_init;
}

#endif

MALLOC_ALWAYS_INLINE
static inline void
_malloc_lock_lock(_malloc_lock_s *lock) {
    return os_lock_lock(lock);
}

MALLOC_ALWAYS_INLINE
static inline bool
_malloc_lock_trylock(_malloc_lock_s *lock) {
    return os_lock_trylock(lock);
}

MALLOC_ALWAYS_INLINE
static inline void
_malloc_lock_unlock(_malloc_lock_s *lock) {
    return os_lock_unlock(lock);
}

#endif // !CONFIG_OS_LOCK_UNFAIR

#endif // __LOCKING_H
