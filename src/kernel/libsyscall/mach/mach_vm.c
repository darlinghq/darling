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

/*
 * Make sure we don't accidentally include the external definitions of
 * the routines we're interposing on below.
 */
#define _vm_map_user_
#define _mach_vm_user_
#include <mach/mach.h>
#include <mach/mach_traps.h>
#undef _vm_map_user_
#include <mach/vm_map_internal.h>
#undef _mach_vm_user_
#include <mach/mach_vm_internal.h>

#include "stack_logging_internal.h"

malloc_logger_t *__syscall_logger = NULL;   // This may get set by Libc's malloc stack logging initialization code.

kern_return_t
mach_vm_allocate(
	mach_port_name_t target,
	mach_vm_address_t *address,
	mach_vm_size_t size,
	int flags)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_vm_allocate_trap(target, address, size, flags);

	if (rv == MACH_SEND_INVALID_DEST) {
		rv = _kernelrpc_mach_vm_allocate(target, address, size, flags);
	}

	if (__syscall_logger && rv == KERN_SUCCESS && !(flags & VM_MAKE_TAG(VM_MEMORY_STACK))) {
		int userTagFlags = flags & VM_FLAGS_ALIAS_MASK;
		__syscall_logger(stack_logging_type_vm_allocate | userTagFlags, (uintptr_t)target, (uintptr_t)size, 0, (uintptr_t)*address, 0);
	}

	return rv;
}

kern_return_t
mach_vm_deallocate(
	mach_port_name_t target,
	mach_vm_address_t address,
	mach_vm_size_t size)
{
	kern_return_t rv;

	if (__syscall_logger) {
		__syscall_logger(stack_logging_type_vm_deallocate, (uintptr_t)target, (uintptr_t)address, (uintptr_t)size, 0, 0);
	}

	rv = _kernelrpc_mach_vm_deallocate_trap(target, address, size);

	if (rv == MACH_SEND_INVALID_DEST) {
		rv = _kernelrpc_mach_vm_deallocate(target, address, size);
	}

	return rv;
}

kern_return_t
mach_vm_protect(
	mach_port_name_t task,
	mach_vm_address_t address,
	mach_vm_size_t size,
	boolean_t set_maximum,
	vm_prot_t new_protection)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_vm_protect_trap(task, address, size, set_maximum,
	    new_protection);

	if (rv == MACH_SEND_INVALID_DEST) {
		rv = _kernelrpc_mach_vm_protect(task, address, size,
		    set_maximum, new_protection);
	}

	return rv;
}

kern_return_t
vm_allocate(
	mach_port_name_t task,
	vm_address_t *address,
	vm_size_t size,
	int flags)
{
	kern_return_t rv;
	mach_vm_address_t mach_addr;

	mach_addr = (mach_vm_address_t)*address;
	rv = mach_vm_allocate(task, &mach_addr, size, flags);
#if defined(__LP64__)
	*address = mach_addr;
#else
	*address = (vm_address_t)(mach_addr & ((vm_address_t)-1));
#endif

	return rv;
}

kern_return_t
vm_deallocate(
	mach_port_name_t task,
	vm_address_t address,
	vm_size_t size)
{
	kern_return_t rv;

	rv = mach_vm_deallocate(task, address, size);

	return rv;
}

kern_return_t
vm_protect(
	mach_port_name_t task,
	vm_address_t address,
	vm_size_t size,
	boolean_t set_maximum,
	vm_prot_t new_protection)
{
	kern_return_t rv;

	rv = mach_vm_protect(task, address, size, set_maximum, new_protection);

	return rv;
}

kern_return_t
mach_vm_map(
	mach_port_name_t target,
	mach_vm_address_t *address,
	mach_vm_size_t size,
	mach_vm_offset_t mask,
	int flags,
	mem_entry_name_port_t object,
	memory_object_offset_t offset,
	boolean_t copy,
	vm_prot_t cur_protection,
	vm_prot_t max_protection,
	vm_inherit_t inheritance)
{
	kern_return_t rv = MACH_SEND_INVALID_DEST;

	if (object == MEMORY_OBJECT_NULL && max_protection == VM_PROT_ALL &&
	    inheritance == VM_INHERIT_DEFAULT) {
		rv = _kernelrpc_mach_vm_map_trap(target, address, size, mask, flags,
		    cur_protection);
	}

	if (rv == MACH_SEND_INVALID_DEST) {
		rv = _kernelrpc_mach_vm_map(target, address, size, mask, flags, object,
		    offset, copy, cur_protection, max_protection, inheritance);
	}

	if (__syscall_logger && rv == KERN_SUCCESS && !(flags & VM_MAKE_TAG(VM_MEMORY_STACK))) {
		int eventTypeFlags = stack_logging_type_vm_allocate | stack_logging_type_mapped_file_or_shared_mem;
		int userTagFlags = flags & VM_FLAGS_ALIAS_MASK;
		__syscall_logger(eventTypeFlags | userTagFlags, (uintptr_t)target, (uintptr_t)size, 0, (uintptr_t)*address, 0);
	}

	return rv;
}

