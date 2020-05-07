/*
 * Copyright (c) 2000-2005 Apple Computer, Inc. All rights reserved.
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
 * Copyright (C) 1998 Apple Computer
 * All Rights Reserved
 */
/*
 * @OSF_COPYRIGHT@
 */
/*
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988,1987 Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 *	File:	kern/simple_lock.h (derived from kern/lock.h)
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *	Date:	1985
 *
 *	Atomic primitives and Simple Locking primitives definitions
 */

#ifdef  KERNEL_PRIVATE

#ifndef _KERN_SIMPLE_LOCK_H_
#define _KERN_SIMPLE_LOCK_H_

#include <sys/cdefs.h>
#include <mach/boolean.h>
#include <kern/kern_types.h>
#include <kern/lock_group.h>
#include <machine/simple_lock.h>

#ifdef  MACH_KERNEL_PRIVATE
#include <machine/atomic.h>
#include <mach_ldebug.h>

extern void                     hw_lock_init(
	hw_lock_t);

#if LOCK_STATS
extern void                     hw_lock_lock(
	hw_lock_t,
	lck_grp_t*);

extern void                     hw_lock_lock_nopreempt(
	hw_lock_t,
	lck_grp_t*);

extern unsigned int             hw_lock_to(
	hw_lock_t,
	uint64_t,
	lck_grp_t*);

extern unsigned int             hw_lock_try(
	hw_lock_t,
	lck_grp_t*);

extern unsigned int             hw_lock_try_nopreempt(
	hw_lock_t,
	lck_grp_t*);

#else

extern void                     hw_lock_lock(
	hw_lock_t);

#define hw_lock_lock(lck, grp) hw_lock_lock(lck)

extern void                     hw_lock_lock_nopreempt(
	hw_lock_t);
#define hw_lock_lock_nopreempt(lck, grp) hw_lock_lock_nopreempt(lck)

extern unsigned int             hw_lock_to(
	hw_lock_t,
	uint64_t);
#define hw_lock_to(lck, timeout, grp) hw_lock_to(lck, timeout)


extern unsigned int             hw_lock_try(
	hw_lock_t);
#define hw_lock_try(lck, grp) hw_lock_try(lck)

extern unsigned int             hw_lock_try_nopreempt(
	hw_lock_t);
#define hw_lock_try_nopreempt(lck, grp) hw_lock_try_nopreempt(lck)


#endif /* LOCK_STATS */

extern void                     hw_lock_unlock(
	hw_lock_t);

extern void                     hw_lock_unlock_nopreempt(
	hw_lock_t);

extern unsigned int             hw_lock_held(
	hw_lock_t);

extern boolean_t hw_atomic_test_and_set32(uint32_t *target, uint32_t test_mask, uint32_t set_mask, enum memory_order ord, boolean_t wait);
#endif  /* MACH_KERNEL_PRIVATE */

__BEGIN_DECLS

extern void *                   hw_wait_while_equals(
	void    **address,
	void    *current);

extern void                     usimple_lock_init(
	usimple_lock_t,
	unsigned short);

#if LOCK_STATS
extern void                     usimple_lock(
	usimple_lock_t,
	lck_grp_t*);

extern unsigned int             usimple_lock_try(
	usimple_lock_t,
	lck_grp_t*);

extern void             usimple_lock_try_lock_loop(
	usimple_lock_t,
	lck_grp_t*);

#if defined(__x86_64__)
extern unsigned int     usimple_lock_try_lock_mp_signal_safe_loop_deadline(
	usimple_lock_t,
	uint64_t,
	lck_grp_t*);

extern unsigned int     usimple_lock_try_lock_mp_signal_safe_loop_duration(
	usimple_lock_t,
	uint64_t,
	lck_grp_t*);
#endif

#else
extern void                     usimple_lock(
	usimple_lock_t);
#define usimple_lock(lck, grp) usimple_lock(lck)


extern unsigned int             usimple_lock_try(
	usimple_lock_t);

#define usimple_lock_try(lck, grp) usimple_lock_try(lck)

extern void             usimple_lock_try_lock_loop(
	usimple_lock_t);
