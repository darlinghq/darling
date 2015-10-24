/*
 * Copyright (c) 2011 Apple Inc. All rights reserved.
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

// OSAtomic.h is included by files that include this C file.
/* #include "OSAtomic.h" */

typedef int32_t OSSpinLock;

/*
 * Bear with me, there's method to this madness. clang actually produces more optimal
 * fastpath code if the lock code is split in half like this.
 *
 * Once we're spinning in userspace/kernel like this then we can be more lenient about
 * how much effort is spent doing the spin. Where the lock is uncontended, this split
 * arrangement produces a very good fastpath at the cost of some code duplication.
 *
 * Similarly, the gotos produce less wasteful code during spins then the equivalent code
 * defined with loops.
 */

#import <mach/mach_traps.h>

static inline void _OSSpinLockSlow(volatile OSSpinLock * lock) _OSATOMIC_VARIANT(_OSSpinLockSlow);
static inline void _OSSpinLockSlow(volatile OSSpinLock * lock)
{
	int32_t r;
	uint32_t t;

_spin: ;
#if (defined(_ARM_ARCH_7) && !defined(_OSATOMIC_NO_BARRIERS))
	uint32_t tries = MP_SPIN_TRIES;
	do {
		if (*lock == 0) goto _try_store;
		_osatomic_pause();
	} while (--tries);
#endif

	__asm__ ("mov	r0, %[_a] ;"
			 "mov	r1, %[_b] ;"
			 "mov	r2, %[_c] ;"
			 "bl	_syscall_thread_switch ;"
			 : : [_a] "i" (0), [_b] "i" (1), [_c] "i" (1)
			 : "r0", "r1", "r2", "r9", "r12", "lr" );

_try_store:
	do {
		_osatomic_load_exclusive(lock, r);
		if (slowpath(r)) goto _spin;
		_osatomic_store_exclusive(lock, 1, t);
	} while (slowpath(t));

	_osatomic_barrier();
}

void OSSpinLockLock(volatile OSSpinLock * lock) _OSATOMIC_VARIANT(OSSpinLockLock);
void OSSpinLockLock(volatile OSSpinLock * lock)
{
	_OSATOMIC_ALIAS(spin_lock, OSSpinLockLock);
	_OSATOMIC_ALIAS(_spin_lock, OSSpinLockLock);

	int32_t r;
	uint32_t t;

	do {
		_osatomic_load_exclusive(lock, r);
		if (slowpath(r)) return _OSSpinLockSlow(lock);
		_osatomic_store_exclusive(lock, 1, t);
	} while (slowpath(t));

	_osatomic_barrier();
}

#ifndef _OSATOMIC_WFE

bool OSSpinLockTry(OSSpinLock * lock) _OSATOMIC_VARIANT(OSSpinLockTry);
bool OSSpinLockTry(OSSpinLock * lock)
{
	_OSATOMIC_ALIAS(spin_lock_try, OSSpinLockTry);
	_OSATOMIC_ALIAS(_spin_lock_try, OSSpinLockTry);

	int32_t r;
	uint32_t t;

	do {
		_osatomic_load_exclusive(lock, r);
		if (slowpath(r)) return false;
		_osatomic_store_exclusive(lock, 1, t);
	} while (slowpath(t));

	_osatomic_barrier();
	return (r == 0);
}

void OSSpinLockUnlock(OSSpinLock * lock) _OSATOMIC_VARIANT(OSSpinLockUnlock);
void OSSpinLockUnlock(OSSpinLock * lock)
{
	_OSATOMIC_ALIAS(spin_unlock, OSSpinLockUnlock);
	_OSATOMIC_ALIAS(_spin_unlock, OSSpinLockUnlock);

	_osatomic_barrier();
	*lock = 0;
}

#endif // _OSATOMIC_WFE
