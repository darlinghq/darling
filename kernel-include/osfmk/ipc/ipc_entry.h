/*
 * Copyright (c) 2000-2004 Apple Computer, Inc. All rights reserved.
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
/*
 *	File:	ipc/ipc_entry.h
 *	Author:	Rich Draves
 *	Date:	1989
 *
 *	Definitions for translation entries, which represent
 *	tasks' capabilities for ports and port sets.
 */

#ifndef _IPC_IPC_ENTRY_H_
#define _IPC_IPC_ENTRY_H_

#include <mach/mach_types.h>
#include <mach/port.h>
#include <mach/kern_return.h>

#include <kern/kern_types.h>
#include <kern/zalloc.h>

#include <ipc/ipc_types.h>

/*
 *	Spaces hold capabilities for ipc_object_t's.
 *	Each ipc_entry_t records a capability.  Most capabilities have
 *	small names, and the entries are elements of a table.
 *
 *	The ie_index field of entries in the table implements
 *	a ordered hash table with open addressing and linear probing.
 *
 *	The ie_dist field holds the distance to the desired spot,
 *	which is used to implement robin-hood hashing.
 *
 *	This hash table converts (space, object) -> name.
 *	It is used independently of the other fields.
 *
 *	Free (unallocated) entries in the table have null ie_object
 *	fields.  The ie_bits field is zero except for IE_BITS_GEN.
 *	The ie_next (ie_request) field links free entries into a free list.
 *
 *	The first entry in the table (index 0) is always free.
 *	It is used as the head of the free list.
 */

#define IPC_ENTRY_DIST_BITS   12
#define IPC_ENTRY_DIST_MAX    ((1 << IPC_ENTRY_DIST_BITS) - 1)
#define IPC_ENTRY_INDEX_BITS  20
#define IPC_ENTRY_INDEX_MAX   ((1 << IPC_ENTRY_INDEX_BITS) - 1)

struct ipc_entry {
	struct ipc_object  *ie_object;
	ipc_entry_bits_t    ie_bits;
	uint32_t            ie_dist  : IPC_ENTRY_DIST_BITS;
	mach_port_index_t   ie_index : IPC_ENTRY_INDEX_BITS;
	union {
		mach_port_index_t next;         /* next in freelist, or...  */
		ipc_table_index_t request;      /* dead name request notify */
	} index;
};

#define ie_request      index.request
#define ie_next         index.next

#define IE_REQ_NONE             0               /* no request */

#define IE_BITS_UREFS_MASK      0x0000ffff      /* 16 bits of user-reference */
#define IE_BITS_UREFS(bits)     ((bits) & IE_BITS_UREFS_MASK)

#define IE_BITS_TYPE_MASK       0x001f0000      /* 5 bits of capability type */
#define IE_BITS_TYPE(bits)      ((bits) & IE_BITS_TYPE_MASK)

#define IE_BITS_EXTYPE_MASK     0x00200000      /* 1 bit for extended capability */

#ifndef NO_PORT_GEN
#define IE_BITS_GEN_MASK        0xff000000      /* 8 bits for generation */
#define IE_BITS_GEN(bits)       ((bits) & IE_BITS_GEN_MASK)
#define IE_BITS_GEN_ONE         0x04000000      /* low bit of generation */
#define IE_BITS_ROLL_POS        22              /* LSB pos of generation rollover */
#define IE_BITS_ROLL_BITS       2               /* number of generation rollover bits */
#define IE_BITS_ROLL_MASK       (((1 << IE_BITS_ROLL_BITS) - 1) << IE_BITS_ROLL_POS)
#define IE_BITS_ROLL(bits)      ((((bits) & IE_BITS_ROLL_MASK) << 8) ^ IE_BITS_GEN_MASK)

/*
 * Restart a generation counter with the specified bits for the rollover point.
 * There are 4 different rollover points:
 * bits    rollover period
 * 0 0     64
 * 0 1     48
 * 1 0     32
 * 1 1     16
 */