#define usimple_lock_try_lock_loop(lck, grp) usimple_lock_try_lock_loop(lck)

#if defined(__x86_64__)
extern unsigned int     usimple_lock_try_lock_mp_signal_safe_loop_deadline(
	usimple_lock_t,
	uint64_t);
#define usimple_lock_try_lock_mp_signal_safe_loop_deadline(lck, ddl, grp) usimple_lock_try_lock_mp_signal_safe_loop_deadline(lck, ddl)

extern unsigned int     usimple_lock_try_lock_mp_signal_safe_loop_duration(
	usimple_lock_t,
	uint64_t);
#define usimple_lock_try_lock_mp_signal_safe_loop_duration(lck, dur, grp) usimple_lock_try_lock_mp_signal_safe_loop_duration(lck, dur)
#endif

#endif /* LOCK_STATS */

extern void                     usimple_unlock(
	usimple_lock_t);


__END_DECLS

#define ETAP_NO_TRACE   0
#define ETAP_IO_AHA             0

/*
 * If we got to here and we still don't have simple_lock_init
 * defined, then we must either be outside the osfmk component,
 * running on a true SMP, or need debug.
 */
#if !defined(simple_lock_init)
#define simple_lock_init(l, t)   usimple_lock_init(l,t)
#define simple_lock(l, grp)          usimple_lock(l, grp)
#define simple_unlock(l)        usimple_unlock(l)
#define simple_lock_try(l, grp)      usimple_lock_try(l, grp)
#define simple_lock_try_lock_loop(l, grp)    usimple_lock_try_lock_loop(l, grp)
#define simple_lock_try_lock_mp_signal_safe_loop_deadline(l, ddl, grp)    usimple_lock_try_lock_mp_signal_safe_loop_deadline(l, ddl, grp)
#define simple_lock_try_lock_mp_signal_safe_loop_duration(l, dur, grp)    usimple_lock_try_lock_mp_signal_safe_loop_duration(l, dur, grp)
#define simple_lock_addr(l)     (&(l))
#endif /* !defined(simple_lock_init) */

#ifdef MACH_KERNEL_PRIVATE

typedef uint32_t hw_lock_bit_t;

#if LOCK_STATS
extern void     hw_lock_bit(
	hw_lock_bit_t *,
	unsigned int,
	lck_grp_t*);

extern void     hw_lock_bit_nopreempt(
	hw_lock_bit_t *,
	unsigned int,
	lck_grp_t*);

extern unsigned int hw_lock_bit_try(
	hw_lock_bit_t *,
	unsigned int,
	lck_grp_t*);

extern unsigned int hw_lock_bit_to(
	hw_lock_bit_t *,
	unsigned int,
	uint32_t,
	lck_grp_t*);

#else
extern void     hw_lock_bit(
	hw_lock_bit_t *,
	unsigned int);
#define hw_lock_bit(lck, bit, grp) hw_lock_bit(lck, bit)

extern void     hw_lock_bit_nopreempt(
	hw_lock_bit_t *,
	unsigned int);
#define hw_lock_bit_nopreempt(lck, bit, grp) hw_lock_bit_nopreempt(lck, bit)

extern unsigned int hw_lock_bit_try(
	hw_lock_bit_t *,
	unsigned int);
#define hw_lock_bit_try(lck, bit, grp) hw_lock_bit_try(lck, bit)

extern unsigned int hw_lock_bit_to(
	hw_lock_bit_t *,
	unsigned int,
	uint32_t);
#define hw_lock_bit_to(lck, bit, timeout, grp) hw_lock_bit_to(lck, bit, timeout)

#endif /* LOCK_STATS */

extern void     hw_unlock_bit(
	hw_lock_bit_t *,
	unsigned int);

extern void     hw_unlock_bit_nopreempt(
	hw_lock_bit_t *,
	unsigned int);

#define hw_lock_bit_held(l, b) (((*(l))&(1<<b))!=0)

#endif  /* MACH_KERNEL_PRIVATE */

#endif /*!_KERN_SIMPLE_LOCK_H_*/

#endif  /* KERNEL_PRIVATE */
