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
 * @OSF_COPYRIGHT@
 */
/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
 */

#ifndef	_MACH_DEBUG_ZONE_INFO_H_
#define _MACH_DEBUG_ZONE_INFO_H_

#include <mach/boolean.h>
#include <mach/machine/vm_types.h>

/*
 *	Legacy definitions for host_zone_info().  This interface, and
 *	these definitions have been deprecated in favor of the new
 *	mach_zone_info() inteface and types below.
 */

#define ZONE_NAME_MAX_LEN		80

typedef struct zone_name {
	char		zn_name[ZONE_NAME_MAX_LEN];
} zone_name_t;

typedef zone_name_t *zone_name_array_t;


typedef struct zone_info {
	integer_t	zi_count;	/* Number of elements used now */
	vm_size_t	zi_cur_size;	/* current memory utilization */
	vm_size_t	zi_max_size;	/* how large can this zone grow */
	vm_size_t	zi_elem_size;	/* size of an element */
	vm_size_t	zi_alloc_size;	/* size used for more memory */
	integer_t	zi_pageable;	/* zone pageable? */
	integer_t	zi_sleepable;	/* sleep if empty? */
	integer_t	zi_exhaustible;	/* merely return if empty? */
	integer_t	zi_collectable;	/* garbage collect elements? */
} zone_info_t;

typedef zone_info_t *zone_info_array_t;


/*
 *	Remember to update the mig type definitions
 *	in mach_debug_types.defs when adding/removing fields.
 */

#define MACH_ZONE_NAME_MAX_LEN		80

typedef struct mach_zone_name {
	char		mzn_name[ZONE_NAME_MAX_LEN];
} mach_zone_name_t;

typedef mach_zone_name_t *mach_zone_name_array_t;

typedef struct mach_zone_info_data {
	uint64_t	mzi_count;	/* count of elements in use */
	uint64_t	mzi_cur_size;	/* current memory utilization */
	uint64_t	mzi_max_size;	/* how large can this zone grow */
        uint64_t	mzi_elem_size;	/* size of an element */
	uint64_t	mzi_alloc_size;	/* size used for more memory */
	uint64_t	mzi_sum_size;	/* sum of all allocs (life of zone) */
	uint64_t	mzi_exhaustible;	/* merely return if empty? */
	uint64_t	mzi_collectable;	/* garbage collect elements? */
} mach_zone_info_t;

typedef mach_zone_info_t *mach_zone_info_array_t;

typedef struct task_zone_info_data {
	uint64_t	tzi_count;	/* count of elements in use */
	uint64_t	tzi_cur_size;	/* current memory utilization */
	uint64_t	tzi_max_size;	/* how large can this zone grow */
        uint64_t	tzi_elem_size;	/* size of an element */
	uint64_t	tzi_alloc_size;	/* size used for more memory */
	uint64_t	tzi_sum_size;	/* sum of all allocs (life of zone) */
	uint64_t	tzi_exhaustible;	/* merely return if empty? */
	uint64_t	tzi_collectable;	/* garbage collect elements? */
	uint64_t	tzi_caller_acct;	/* charged to caller (or kernel) */
	uint64_t	tzi_task_alloc;	/* sum of all allocs by this task */
	uint64_t	tzi_task_free;	/* sum of all frees by this task */
} task_zone_info_t;

typedef task_zone_info_t *task_zone_info_array_t;
#endif	/* _MACH_DEBUG_ZONE_INFO_H_ */
