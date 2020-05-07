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
 * Copyright (c) 1991 Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS
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
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */
/*
 */

#ifndef _VM_MEMORY_OBJECT_H_
#define _VM_MEMORY_OBJECT_H_

#include <mach_pagemap.h>

#include <mach/boolean.h>
#include <mach/mach_types.h>
#include <mach/memory_object_types.h>
#include <ipc/ipc_types.h>

extern memory_object_default_t memory_manager_default;

__private_extern__
memory_object_default_t memory_manager_default_reference(void);

__private_extern__
kern_return_t           memory_manager_default_check(void);

__private_extern__
void                    memory_manager_default_init(void);

__private_extern__
void                    memory_object_control_bootstrap(void);
__private_extern__
memory_object_control_t memory_object_control_allocate(
	vm_object_t             object);

__private_extern__
void                    memory_object_control_collapse(
	memory_object_control_t control,
	vm_object_t             object);

__private_extern__
vm_object_t             memory_object_control_to_vm_object(
	memory_object_control_t control);
__private_extern__
vm_object_t             memory_object_to_vm_object(
	memory_object_t mem_obj);

extern
mach_port_t             convert_mo_control_to_port(
	memory_object_control_t control);

extern void memory_object_control_disable(
	memory_object_control_t control);

extern
memory_object_control_t convert_port_to_mo_control(
	mach_port_t             port);

extern
mach_port_t             convert_memory_object_to_port(
	memory_object_t         object);

extern
memory_object_t         convert_port_to_memory_object(
	mach_port_t             port);

extern upl_t convert_port_to_upl(
	ipc_port_t      port);

extern ipc_port_t convert_upl_to_port( upl_t );

__private_extern__ void upl_no_senders(ipc_port_t, mach_port_mscount_t);

extern kern_return_t    memory_object_pages_resident(
	memory_object_control_t         control,
	boolean_t                       *               has_pages_resident);

extern kern_return_t    memory_object_signed(
	memory_object_control_t         control,
	boolean_t                       is_signed);

extern boolean_t        memory_object_is_signed(
	memory_object_control_t control);

extern boolean_t        memory_object_is_shared_cache(
	memory_object_control_t         control);

extern void             memory_object_mark_used(
	memory_object_control_t         control);

extern void             memory_object_mark_unused(
	memory_object_control_t         control,
	boolean_t                       rage);

extern void             memory_object_mark_io_tracking(
	memory_object_control_t         control);

extern void             memory_object_mark_trusted(
	memory_object_control_t         control);

#if CONFIG_SECLUDED_MEMORY
extern void             memory_object_mark_eligible_for_secluded(
	memory_object_control_t         control,
	boolean_t                       eligible_for_secluded);
#endif /* CONFIG_SECLUDED_MEMORY */

#endif  /* _VM_MEMORY_OBJECT_H_ */
