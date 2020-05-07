/*
 * Copyright (c) 2000-2007 Apple Inc. All rights reserved.
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
 *	File:	ipc/ipc_object.h
 *	Author:	Rich Draves
 *	Date:	1989
 *
 *	Definitions for IPC objects, for which tasks have capabilities.
 */

#ifndef _IPC_IPC_OBJECT_H_
#define _IPC_IPC_OBJECT_H_

#include <mach/kern_return.h>
#include <mach/message.h>
#include <kern/locks.h>
#include <kern/macro_help.h>
#include <kern/assert.h>
#include <kern/zalloc.h>
#include <ipc/ipc_types.h>
#include <libkern/OSAtomic.h>

typedef natural_t ipc_object_refs_t;    /* for ipc/ipc_object.h		*/
typedef natural_t ipc_object_bits_t;
typedef natural_t ipc_object_type_t;

/*
 * The ipc_object is used to both tag and reference count these two data
 * structures, and (Noto Bene!) pointers to either of these or the
 * ipc_object at the head of these are freely cast back and forth; hence
 * the ipc_object MUST BE FIRST in the ipc_common_data.
 *
 * If the RPC implementation enabled user-mode code to use kernel-level
 * data structures (as ours used to), this peculiar structuring would
 * avoid having anything in user code depend on the kernel configuration
 * (with which lock size varies).
 */
struct ipc_object {
	ipc_object_bits_t io_bits;
	ipc_object_refs_t io_references;
	lck_spin_t        io_lock_data;
} __attribute__((aligned(8)));

/*
 * If another object type needs to participate in io_kotype()-based
 * dispatching, it must include a stub structure as the first
 * element
 */
struct ipc_object_header {
	ipc_object_bits_t io_bits;
#ifdef __LP64__
	natural_t         io_padding; /* pad to natural boundary */
#endif
};

/*
 * Legacy defines.  Should use IPC_OBJECT_NULL, etc...
 */
#define IO_NULL                 ((ipc_object_t) 0)
#define IO_DEAD                 ((ipc_object_t) ~0UL)
#define IO_VALID(io)            (((io) != IO_NULL) && ((io) != IO_DEAD))

/*
 *	IPC steals the high-order bits from the kotype to use
 *	for its own purposes.  This allows IPC to record facts
 *	about ports that aren't otherwise obvious from the
 *	existing port fields.  In particular, IPC can optionally
 *	mark a port for no more senders detection.  Any change
 *	to IO_BITS_PORT_INFO must be coordinated with bitfield
 *	definitions in ipc_port.h.
 */
#define IO_BITS_PORT_INFO       0x0000f000      /* stupid port tricks */
#define IO_BITS_KOTYPE          0x000007ff      /* used by the object */
#define IO_BITS_KOBJECT         0x00000800      /* port belongs to a kobject */
#define IO_BITS_OTYPE           0x7fff0000      /* determines a zone */
#define IO_BITS_ACTIVE          0x80000000      /* is object alive? */

#define io_active(io)           (((io)->io_bits & IO_BITS_ACTIVE) != 0)

#define io_otype(io)            (((io)->io_bits & IO_BITS_OTYPE) >> 16)
#define io_kotype(io)           ((io)->io_bits & IO_BITS_KOTYPE)
#define io_is_kobject(io)       (((io)->io_bits & IO_BITS_KOBJECT) != IKOT_NONE)

#define io_makebits(active, otype, kotype)      \
	(((active) ? IO_BITS_ACTIVE : 0) | ((otype) << 16) | (kotype))

/*
 * Object types: ports, port sets, kernel-loaded ports
 */
#define IOT_PORT                0
#define IOT_PORT_SET            1
#define IOT_NUMBER              2               /* number of types used */

extern zone_t ipc_object_zones[IOT_NUMBER];
extern lck_grp_t        ipc_lck_grp;

#define io_alloc(otype)         \
	        ((ipc_object_t) zalloc(ipc_object_zones[(otype)]))

extern void     io_free(
	unsigned int    otype,
	ipc_object_t    object);

/*
 * Here we depend on the ipc_object being first within the kernel struct
 * (ipc_port and ipc_pset).
 */
#define io_lock_init(io) \
	lck_spin_init(&(io)->io_lock_data, &ipc_lck_grp, &ipc_lck_attr)
#define io_lock_destroy(io) \
	lck_spin_destroy(&(io)->io_lock_data, &ipc_lck_grp)
#define io_lock_held(io) \
	LCK_SPIN_ASSERT(&(io)->io_lock_data, LCK_ASSERT_OWNED)
#define io_lock_held_kdp(io) \
	kdp_lck_spin_is_acquired(&(io)->io_lock_data)
#define io_unlock(io) \
	lck_spin_unlock(&(io)->io_lock_data)

extern void io_lock(
	ipc_object_t io);
extern boolean_t io_lock_try(
	ipc_object_t io);

#define _VOLATILE_ volatile

/* Sanity check the ref count.  If it is 0, we may be doubly zfreeing.
 * If it is larger than max int, it has been corrupted or leaked,
 * probably by being modified into an address (this is architecture
 * dependent, but it's safe to assume there cannot really be max int
 * references unless some code is leaking the io_reference without leaking
 * object). Saturate the io_reference on release kernel if it reaches
 * max int to avoid use after free.
 *
 * NOTE: The 0 test alone will not catch double zfreeing of ipc_port
 * structs, because the io_references field is the first word of the struct,
 * and zfree modifies that to point to the next free zone element.
 */
#define IO_MAX_REFERENCES                                               \
	(unsigned)(~0 ^ (1U << (sizeof(int)*BYTE_SIZE - 1)))

