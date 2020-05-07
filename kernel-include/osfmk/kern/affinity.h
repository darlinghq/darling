/*
 * Copyright (c) 2007-2008 Apple Inc. All rights reserved.
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

#ifdef  XNU_KERNEL_PRIVATE

#ifndef _KERN_AFFINITY_H_
#define _KERN_AFFINITY_H_

#ifdef  MACH_KERNEL_PRIVATE

#include <kern/queue.h>
#include <kern/processor.h>

/*
 * An affinity set object represents a set of threads identified by the user
 * to be sharing (cache) affinity. A task may have multiple affinity sets
 * defined. Each set has dis-affinity other sets. Tasks related by inheritance
 * may share the same affinity set namespace.
 * Affinity sets are used to advise (hint) thread placement.
 */
struct affinity_set {
	struct affinity_space *aset_space;      /* namespace */
	queue_chain_t   aset_affinities;        /* links affinities in group */
	queue_head_t    aset_threads;           /* threads in affinity set */
	uint32_t        aset_thread_count;      /* num threads in set */
	uint32_t        aset_tag;               /* user-assigned tag */
	uint32_t        aset_num;               /* kernel-assigned affinity */
	processor_set_t aset_pset;              /* processor set */
};

extern boolean_t        thread_affinity_is_supported(void);
extern void             thread_affinity_dup(thread_t parent, thread_t child);
extern void             thread_affinity_terminate(thread_t thread);
extern void             task_affinity_create(
	task_t,
	task_t);
extern void             task_affinity_deallocate(
	task_t);
extern kern_return_t    task_affinity_info(
	task_t,
	task_info_t,
	mach_msg_type_number_t  *);

#endif  /* MACH_KERNEL_PRIVATE */

extern kern_return_t    thread_affinity_set(thread_t thread, uint32_t tag);
extern uint32_t         thread_affinity_get(thread_t thread);
extern void             thread_affinity_exec(thread_t thread);

#endif  /* _KERN_AFFINITY_H_ */

#endif  /* XNU_KERNEL_PRIVATE */
