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
 *	File:	ipc/ipc_right.h
 *	Author:	Rich Draves
 *	Date:	1989
 *
 *	Declarations of functions to manipulate IPC capabilities.
 */

#ifndef _IPC_IPC_RIGHT_H_
#define _IPC_IPC_RIGHT_H_

#include <mach/boolean.h>
#include <mach/kern_return.h>
#include <ipc/ipc_port.h>
#include <ipc/ipc_entry.h>

#define ipc_right_lookup_read           ipc_right_lookup_write
#define ipc_right_lookup_two_read       ipc_right_lookup_two_write

typedef uint32_t ipc_right_copyin_flags_t;

#define IPC_RIGHT_COPYIN_FLAGS_NONE                   0x0
#define IPC_RIGHT_COPYIN_FLAGS_DEADOK                 0x1
#define IPC_RIGHT_COPYIN_FLAGS_ALLOW_IMMOVABLE_SEND   0x2
#define IPC_RIGHT_COPYIN_FLAGS_ALLOW_DEAD_SEND_ONCE   0x4   /* allow copyin of a send once right to a dead port with no dead name requests */

/* Find an entry in a space, given the name */
extern kern_return_t ipc_right_lookup_write(
	ipc_space_t             space,
	mach_port_name_t        name,
	ipc_entry_t             *entryp);

/* Find two entries in a space, given two names */
extern kern_return_t ipc_right_lookup_two_write(
	ipc_space_t             space,
	mach_port_name_t        name1,
	ipc_entry_t             *entryp1,
	mach_port_name_t        name2,
	ipc_entry_t             *entryp2);

/* Translate (space, object) -> (name, entry) */
extern boolean_t ipc_right_reverse(
	ipc_space_t             space,
	ipc_object_t            object,
	mach_port_name_t        *namep,
	ipc_entry_t             *entryp);

/* Make a notification request, returning the previous send-once right */
extern kern_return_t ipc_right_request_alloc(
	ipc_space_t             space,
	mach_port_name_t        name,
	boolean_t               immediate,
	boolean_t               send_possible,
	ipc_port_t              notify,
	ipc_port_t              *previousp);

/* Cancel a notification request and return the send-once right */
extern ipc_port_t ipc_right_request_cancel(
	ipc_space_t             space,
	ipc_port_t              port,
	mach_port_name_t        name,
	ipc_entry_t             entry);

#define ipc_right_request_cancel_macro(space, port, name, entry)                \
	         ((entry->ie_request == IE_REQ_NONE) ? IP_NULL :                \
	         ipc_right_request_cancel((space), (port), (name), (entry)))

/* Check if an entry is being used */
extern boolean_t ipc_right_inuse(
	ipc_space_t             space,
	mach_port_name_t        name,
	ipc_entry_t             entry);

/* Check if the port has died */
extern boolean_t ipc_right_check(
	ipc_space_t              space,
	ipc_port_t               port,
	mach_port_name_t         name,
	ipc_entry_t              entry,
	ipc_right_copyin_flags_t flags);

/* Clean up an entry in a dead space */
extern void ipc_right_terminate(
	ipc_space_t             space,
	mach_port_name_t        name,
	ipc_entry_t             entry);

/* Destroy an entry in a space */
extern kern_return_t ipc_right_destroy(
	ipc_space_t             space,
	mach_port_name_t        name,
	ipc_entry_t             entry,
	boolean_t               check_guard,
	uint64_t                guard);

/* Release a send/send-once/dead-name user reference */
extern kern_return_t ipc_right_dealloc(
	ipc_space_t             space,
	mach_port_name_t        name,
	ipc_entry_t             entry);

/* Modify the user-reference count for a right */
extern kern_return_t ipc_right_delta(
	ipc_space_t             space,
	mach_port_name_t        name,
	ipc_entry_t             entry,
	mach_port_right_t       right,
	mach_port_delta_t       delta);

/* Destroy a receive right; Modify ref count for send rights */
extern kern_return_t ipc_right_destruct(
	ipc_space_t             space,
	mach_port_name_t        name,
	ipc_entry_t             entry,
	mach_port_delta_t       srdelta,
	uint64_t                guard);

/* Retrieve information about a right */
extern kern_return_t ipc_right_info(
	ipc_space_t             space,
	mach_port_name_t        name,
	ipc_entry_t             entry,
	mach_port_type_t        *typep,
	mach_port_urefs_t       *urefsp);

/* Check if a subsequent ipc_right_copyin of the reply port will succeed */
extern boolean_t ipc_right_copyin_check_reply(
	ipc_space_t              space,
	mach_port_name_t         reply_name,
	ipc_entry_t              reply_entry,
	mach_msg_type_name_t     reply_type);

/* Copyin a capability from a space */
extern kern_return_t ipc_right_copyin(
	ipc_space_t               space,
	mach_port_name_t          name,
	ipc_entry_t               entry,
	mach_msg_type_name_t      msgt_name,
	ipc_right_copyin_flags_t  flags,
	ipc_object_t              *objectp,
	ipc_port_t                *sorightp,
	ipc_port_t                *releasep,
	int                       *assertcntp,
	mach_port_context_t       context,
	mach_msg_guard_flags_t    *guard_flags);

/* Copyin a pair of dispositions from a space */
extern kern_return_t ipc_right_copyin_two(
	ipc_space_t               space,
	mach_port_name_t          name,
	ipc_entry_t               entry,
	mach_msg_type_name_t      msgt_one,
	mach_msg_type_name_t      msgt_two,
	ipc_object_t              *objectp,
	ipc_port_t                *sorightp,
	ipc_port_t                *releasep);

/* Copyout a capability to a space */
extern kern_return_t ipc_right_copyout(
	ipc_space_t             space,
	mach_port_name_t        name,
	ipc_entry_t             entry,
	mach_msg_type_name_t    msgt_name,
	mach_port_context_t     *context,
	mach_msg_guard_flags_t  *guard_flags,
	ipc_object_t            object);

#endif  /* _IPC_IPC_RIGHT_H_ */
