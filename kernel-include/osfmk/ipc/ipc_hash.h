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
 *	File:	ipc/ipc_hash.h
 *	Author:	Rich Draves
 *	Date:	1989
 *
 *	Declarations of entry hash table operations.
 */

#ifndef _IPC_IPC_HASH_H_
#define _IPC_IPC_HASH_H_

#include <mach/port.h>
#include <mach/mach_types.h>
#include <mach/boolean.h>
#include <mach/kern_return.h>
#include <ipc/ipc_types.h>
/*
 * Exported interfaces
 */

/* Lookup (space, obj) in the appropriate reverse hash table */
extern boolean_t ipc_hash_lookup(
	ipc_space_t             space,
	ipc_object_t            obj,
	mach_port_name_t        *namep,
	ipc_entry_t             *entryp);

/* Insert an entry into the appropriate reverse hash table */
extern void ipc_hash_insert(
	ipc_space_t             space,
	ipc_object_t            obj,
	mach_port_name_t        name,
	ipc_entry_t             entry);

/* Delete an entry from the appropriate reverse hash table */
extern void ipc_hash_delete(
	ipc_space_t             space,
	ipc_object_t            obj,
	mach_port_name_t        name,
	ipc_entry_t             entry);

/*
 *	For use by functions that know what they're doing:
 *	local primitives are for table entries.
 */

/* Lookup (space, obj) in local hash table */
extern boolean_t ipc_hash_table_lookup(
	ipc_entry_t             table,
	ipc_entry_num_t         size,
	ipc_object_t            obj,
	mach_port_name_t        *namep,
	ipc_entry_t             *entryp);

/* Inserts an entry into the local reverse hash table */
extern void ipc_hash_table_insert(
	ipc_entry_t             table,
	ipc_entry_num_t         size,
	ipc_object_t            obj,
	mach_port_index_t       index,
	ipc_entry_t             entry);

/* Delete an entry from the appropriate reverse hash table */
extern void ipc_hash_table_delete(
	ipc_entry_t             table,
	ipc_entry_num_t         size,
	ipc_object_t            obj,
	mach_port_name_t        name,
	ipc_entry_t             entry);

#include <mach_ipc_debug.h>

#if     MACH_IPC_DEBUG

#include <mach_debug/hash_info.h>

extern natural_t ipc_hash_info(
	hash_info_bucket_t      *info,
	natural_t count);

#endif  /* MACH_IPC_DEBUG */

#endif  /* _IPC_IPC_HASH_H_ */
