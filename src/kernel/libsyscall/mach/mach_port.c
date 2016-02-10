/*
 * Copyright (c) 2011 Apple Inc. All rights reserved.
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

#include <mach/mach_port_internal.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <mach/mach_traps.h>

kern_return_t
mach_port_names(
	ipc_space_t task,
	mach_port_name_array_t *names,
	mach_msg_type_number_t *namesCnt,
	mach_port_type_array_t *types,
	mach_msg_type_number_t *typesCnt)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_names(task, names, namesCnt, types,
			typesCnt);

	return (rv);
}

kern_return_t
mach_port_type(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_type_t *ptype)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_type(task, name, ptype);

	return (rv);
}

kern_return_t
mach_port_rename(
	ipc_space_t task,
	mach_port_name_t old_name,
	mach_port_name_t new_name)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_rename(task, old_name, new_name);

	return (rv);
}

kern_return_t
mach_port_allocate_name(
	ipc_space_t task,
	mach_port_right_t right,
	mach_port_name_t name)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_allocate_name(task, right, name);

	return (rv);
}

kern_return_t
mach_port_allocate(
	ipc_space_t task,
	mach_port_right_t right,
	mach_port_name_t *name)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_allocate_trap(task, right, name);

	if (rv == MACH_SEND_INVALID_DEST)
		rv = _kernelrpc_mach_port_allocate(task, right, name);

	return (rv);
}

kern_return_t
mach_port_destroy(
	ipc_space_t task,
	mach_port_name_t name)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_destroy_trap(task, name);

	if (rv == MACH_SEND_INVALID_DEST)
		rv = _kernelrpc_mach_port_destroy(task, name);

	return (rv);
}

kern_return_t
mach_port_deallocate(
	ipc_space_t task,
	mach_port_name_t name)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_deallocate_trap(task, name); 

	if (rv == MACH_SEND_INVALID_DEST)
		rv = _kernelrpc_mach_port_deallocate(task,name);

	return (rv);
}

kern_return_t
mach_port_get_refs(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_right_t right,
	mach_port_urefs_t *refs)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_get_refs(task, name, right, refs);

	return (rv);
}

kern_return_t
mach_port_mod_refs(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_right_t right,
	mach_port_delta_t delta)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_mod_refs_trap(task, name, right, delta); 

	if (rv == MACH_SEND_INVALID_DEST)
		rv = _kernelrpc_mach_port_mod_refs(task, name, right, delta);

	return (rv);
}

kern_return_t
mach_port_peek(
	ipc_space_t		task,
	mach_port_name_t	name,
	mach_msg_trailer_type_t trailer_type,
	mach_port_seqno_t	*seqnop,
	mach_msg_size_t		*msg_sizep,
	mach_msg_id_t		*msg_idp,
	mach_msg_trailer_info_t trailer_infop,
	mach_msg_type_number_t	*trailer_sizep)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_peek(task, name, trailer_type, 
				       seqnop, msg_sizep, msg_idp,
				       trailer_infop, trailer_sizep);

	return (rv);
}

kern_return_t
mach_port_set_mscount(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_mscount_t mscount)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_set_mscount(task, name, mscount);

	return (rv);
}

kern_return_t
mach_port_get_set_status(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_name_array_t *members,
	mach_msg_type_number_t *membersCnt)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_get_set_status(task, name, members,
			membersCnt);

	return (rv);
}

kern_return_t
mach_port_move_member(
	ipc_space_t task,
	mach_port_name_t member,
	mach_port_name_t after)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_move_member_trap(task, member, after);

	if (rv == MACH_SEND_INVALID_DEST)
		rv = _kernelrpc_mach_port_move_member(task, member, after);

	return (rv);
}

kern_return_t
mach_port_request_notification(
	ipc_space_t task,
	mach_port_name_t name,
	mach_msg_id_t msgid,
	mach_port_mscount_t sync,
	mach_port_t notify,
	mach_msg_type_name_t notifyPoly,
	mach_port_t *previous)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_request_notification(task, name, msgid,
		sync, notify, notifyPoly, previous);

	return (rv);
}

kern_return_t
mach_port_insert_right(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_t poly,
	mach_msg_type_name_t polyPoly)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_insert_right_trap(task, name, poly, polyPoly); 

	if (rv == MACH_SEND_INVALID_DEST)
		rv = _kernelrpc_mach_port_insert_right(task, name, poly,
		    polyPoly);

	return (rv);
}

kern_return_t
mach_port_extract_right(
	ipc_space_t task,
	mach_port_name_t name,
	mach_msg_type_name_t msgt_name,
	mach_port_t *poly,
	mach_msg_type_name_t *polyPoly)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_extract_right(task, name, msgt_name,
		poly, polyPoly);

	return (rv);
}

kern_return_t
mach_port_set_seqno(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_seqno_t seqno)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_set_seqno(task, name, seqno);

	return (rv);
}

kern_return_t
mach_port_get_attributes(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_flavor_t flavor,
	mach_port_info_t port_info_out,
	mach_msg_type_number_t *port_info_outCnt)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_get_attributes(task, name, flavor,
			port_info_out, port_info_outCnt);

	return (rv);
}

kern_return_t
mach_port_set_attributes(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_flavor_t flavor,
	mach_port_info_t port_info,
	mach_msg_type_number_t port_infoCnt)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_set_attributes(task, name, flavor,
			port_info, port_infoCnt);

	return (rv);
}

kern_return_t
mach_port_allocate_qos(
	ipc_space_t task,
	mach_port_right_t right,
	mach_port_qos_t *qos,
	mach_port_name_t *name)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_allocate_qos(task, right, qos, name);

	return (rv);
}

kern_return_t
mach_port_allocate_full(
	ipc_space_t task,
	mach_port_right_t right,
	mach_port_t proto,
	mach_port_qos_t *qos,
	mach_port_name_t *name)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_allocate_full(task, right, proto, qos, name);

	return (rv);
}

kern_return_t
task_set_port_space(
	ipc_space_t task,
	int table_entries)
{
	kern_return_t rv;

	rv = _kernelrpc_task_set_port_space(task, table_entries);

	return (rv);
}

kern_return_t
mach_port_get_srights(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_rights_t *srights)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_get_srights(task, name, srights);

	return (rv);
}

kern_return_t
mach_port_space_info(
	ipc_space_t task,
	ipc_info_space_t *space_info,
	ipc_info_name_array_t *table_info,
	mach_msg_type_number_t *table_infoCnt,
	ipc_info_tree_name_array_t *tree_info,
	mach_msg_type_number_t *tree_infoCnt)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_space_info(task, space_info, table_info,
			table_infoCnt, tree_info, tree_infoCnt);

	return (rv);
}

kern_return_t
mach_port_space_basic_info(
	ipc_space_t task,
	ipc_info_space_basic_t *space_basic_info)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_space_basic_info(task, space_basic_info);

	return (rv);
}

kern_return_t
mach_port_dnrequest_info(
	ipc_space_t task,
	mach_port_name_t name,
	unsigned *dnr_total,
	unsigned *dnr_used)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_dnrequest_info(task, name, dnr_total,
			dnr_used);

	return (rv);
}

kern_return_t
mach_port_kernel_object(
	ipc_space_t task,
	mach_port_name_t name,
	unsigned *object_type,
	unsigned *object_addr)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_kernel_object(task, name,
			object_type, object_addr);

	return (rv);
}

kern_return_t
mach_port_insert_member(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_name_t pset)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_insert_member_trap(task, name, pset);

	if (rv == MACH_SEND_INVALID_DEST)
		rv = _kernelrpc_mach_port_insert_member(task, name, pset);

	return (rv);
}

kern_return_t
mach_port_extract_member(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_name_t pset)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_extract_member_trap(task, name, pset); 

	if (rv == MACH_SEND_INVALID_DEST)
		rv = _kernelrpc_mach_port_extract_member(task, name, pset);

	return (rv);
}

kern_return_t
mach_port_get_context(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_context_t *context)
{
	kern_return_t rv;
	mach_vm_address_t wide_context;

	rv = _kernelrpc_mach_port_get_context(task, name, &wide_context);

	if (rv == KERN_SUCCESS) {
		*context = (mach_port_context_t)wide_context;
	}

	return (rv);
}

kern_return_t
mach_port_set_context(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_context_t context)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_set_context(task, name, context);

	return (rv);
}

kern_return_t
mach_port_kobject(
	ipc_space_t task,
	mach_port_name_t name,
	natural_t *object_type,
	mach_vm_address_t *object_addr)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_kobject(task, name, object_type, object_addr);

	return (rv);
}

kern_return_t
mach_port_construct(
	ipc_space_t		task,
	mach_port_options_t	*options,
	mach_port_context_t	context,
	mach_port_name_t	*name)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_construct_trap(task, options, (uint64_t) context, name);

	if (rv == MACH_SEND_INVALID_DEST)
		rv = _kernelrpc_mach_port_construct(task, options, (uint64_t) context, name);

	return (rv);
}

kern_return_t
mach_port_destruct(
	ipc_space_t		task,
	mach_port_name_t	name,
	mach_port_delta_t	srdelta,
	mach_port_context_t	guard)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_destruct_trap(task, name, srdelta, (uint64_t) guard);

	if (rv == MACH_SEND_INVALID_DEST)
		rv = _kernelrpc_mach_port_destruct(task, name, srdelta, (uint64_t) guard);

	return (rv);

}

kern_return_t
mach_port_guard(
	ipc_space_t		task,
	mach_port_name_t	name,
	mach_port_context_t	guard,
	boolean_t		strict)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_guard_trap(task, name, (uint64_t) guard, strict);

	if (rv == MACH_SEND_INVALID_DEST)
		rv = _kernelrpc_mach_port_guard(task, name, (uint64_t) guard, strict);

	return (rv);

}

kern_return_t
mach_port_unguard(
	ipc_space_t		task,
	mach_port_name_t	name,
	mach_port_context_t	guard)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_port_unguard_trap(task, name, (uint64_t) guard);

	if (rv == MACH_SEND_INVALID_DEST)
		rv = _kernelrpc_mach_port_unguard(task, name, (uint64_t) guard);

	return (rv);

}

