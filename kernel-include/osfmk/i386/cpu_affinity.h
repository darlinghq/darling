/*
 * Copyright (c) 2003-2007 Apple Inc. All rights reserved.
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
#ifdef KERNEL_PRIVATE
#ifndef _I386_CPU_AFFINITY_H_
#define _I386_CPU_AFFINITY_H_

#include <i386/cpu_topology.h>

typedef struct x86_affinity_set {
	struct x86_affinity_set     *next;/* Forward link */
	struct x86_cpu_cache        *cache;/* The L2 cache concerned */
	processor_set_t             pset;/* The processor set container */
	uint32_t                    num;/* Logical id */
} x86_affinity_set_t;

extern x86_affinity_set_t *x86_affinities;      /* root of all affinities */

extern int              ml_get_max_affinity_sets(void);
extern processor_set_t  ml_affinity_to_pset(uint32_t affinity_num);

#endif /* _I386_CPU_AFFINITY_H_ */
#endif /* KERNEL_PRIVATE */
