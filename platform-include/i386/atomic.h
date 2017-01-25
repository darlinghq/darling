/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

#ifndef _I386_ATOMIC_H_
#define _I386_ATOMIC_H_

#include <i386/smp.h>

#if	__SMP__

#define memory_order_consume_smp memory_order_consume
#define memory_order_acquire_smp memory_order_acquire
#define memory_order_release_smp memory_order_release
#define memory_order_acq_rel_smp memory_order_acq_rel
#define memory_order_seq_cst_smp memory_order_seq_cst

#else

#define memory_order_consume_smp memory_order_relaxed
#define memory_order_acquire_smp memory_order_relaxed
#define memory_order_release_smp memory_order_relaxed
#define memory_order_acq_rel_smp memory_order_relaxed
#define memory_order_seq_cst_smp memory_order_relaxed

#endif

#ifdef ATOMIC_PRIVATE

static boolean_t
atomic_compare_exchange(uintptr_t *target, uintptr_t oldval, uintptr_t newval,
			enum memory_order ord, boolean_t wait)
{
	(void)wait;
	return __c11_atomic_compare_exchange_strong((_Atomic uintptr_t *)target, &oldval, newval, ord, memory_order_relaxed);
}

#endif // ATOMIC_PRIVATE


#endif // _I386_ATOMIC_H_

