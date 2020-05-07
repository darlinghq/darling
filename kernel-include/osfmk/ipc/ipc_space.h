/*
 * Copyright (c) 2000-2016 Apple Computer, Inc. All rights reserved.
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
 * NOTICE: This file was modified by McAfee Research in 2004 to introduce
 * support for mandatory and extensible security protections.  This notice
 * is included in support of clause 2.2 (b) of the Apple Public License,
 * Version 2.0.
 */
/*
 */
/*
 *	File:	ipc/ipc_space.h
 *	Author:	Rich Draves
 *	Date:	1989
 *
 *	Definitions for IPC spaces of capabilities.
 */

#ifndef _IPC_IPC_SPACE_H_
#define _IPC_IPC_SPACE_H_


#include <prng/random.h>
#include <mach/mach_types.h>
#include <mach/boolean.h>
#include <mach/kern_return.h>
#include <mach/vm_types.h>

#include <sys/appleapiopts.h>

#ifdef __APPLE_API_PRIVATE
#ifdef MACH_KERNEL_PRIVATE
#include <kern/macro_help.h>
#include <kern/kern_types.h>
#include <kern/locks.h>
#include <kern/task.h>
#include <kern/zalloc.h>
#include <ipc/ipc_entry.h>
#include <ipc/ipc_types.h>

#include <libkern/OSAtomic.h>

/*
 *	Every task has a space of IPC capabilities.
 *	IPC operations like send and receive use this space.
 *	IPC kernel calls manipulate the space of the target task.
 *
 *	Every space has a non-NULL is_table with is_table_size entries.
 *
 *	Only one thread can be growing the space at a time.  Others
 *	that need it grown wait for the first.  We do almost all the
 *	work with the space unlocked, so lookups proceed pretty much
 *	unaffected while the grow operation is underway.
 */

typedef natural_t ipc_space_refs_t;
#define IS_REFS_MAX     0x0fffffff
#define IS_INACTIVE     0x40000000      /* space is inactive */
#define IS_GROWING      0x20000000      /* space is growing */
#define IS_ENTROPY_CNT  1               /* per-space entropy pool size */

struct ipc_space {
	lck_spin_t      is_lock_data;
	ipc_space_refs_t is_bits;       /* holds refs, active, growing */
	ipc_entry_num_t is_table_size;  /* current size of table */
	ipc_entry_num_t is_table_hashed;/* count of hashed elements */
	ipc_entry_num_t is_table_free;  /* count of free elements */
	ipc_entry_t is_table;           /* an array of entries */
	task_t is_task;                 /* associated task */
	struct ipc_table_size *is_table_next; /* info for larger table */
	ipc_entry_num_t is_low_mod;     /* lowest modified entry during growth */
	ipc_entry_num_t is_high_mod;    /* highest modified entry during growth */
	struct bool_gen bool_gen;       /* state for boolean RNG */
	unsigned int is_entropy[IS_ENTROPY_CNT]; /* pool of entropy taken from RNG */
	int is_node_id;                 /* HOST_LOCAL_NODE, or remote node if proxy space */
};

#define IS_NULL                 ((ipc_space_t) 0)
#define IS_INSPECT_NULL         ((ipc_space_inspect_t) 0)

#define is_active(is)           (((is)->is_bits & IS_INACTIVE) != IS_INACTIVE)

static inline void
is_mark_inactive(ipc_space_t is)
{
	assert(is_active(is));
	OSBitOrAtomic(IS_INACTIVE, &is->is_bits);
}

#define is_growing(is)          (((is)->is_bits & IS_GROWING) == IS_GROWING)

static inline void
is_start_growing(ipc_space_t is)
{
	assert(!is_growing(is));
	OSBitOrAtomic(IS_GROWING, &is->is_bits);
}

static inline void
is_done_growing(ipc_space_t is)
{
	assert(is_growing(is));
	OSBitAndAtomic(~IS_GROWING, &is->is_bits);
}

extern zone_t ipc_space_zone;

#define is_alloc()              ((ipc_space_t) zalloc(ipc_space_zone))
#define is_free(is)             zfree(ipc_space_zone, (is))