static inline void
io_reference(ipc_object_t io)
{
	ipc_object_refs_t new_io_references;
	ipc_object_refs_t old_io_references;

	if ((io)->io_references == 0 ||
	    (io)->io_references >= IO_MAX_REFERENCES) {
		panic("%s: reference count %u is invalid\n", __func__, (io)->io_references);
	}

	do {
		old_io_references = (io)->io_references;
		new_io_references = old_io_references + 1;
		if (old_io_references == IO_MAX_REFERENCES) {
			break;
		}
	} while (OSCompareAndSwap(old_io_references, new_io_references,
	    &((io)->io_references)) == FALSE);
}


static inline void
io_release(ipc_object_t io)
{
	ipc_object_refs_t new_io_references;
	ipc_object_refs_t old_io_references;

	if ((io)->io_references == 0 ||
	    (io)->io_references >= IO_MAX_REFERENCES) {
		panic("%s: reference count %u is invalid\n", __func__, (io)->io_references);
	}

	do {
		old_io_references = (io)->io_references;
		new_io_references = old_io_references - 1;
		if (old_io_references == IO_MAX_REFERENCES) {
			break;
		}
	} while (OSCompareAndSwap(old_io_references, new_io_references,
	    &((io)->io_references)) == FALSE);

	/* If we just removed the last reference count */
	if (1 == old_io_references) {
		/* Free the object */
		io_free(io_otype((io)), (io));
	}
}

/*
 * Retrieve a label for use in a kernel call that takes a security
 * label as a parameter. If necessary, io_getlabel acquires internal
 * (not io_lock) locks, and io_unlocklabel releases them.
 */

struct label;
extern struct label *io_getlabel(ipc_object_t obj);
#define io_unlocklabel(obj)

/*
 * Exported interfaces
 */

/* Take a reference to an object */
extern void ipc_object_reference(
	ipc_object_t    object);

/* Release a reference to an object */
extern void ipc_object_release(
	ipc_object_t    object);

/* Look up an object in a space */
extern kern_return_t ipc_object_translate(
	ipc_space_t             space,
	mach_port_name_t        name,
	mach_port_right_t       right,
	ipc_object_t            *objectp);

/* Look up two objects in a space, locking them in the order described */
extern kern_return_t ipc_object_translate_two(
	ipc_space_t             space,
	mach_port_name_t        name1,
	mach_port_right_t       right1,
	ipc_object_t            *objectp1,
	mach_port_name_t        name2,
	mach_port_right_t       right2,
	ipc_object_t            *objectp2);

/* Validate an object as belonging to the correct zone */
extern void ipc_object_validate(
	ipc_object_t object);

/* Allocate a dead-name entry */
extern kern_return_t
ipc_object_alloc_dead(
	ipc_space_t             space,
	mach_port_name_t        *namep);

/*  Allocate a dead-name entry, with a specific name */
extern kern_return_t ipc_object_alloc_dead_name(
	ipc_space_t             space,
	mach_port_name_t        name);

/* Allocate an object */
extern kern_return_t ipc_object_alloc(
	ipc_space_t             space,
	ipc_object_type_t       otype,
	mach_port_type_t        type,
	mach_port_urefs_t       urefs,
	mach_port_name_t        *namep,
	ipc_object_t            *objectp);

/* Allocate an object, with a specific name */
extern kern_return_t ipc_object_alloc_name(
	ipc_space_t             space,
	ipc_object_type_t       otype,
	mach_port_type_t        type,
	mach_port_urefs_t       urefs,
	mach_port_name_t        name,
	ipc_object_t            *objectp);

/* Convert a send type name to a received type name */
extern mach_msg_type_name_t ipc_object_copyin_type(
	mach_msg_type_name_t    msgt_name);

/* Copyin a capability from a space */
extern kern_return_t ipc_object_copyin(
	ipc_space_t             space,
	mach_port_name_t        name,
	mach_msg_type_name_t    msgt_name,
	ipc_object_t            *objectp,
	mach_port_context_t     context,
	mach_msg_guard_flags_t  *guard_flags,
	uint32_t                kmsg_flags);

/* Copyin a naked capability from the kernel */
extern void ipc_object_copyin_from_kernel(
	ipc_object_t            object,
	mach_msg_type_name_t    msgt_name);

/* Destroy a naked capability */
extern void ipc_object_destroy(
	ipc_object_t            object,
	mach_msg_type_name_t    msgt_name);

/* Destroy a naked destination capability */
extern void ipc_object_destroy_dest(
	ipc_object_t            object,
	mach_msg_type_name_t    msgt_name);

/* Insert a send right into an object already in the current space */
extern kern_return_t ipc_object_insert_send_right(
	ipc_space_t             space,
	mach_port_name_t        name,
	mach_msg_type_name_t    msgt_name);

/* Copyout a capability, placing it into a space */
extern kern_return_t ipc_object_copyout(
	ipc_space_t             space,
	ipc_object_t            object,
	mach_msg_type_name_t    msgt_name,
	mach_port_context_t     *context,
	mach_msg_guard_flags_t  *guard_flags,
	mach_port_name_t        *namep);

/* Copyout a capability with a name, placing it into a space */
extern kern_return_t ipc_object_copyout_name(
	ipc_space_t             space,
	ipc_object_t            object,
	mach_msg_type_name_t    msgt_name,
	mach_port_name_t        name);

/* Translate/consume the destination right of a message */
extern void ipc_object_copyout_dest(
	ipc_space_t             space,
	ipc_object_t            object,
	mach_msg_type_name_t    msgt_name,
	mach_port_name_t        *namep);

#endif  /* _IPC_IPC_OBJECT_H_ */