kern_return_t
mach_vm_remap(
	mach_port_name_t target,
	mach_vm_address_t *address,
	mach_vm_size_t size,
	mach_vm_offset_t mask,
	int flags,
	mach_port_name_t src_task,
	mach_vm_address_t src_address,
	boolean_t copy,
	vm_prot_t *cur_protection,
	vm_prot_t *max_protection,
	vm_inherit_t inheritance)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_vm_remap(target, address, size, mask, flags,
	    src_task, src_address, copy, cur_protection, max_protection,
	    inheritance);

	if (__syscall_logger && rv == KERN_SUCCESS) {
		int eventTypeFlags = stack_logging_type_vm_allocate | stack_logging_type_mapped_file_or_shared_mem;
		int userTagFlags = flags & VM_FLAGS_ALIAS_MASK;
		__syscall_logger(eventTypeFlags | userTagFlags, (uintptr_t)target, (uintptr_t)size, 0, (uintptr_t)*address, 0);
	}

	return rv;
}

kern_return_t
mach_vm_read(
	mach_port_name_t target,
	mach_vm_address_t address,
	mach_vm_size_t size,
	vm_offset_t *data,
	mach_msg_type_number_t *dataCnt)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_vm_read(target, address, size, data, dataCnt);

	if (__syscall_logger && rv == KERN_SUCCESS) {
		int eventTypeFlags = stack_logging_type_vm_allocate | stack_logging_type_mapped_file_or_shared_mem;
		// The target argument is the remote task from which data is being read,
		// so pass mach_task_self() as the destination task receiving the allocation.
		__syscall_logger(eventTypeFlags, (uintptr_t)mach_task_self(), (uintptr_t)*dataCnt, 0, *data, 0);
	}

	return rv;
}

kern_return_t
vm_map(
	mach_port_name_t target,
	vm_address_t *address,
	vm_size_t size,
	vm_offset_t mask,
	int flags,
	mem_entry_name_port_t object,
	vm_offset_t offset,
	boolean_t copy,
	vm_prot_t cur_protection,
	vm_prot_t max_protection,
	vm_inherit_t inheritance)
{
	kern_return_t rv;

	rv = _kernelrpc_vm_map(target, address, size, mask, flags, object,
	    offset, copy, cur_protection, max_protection, inheritance);

	if (__syscall_logger && rv == KERN_SUCCESS) {
		int eventTypeFlags = stack_logging_type_vm_allocate | stack_logging_type_mapped_file_or_shared_mem;
		int userTagFlags = flags & VM_FLAGS_ALIAS_MASK;
		__syscall_logger(eventTypeFlags | userTagFlags, (uintptr_t)target, (uintptr_t)size, 0, (uintptr_t)*address, 0);
	}

	return rv;
}

kern_return_t
vm_remap(
	mach_port_name_t target,
	vm_address_t *address,
	vm_size_t size,
	vm_offset_t mask,
	int flags,
	mach_port_name_t src_task,
	vm_address_t src_address,
	boolean_t copy,
	vm_prot_t *cur_protection,
	vm_prot_t *max_protection,
	vm_inherit_t inheritance)
{
	kern_return_t rv;

	rv = _kernelrpc_vm_remap(target, address, size, mask, flags,
	    src_task, src_address, copy, cur_protection, max_protection,
	    inheritance);

	if (__syscall_logger) {
		int eventTypeFlags = stack_logging_type_vm_allocate | stack_logging_type_mapped_file_or_shared_mem;
		int userTagFlags = flags & VM_FLAGS_ALIAS_MASK;
		__syscall_logger(eventTypeFlags | userTagFlags, (uintptr_t)target, (uintptr_t)size, 0, (uintptr_t)*address, 0);
	}

	return rv;
}

kern_return_t
vm_read(
	mach_port_name_t target,
	vm_address_t address,
	vm_size_t size,
	vm_offset_t *data,
	mach_msg_type_number_t *dataCnt)
{
	kern_return_t rv;

	rv = _kernelrpc_vm_read(target, address, size, data, dataCnt);

	if (__syscall_logger && rv == KERN_SUCCESS) {
		int eventTypeFlags = stack_logging_type_vm_allocate | stack_logging_type_mapped_file_or_shared_mem;
		// The target argument is the remote task from which data is being read,
		// so pass mach_task_self() as the destination task receiving the allocation.
		__syscall_logger(eventTypeFlags, (uintptr_t)mach_task_self(), (uintptr_t)*dataCnt, 0, *data, 0);
	}

	return rv;
}

kern_return_t
mach_vm_purgable_control(
	mach_port_name_t        target,
	mach_vm_offset_t        address,
	vm_purgable_t           control,
	int                     *state)
{
	kern_return_t rv;

	rv = _kernelrpc_mach_vm_purgable_control_trap(target, address, control, state);

	if (rv == MACH_SEND_INVALID_DEST) {
		rv = _kernelrpc_mach_vm_purgable_control(target, address, control, state);
	}

	return rv;
}

kern_return_t
vm_purgable_control(
	mach_port_name_t        task,
	vm_offset_t             address,
	vm_purgable_t           control,
	int                     *state)
{
	return mach_vm_purgable_control(task,
	           (mach_vm_offset_t) address,
	           control,
	           state);
}
