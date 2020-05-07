/*
 * Copyright (c) 2000-2006 Apple Computer, Inc. All rights reserved.
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
 * NOTICE: This file was modified by SPARTA, Inc. in 2005 to introduce
 * support for mandatory and extensible security protections.  This notice
 * is included in support of clause 2.2 (b) of the Apple Public License,
 * Version 2.0.
 */
/*
 */
/*
 *	File:	kern/ipc_kobject.h
 *	Author:	Rich Draves
 *	Date:	1989
 *
 *	Declarations for letting a port represent a kernel object.
 */

#ifdef MACH_KERNEL_PRIVATE
#include <ipc/ipc_kmsg.h>
#include <ipc/ipc_port.h>
#endif /* MACH_KERNEL_PRIVATE */

#ifndef _KERN_IPC_KOBJECT_H_
#define _KERN_IPC_KOBJECT_H_

#ifdef KERNEL_PRIVATE
/*
 * This is the legacy in-kernel ipc-object mechanism.  Over the next
 * several months, this will be phased out in favor of a mechanism that
 * is less Mach IPC specific, and common across in-mach, in-kernel-component,
 * and user-level-component (Plugin) models.
 */
#include <mach/machine/vm_types.h>
#include <mach/mach_types.h>

typedef natural_t       ipc_kobject_type_t;

#define IKOT_NONE                               0
#define IKOT_THREAD                             1
#define IKOT_TASK                               2
#define IKOT_HOST                               3
#define IKOT_HOST_PRIV                  4
#define IKOT_PROCESSOR                  5
#define IKOT_PSET                               6
#define IKOT_PSET_NAME                  7
#define IKOT_TIMER                              8
#define IKOT_PAGING_REQUEST             9
#define IKOT_MIG                                10
#define IKOT_MEMORY_OBJECT              11
#define IKOT_XMM_PAGER                  12
#define IKOT_XMM_KERNEL                 13
#define IKOT_XMM_REPLY                  14
#define IKOT_UND_REPLY                  15
#define IKOT_HOST_NOTIFY                16
#define IKOT_HOST_SECURITY              17
#define IKOT_LEDGER                             18
#define IKOT_MASTER_DEVICE              19
#define IKOT_TASK_NAME                  20
#define IKOT_SUBSYSTEM                  21
#define IKOT_IO_DONE_QUEUE              22
#define IKOT_SEMAPHORE                  23
#define IKOT_LOCK_SET                   24
#define IKOT_CLOCK                              25
#define IKOT_CLOCK_CTRL                 26
#define IKOT_IOKIT_IDENT                27
#define IKOT_NAMED_ENTRY                28
#define IKOT_IOKIT_CONNECT              29
#define IKOT_IOKIT_OBJECT               30
#define IKOT_UPL                                31
#define IKOT_MEM_OBJ_CONTROL            32
#define IKOT_AU_SESSIONPORT             33
#define IKOT_FILEPORT                   34
#define IKOT_LABELH                     35
#define IKOT_TASK_RESUME                36
#define IKOT_VOUCHER                    37
#define IKOT_VOUCHER_ATTR_CONTROL       38
#define IKOT_WORK_INTERVAL              39
#define IKOT_UX_HANDLER                 40
#define IKOT_UEXT_OBJECT                41
#define IKOT_ARCADE_REG                 42

/*
 * Add new entries here and adjust IKOT_UNKNOWN.
 * Please keep ipc/ipc_object.c:ikot_print_array up to date.
 */
#define IKOT_UNKNOWN                    43      /* magic catchall       */
#define IKOT_MAX_TYPE   (IKOT_UNKNOWN+1)        /* # of IKOT_ types	*/

#ifdef MACH_KERNEL_PRIVATE

/*
 *	Define types of kernel objects that use page lists instead
 *	of entry lists for copyin of out of line memory.
 */

/* Dispatch a kernel server function */
extern ipc_kmsg_t ipc_kobject_server(
	ipc_kmsg_t                  request,
	mach_msg_option_t           option);

/* Make a port represent a kernel object of the given type */
extern void ipc_kobject_set(
	ipc_port_t                  port,
	ipc_kobject_t               kobject,
	ipc_kobject_type_t          type);

extern void ipc_kobject_set_atomically(
	ipc_port_t                  port,
	ipc_kobject_t               kobject,
	ipc_kobject_type_t          type);

__options_decl(ipc_kobject_alloc_options_t, uint32_t, {
	/* Just make the naked port */
	IPC_KOBJECT_ALLOC_NONE      = 0x00000000,
	/* Make a send right */
	IPC_KOBJECT_ALLOC_MAKE_SEND = 0x00000001,
	/* Register for no-more-senders */
	IPC_KOBJECT_ALLOC_NSREQUEST = 0x00000002,
	/* Make it no grant port */
	IPC_KOBJECT_ALLOC_NO_GRANT  = 0x00000004,
	/* Make all the send rights immovable */
	IPC_KOBJECT_ALLOC_IMMOVABLE_SEND = 0x00000008,
});

/* Allocates a kobject port, never fails */
extern ipc_port_t ipc_kobject_alloc_port(
	ipc_kobject_t               kobject,
	ipc_kobject_type_t          type,
	ipc_kobject_alloc_options_t options);

/* Makes a send right, lazily allocating a kobject port, arming for no-senders, never fails */
extern boolean_t ipc_kobject_make_send_lazy_alloc_port(
	ipc_port_t                 *port_store,
	ipc_kobject_t               kobject,
	ipc_kobject_type_t          type) __result_use_check;


/* Release any kernel object resources associated with a port */
extern void ipc_kobject_destroy(
	ipc_port_t                  port);

#define null_conversion(port)   (port)

extern kern_return_t
uext_server(ipc_kmsg_t request, ipc_kmsg_t * reply);

#endif /* MACH_KERNEL_PRIVATE */

#endif /* KERNEL_PRIVATE */

#endif /* _KERN_IPC_KOBJECT_H_ */