static inline ipc_entry_bits_t
ipc_entry_new_rollpoint(
	ipc_entry_bits_t rollbits)
{
	rollbits = (rollbits << IE_BITS_ROLL_POS) & IE_BITS_ROLL_MASK;
	ipc_entry_bits_t newgen = IE_BITS_GEN_MASK + IE_BITS_GEN_ONE;
	return newgen | rollbits;
}

/*
 * Get the next gencount, modulo the entry's rollover point. If the sum rolls over,
 * the caller should re-start the generation counter with a different rollpoint.
 */
static inline ipc_entry_bits_t
ipc_entry_new_gen(
	ipc_entry_bits_t oldgen)
{
	ipc_entry_bits_t sum  = (oldgen + IE_BITS_GEN_ONE) & IE_BITS_GEN_MASK;
	ipc_entry_bits_t roll = oldgen & IE_BITS_ROLL_MASK;
	ipc_entry_bits_t newgen = (sum % IE_BITS_ROLL(oldgen)) | roll;
	return newgen;
}

/* Determine if a gencount has rolled over or not. */
static inline boolean_t
ipc_entry_gen_rolled(
	ipc_entry_bits_t oldgen,
	ipc_entry_bits_t newgen)
{
	return (oldgen & IE_BITS_GEN_MASK) > (newgen & IE_BITS_GEN_MASK);
}

#else
#define IE_BITS_GEN_MASK        0
#define IE_BITS_GEN(bits)       0
#define IE_BITS_GEN_ONE         0
#define IE_BITS_ROLL_POS        0
#define IE_BITS_ROLL_MASK       0
#define IE_BITS_ROLL(bits)      (bits)

static inline ipc_entry_bits_t
ipc_entry_new_rollpoint(
	ipc_entry_bits_t rollbits)
{
	return 0;
}

static inline ipc_entry_bits_t
ipc_entry_new_gen(
	ipc_entry_bits_t oldgen)
{
	return 0;
}

static inline boolean_t
ipc_entry_gen_rolled(
	ipc_entry_bits_t oldgen,
	ipc_entry_bits_t newgen)
{
	return FALSE;
}

#endif  /* !USE_PORT_GEN */

#define IE_BITS_RIGHT_MASK      0x007fffff      /* relevant to the right */
/*
 * Exported interfaces
 */

/* Search for entry in a space by name */
extern ipc_entry_t ipc_entry_lookup(
	ipc_space_t             space,
	mach_port_name_t        name);

/* Hold a number of entries in a locked space */
extern kern_return_t ipc_entries_hold(
	ipc_space_t             space,
	natural_t               count);

/* claim and initialize a held entry in a locked space */
extern kern_return_t ipc_entry_claim(
	ipc_space_t             space,
	mach_port_name_t        *namep,
	ipc_entry_t             *entryp);

/* Allocate an entry in a space */
extern kern_return_t ipc_entry_get(
	ipc_space_t             space,
	mach_port_name_t        *namep,
	ipc_entry_t             *entryp);

/* Allocate an entry in a space, growing the space if necessary */
extern kern_return_t ipc_entry_alloc(
	ipc_space_t             space,
	mach_port_name_t        *namep,
	ipc_entry_t             *entryp);

/* Allocate/find an entry in a space with a specific name */
extern kern_return_t ipc_entry_alloc_name(
	ipc_space_t             space,
	mach_port_name_t        name,
	ipc_entry_t             *entryp);

/* Deallocate an entry from a space */
extern void ipc_entry_dealloc(
	ipc_space_t             space,
	mach_port_name_t        name,
	ipc_entry_t             entry);

/* Mark and entry modified in a space */
extern void ipc_entry_modified(
	ipc_space_t             space,
	mach_port_name_t        name,
	ipc_entry_t             entry);

/* Grow the table in a space */
extern kern_return_t ipc_entry_grow_table(
	ipc_space_t             space,
	ipc_table_elems_t       target_size);

/* mask on/off default entry generation bits */
extern mach_port_name_t ipc_entry_name_mask(
	mach_port_name_t name);
#endif  /* _IPC_IPC_ENTRY_H_ */
