/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
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
/*
 * @OSF_COPYRIGHT@
 * 
 */
/*
 * HISTORY
 * 
 * Revision 1.1.1.1  1998/09/22 21:05:30  wsanchez
 * Import of Mac OS X kernel (~semeria)
 *
 * Revision 1.1.1.1  1998/03/07 02:25:45  wsanchez
 * Import of OSF Mach kernel (~mburg)
 *
 * Revision 1.1.4.1  1995/06/13  18:20:29  sjs
 * 	Merged from flipc_shared.
 * 	[95/06/07            sjs]
 *
 * Revision 1.1.2.3  1995/03/09  19:42:30  rwd
 * 	Move yield function out of macro and prototype.
 * 	[1995/03/09  19:36:25  rwd]
 * 
 * Revision 1.1.2.2  1995/02/21  17:23:11  randys
 * 	Re-indented code to four space indentation
 * 	[1995/02/21  16:25:39  randys]
 * 
 * Revision 1.1.2.1  1994/12/20  19:02:06  randys
 * 	Moved definition of flipc_simple_lock to flipc_cb.h
 * 	[1994/12/20  17:34:44  randys]
 * 
 * 	Separated the lock macros out into machine dependent and independent files;
 * 	this is the machine independent file.
 * 	[1994/12/20  16:43:38  randys]
 * 
 * $EndLog$
 */

/*
 * mach/flipc_locks.h
 *
 * The machine independent part of the flipc_simple_locks definitions.
 * Most of the locks definitions is in flipc_dep.h, but what isn't
 * dependent on the platform being used is here.
 */

/*
 * Note that the locks defined in this file and in flipc_dep.h are only
 * for use by user level code.  The reason why this file is visible to
 * the kernel is that the kernel section of flipc needs to initialize
 * these locks.
 */

#ifndef _MACH_FLIPC_LOCKS_H_
#define _MACH_FLIPC_LOCKS_H_

/* Get the simple lock type.  */
#include <mach/flipc_cb.h>

/*
 * Lock function prototypes.  This needs to be before any lock definitions
 * that happen to be macros.
 */

/* Initializes lock.  Always a macro (so that kernel code can use it without
   library assistance).  */
void flipc_simple_lock_init(flipc_simple_lock *lock);

/* Returns 1 if lock gained, 0 otherwise.  */
int flipc_simple_lock_try(flipc_simple_lock *lock);

/* Returns 1 if lock is locked, 0 otherwise.  */
int flipc_simple_lock_locked(flipc_simple_lock *lock);

/* Releases the lock.  */
void flipc_simple_lock_release(flipc_simple_lock *lock);

/* Take the lock.  */
void flipc_simple_lock_acquire(flipc_simple_lock *lock);

/* Take two locks.  Does not hold one while spinning on the
   other.  */
void flipc_simple_lock_acquire_2(flipc_simple_lock *lock1,
			   flipc_simple_lock *lock2);

/* Get the machine dependent stuff.  The things that need to be
 * defined in a machine dependent fashion are:
 *
 *   flipc_simple_lock_init	(must be a macro)
 *   flipc_simple_lock_try
 *   flipc_simple_lock_locked
 *   flipc_simple_lock_release
 *
 * These last three don't necessarily have to be macros, but if they
 * aren't definitions must be included in the machine dependent
 * part of the user level library code.
 */
#include <mach/machine/flipc_dep.h>

/*
 * Set at flipc initialization time to thread_yield argument to
 * FLIPC_domain_init
 */

extern void (*flipc_simple_lock_yield_fn)(void);

/*
 * Machine independent definitions; defined in terms of above routines.
 */

/* Take the lock.  Assumes an external define saying how long to
   spin, and an external function to call when we've spun too long.  */
#define flipc_simple_lock_acquire(lock)		\
do {						\
  int __spin_count = 0;				\
						\
  while (flipc_simple_lock_locked(lock)		\
	 || !flipc_simple_lock_try(lock))	\
    if (++__spin_count > LOCK_SPIN_LIMIT) {	\
      (*flipc_simple_lock_yield_fn)();		\
      __spin_count = 0;				\
    }						\
} while (0)

/* Take two locks.  Hold neither while spinning on the other.  */
#define flipc_simple_lock_acquire_2(lock1, lock2)	\
do {							\
  int __spin_count = 0;					\
							\
  while (1) {						\
    while (flipc_simple_lock_locked(lock1)		\
	   || !flipc_simple_lock_try(lock1))		\
      if (++__spin_count > LOCK_SPIN_LIMIT) {		\
	(*flipc_simple_lock_yield_fn)();		\
	__spin_count = 0;				\
      }							\
							\
    if (flipc_simple_lock_try(lock2))			\
      break;						\
    flipc_simple_lock_release(lock1);			\
							\
    while (flipc_simple_lock_locked(lock2)		\
	   || !flipc_simple_lock_try(lock2))		\
      if (++__spin_count > LOCK_SPIN_LIMIT) {		\
	(*flipc_simple_lock_yield_fn)();		\
	__spin_count = 0;				\
      }							\
							\
    if (flipc_simple_lock_try(lock1))			\
      break;						\
    flipc_simple_lock_release(lock2);			\
  }							\
} while (0)

#endif	/* _MACH_FLIPC_LOCKS_H_ */
