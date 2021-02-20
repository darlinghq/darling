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

#if OS_UNFAIR_LOCK_INLINE
#define os_unfair_lock_lock_with_options(lock, options) \
		os_unfair_lock_lock_with_options_inline(lock, options)
#define os_unfair_lock_trylock(lock) \
		os_unfair_lock_trylock_inline(lock)
#define os_unfair_lock_unlock(lock) \
		os_unfair_lock_unlock_inline(lock)
#endif // OS_UNFAIR_LOCK_INLINE

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
	return os_unfair_lock_lock_with_options(lock, OS_UNFAIR_LOCK_ADAPTIVE_SPIN |
			OS_UNFAIR_LOCK_DATA_SYNCHRONIZATION);
}

MALLOC_ALWAYS_INLINE
static inline bool
_malloc_lock_trylock(_malloc_lock_s *lock) {
    return os_unfair_lock_trylock(lock);
}

MALLOC_ALWAYS_INLINE
static inline void
_malloc_lock_unlock(_malloc_lock_s *lock) {
    return os_unfair_lock_unlock(lock);
}

MALLOC_ALWAYS_INLINE
static inline void
_malloc_lock_assert_owner(_malloc_lock_s *lock) {
	os_unfair_lock_assert_owner(lock);
}

#endif // __LOCKING_H