extern ipc_space_t ipc_space_kernel;
extern ipc_space_t ipc_space_reply;
#if     DIPC
extern ipc_space_t ipc_space_remote;
#endif  /* DIPC */
#if     DIPC
extern ipc_space_t default_pager_space;
#endif  /* DIPC */

extern lck_grp_t        ipc_lck_grp;
extern lck_attr_t       ipc_lck_attr;

#define is_lock_init(is)        lck_spin_init(&(is)->is_lock_data, &ipc_lck_grp, &ipc_lck_attr)
#define is_lock_destroy(is)     lck_spin_destroy(&(is)->is_lock_data, &ipc_lck_grp)

#define is_read_lock(is)        lck_spin_lock_grp(&(is)->is_lock_data, &ipc_lck_grp)
#define is_read_unlock(is)      lck_spin_unlock(&(is)->is_lock_data)
#define is_read_sleep(is)       lck_spin_sleep_grp(&(is)->is_lock_data,     \
	                                                LCK_SLEEP_DEFAULT,                                      \
	                                                (event_t)(is),                                          \
	                                                THREAD_UNINT,                                           \
	                                                &ipc_lck_grp)

#define is_write_lock(is)       lck_spin_lock_grp(&(is)->is_lock_data, &ipc_lck_grp)
#define is_write_unlock(is)     lck_spin_unlock(&(is)->is_lock_data)
#define is_write_sleep(is)      lck_spin_sleep_grp(&(is)->is_lock_data,     \
	                                                LCK_SLEEP_DEFAULT,                                      \
	                                                (event_t)(is),                                          \
	                                                THREAD_UNINT,                                           \
	                                                &ipc_lck_grp)

#define is_refs(is)             ((is)->is_bits & IS_REFS_MAX)

static inline void
is_reference(ipc_space_t is)
{
	assert(is_refs(is) > 0 && is_refs(is) < IS_REFS_MAX);
	OSIncrementAtomic(&(is->is_bits));
}


static inline void
is_release(ipc_space_t is)
{
	assert(is_refs(is) > 0);

	/* If we just removed the last reference count */
	if (1 == (OSDecrementAtomic(&(is->is_bits)) & IS_REFS_MAX)) {
		assert(!is_active(is));
		is_lock_destroy(is);
		is_free(is);
	}
}

#define current_space_fast()    (current_task_fast()->itk_space)
#define current_space()         (current_space_fast())

/* Create a special IPC space */
extern kern_return_t ipc_space_create_special(
	ipc_space_t     *spacep);

/* Create a new IPC space */
extern kern_return_t ipc_space_create(
	ipc_table_size_t        initial,
	ipc_space_t             *spacep);

/* Mark a space as dead and cleans up the entries*/
extern void ipc_space_terminate(
	ipc_space_t     space);

/* Clean up the entries - but leave the space alive */
extern void ipc_space_clean(
	ipc_space_t     space);

/* Permute the order of a range within an IPC space */
extern void ipc_space_rand_freelist(
	ipc_space_t             space,
	ipc_entry_t             table,
	mach_port_index_t       bottom,
	mach_port_index_t       top);

/* Generate a new gencount rollover point from a space's entropy pool */
extern ipc_entry_bits_t ipc_space_get_rollpoint(ipc_space_t space);

/* Allocate a new port/set/dead-name in a space */
extern kern_return_t mach_port_allocate_internal(
	ipc_space_t       space,
	mach_port_right_t right,
	mach_port_qos_t   *qosp,
	mach_port_name_t  *namep);

#endif /* MACH_KERNEL_PRIVATE */
#endif /* __APPLE_API_PRIVATE */

#ifdef  __APPLE_API_UNSTABLE
#ifndef MACH_KERNEL_PRIVATE

extern ipc_space_t              current_space(void);

#endif /* !MACH_KERNEL_PRIVATE */
#endif /* __APPLE_API_UNSTABLE */

/* Take a reference on a space */
extern void ipc_space_reference(
	ipc_space_t     space);

/* Realase a reference on a space */
extern void ipc_space_release(
	ipc_space_t     space);

#endif  /* _IPC_IPC_SPACE_H_ */
