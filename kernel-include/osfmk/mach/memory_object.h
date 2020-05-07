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
 * Copyright (c) 1991,1990,1989,1988 Carnegie Mellon University
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
/*
 *	File:	memory_object.h
 *	Author:	Michael Wayne Young
 *
 *	External memory management interface definition.
 */

#ifndef _MACH_MEMORY_OBJECT_H_
#define _MACH_MEMORY_OBJECT_H_

/*
 *	User-visible types used in the external memory
 *	management interface:
 */

#include <mach/port.h>
#include <mach/message.h>
#include <mach/machine/vm_types.h>

typedef mach_port_t     memory_object_t;
/* A memory object ... */
/*  Used by the kernel to retrieve */
/*  or store data */

typedef mach_port_t     memory_object_control_t;
/* Provided to a memory manager; ... */
/*  used to control a memory object */

typedef mach_port_t     memory_object_name_t;
/* Used to describe the memory ... */
/*  object in vm_regions() calls */

typedef mach_port_t     memory_object_rep_t;
/* Per-client handle for mem object */
/*  Used by user programs to specify */
/*  the object to map */

typedef int             memory_object_copy_strategy_t;
/* How memory manager handles copy: */
#define         MEMORY_OBJECT_COPY_NONE         0
/* ... No special support */
#define         MEMORY_OBJECT_COPY_CALL         1
/* ... Make call on memory manager */
#define         MEMORY_OBJECT_COPY_DELAY        2
/* ... Memory manager doesn't
 *     change data externally.
 */
#define         MEMORY_OBJECT_COPY_TEMPORARY    3
/* ... Memory manager doesn't
 *     change data externally, and
 *     doesn't need to see changes.
 */
#define         MEMORY_OBJECT_COPY_SYMMETRIC    4
/* ... Memory manager doesn't
 *     change data externally,
 *     doesn't need to see changes,
 *     and object will not be
 *     multiply mapped.
 *
 *     XXX
 *     Not yet safe for non-kernel use.
 */

#define         MEMORY_OBJECT_COPY_INVALID      5
/* ...	An invalid copy strategy,
 *	for external objects which
 *	have not been initialized.
 *	Allows copy_strategy to be
 *	examined without also
 *	examining pager_ready and
 *	internal.
 */

typedef int             memory_object_return_t;
/* Which pages to return to manager
 *  this time (lock_request) */
#define         MEMORY_OBJECT_RETURN_NONE       0
/* ... don't return any. */
#define         MEMORY_OBJECT_RETURN_DIRTY      1
/* ... only dirty pages. */
#define         MEMORY_OBJECT_RETURN_ALL        2
/* ... dirty and precious pages. */
#define         MEMORY_OBJECT_RETURN_ANYTHING   3
/* ... any resident page. */

#define         MEMORY_OBJECT_NULL      MACH_PORT_NULL


/*
 *	Types for the memory object flavor interfaces
 */

#define MEMORY_OBJECT_INFO_MAX      (1024)
typedef int     *memory_object_info_t;
typedef int      memory_object_flavor_t;
typedef int      memory_object_info_data_t[MEMORY_OBJECT_INFO_MAX];


#define OLD_MEMORY_OBJECT_BEHAVIOR_INFO         10
#define MEMORY_OBJECT_PERFORMANCE_INFO  11
#define OLD_MEMORY_OBJECT_ATTRIBUTE_INFO        12
#define MEMORY_OBJECT_ATTRIBUTE_INFO    14
#define MEMORY_OBJECT_BEHAVIOR_INFO     15


struct old_memory_object_behave_info {
	memory_object_copy_strategy_t   copy_strategy;
	boolean_t                       temporary;
	boolean_t                       invalidate;
};

struct memory_object_perf_info {
	memory_object_cluster_size_t    cluster_size;
	boolean_t                                               may_cache;
};

struct old_memory_object_attr_info {                    /* old attr list */
	boolean_t                       object_ready;
	boolean_t                       may_cache;
	memory_object_copy_strategy_t   copy_strategy;
};

struct memory_object_attr_info {
	memory_object_copy_strategy_t   copy_strategy;
	memory_object_cluster_size_t    cluster_size;
	boolean_t                       may_cache_object;
	boolean_t                       temporary;
};

struct memory_object_behave_info {
	memory_object_copy_strategy_t   copy_strategy;
	boolean_t                       temporary;
	boolean_t                       invalidate;
	boolean_t                       silent_overwrite;
	boolean_t                       advisory_pageout;
};

typedef struct old_memory_object_behave_info *old_memory_object_behave_info_t;
typedef struct old_memory_object_behave_info old_memory_object_behave_info_data_t;

typedef struct memory_object_behave_info *memory_object_behave_info_t;
typedef struct memory_object_behave_info memory_object_behave_info_data_t;

typedef struct memory_object_perf_info  *memory_object_perf_info_t;
typedef struct memory_object_perf_info  memory_object_perf_info_data_t;

typedef struct old_memory_object_attr_info *old_memory_object_attr_info_t;
typedef struct old_memory_object_attr_info old_memory_object_attr_info_data_t;

typedef struct memory_object_attr_info  *memory_object_attr_info_t;
typedef struct memory_object_attr_info  memory_object_attr_info_data_t;

#define OLD_MEMORY_OBJECT_BEHAVE_INFO_COUNT     ((mach_msg_type_number_t) \
	        (sizeof(old_memory_object_behave_info_data_t)/sizeof(int)))
#define MEMORY_OBJECT_BEHAVE_INFO_COUNT         ((mach_msg_type_number_t) \
	        (sizeof(memory_object_behave_info_data_t)/sizeof(int)))
#define MEMORY_OBJECT_PERF_INFO_COUNT           ((mach_msg_type_number_t) \
	        (sizeof(memory_object_perf_info_data_t)/sizeof(int)))
#define OLD_MEMORY_OBJECT_ATTR_INFO_COUNT       ((mach_msg_type_number_t) \
	        (sizeof(old_memory_object_attr_info_data_t)/sizeof(int)))
#define MEMORY_OBJECT_ATTR_INFO_COUNT           ((mach_msg_type_number_t) \
	        (sizeof(memory_object_attr_info_data_t)/sizeof(int)))

#define invalid_memory_object_flavor(f)                                 \
	(f != MEMORY_OBJECT_ATTRIBUTE_INFO &&                           \
	 f != MEMORY_OBJECT_PERFORMANCE_INFO &&                         \
	 f != OLD_MEMORY_OBJECT_BEHAVIOR_INFO &&                        \
	 f != MEMORY_OBJECT_BEHAVIOR_INFO &&                            \
	 f != OLD_MEMORY_OBJECT_ATTRIBUTE_INFO)

#endif  /* _MACH_MEMORY_OBJECT_H_ */
