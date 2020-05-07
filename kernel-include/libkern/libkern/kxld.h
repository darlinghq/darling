/*
 * Copyright (c) 2007-2008 Apple Inc. All rights reserved.
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
#ifndef _KXLD_H
#define _KXLD_H

#include <sys/types.h>
#include <mach/boolean.h>       // boolean_t
#include <mach/kern_return.h>   // kern_return_t
#include <mach/machine.h>       // cpu_type_t and cpu_subtype_t
#include <mach/vm_types.h>

#include "kxld_types.h"

/*******************************************************************************
* API
*******************************************************************************/

/*******************************************************************************
* Creates a state object for the linker.  A context must be created for each
* link thread and destroyed at the end of the thread's life.  A context should
* be reused for all links occuring in that link thread.
*   context             Returns a pointer to the new context object
*   allocate callback   Callback to allocate memory for the linked kext
*   log_callback        Callback for all kxld logging output
*   flags               Flags to control the behavior of kxld
*   cputype             The target arch's CPU type (0 for host arch)
*   cpusubtype          The target arch's CPU subtype (0 for host subtype)
*   pagesize            The target page size (0 for host page size)
*******************************************************************************/
kern_return_t kxld_create_context(
	KXLDContext **context,
	KXLDAllocateCallback allocate_callback,
	KXLDLoggingCallback log_callback,
	KXLDFlags flags,
	cpu_type_t cputype,
	cpu_subtype_t cpusubtype,
	vm_size_t pagesize)
__attribute__((nonnull(1), visibility("default")));

/*******************************************************************************
* Destroys a link context and frees all associated memory.  Should be called at
* the end of a link thread's life.
*******************************************************************************/
void kxld_destroy_context(
	KXLDContext *context)
__attribute__((nonnull, visibility("default")));

/*******************************************************************************
 * Links a kext against its dependencies, using a callback to allocate the memory
 * at which it will be located.
 * NOTE: The object data itself must be mmapped with PROT_WRITE and MAP_PRIVATE
 *   context             The KXLDContext object for the current link thread.
 *   file                The kext object file read into memory.
 *                       Supported formats: Mach-O, Mach-O64, Fat.
 *   size                The size of the kext in memory.  Must be nonzero.
 *   name                The name, usually the bundle identifier, of the kext
 *   callback_data       Data that is to be passed to the callback functions.
 *   dependencies        An array of pointers to the kexts upon which this kext
 *                       is dependent.
 *   num_dependencies    Number of entries in the 'dependencies' array.
 *   linked_object       This will be set to the address of the linked kext
 *                       object. If the address provided by the
 *                       kxld_alloc_callback is considered writable, this
 *                       pointer will be set to that address.  Otherwise, the
 *                       linked object will be written to a temporary buffer
 *                       that should be freed by the caller.
 *   kmod_info_kern      Kernel address of the kmod_info_t structure.
 ******************************************************************************/
kern_return_t kxld_link_file(
	KXLDContext *context,
	u_char *file,
	u_long size,
	const char *name,
	void *callback_data,
	KXLDDependency *dependencies,
	u_int num_dependencies,
	u_char **linked_object,
	kxld_addr_t *kmod_info_kern)
__attribute__((nonnull(1, 2, 4, 6, 8, 9), visibility("default")));


kern_return_t kxld_link_split_file(
	KXLDContext *context,
	splitKextLinkInfo *link_info,
	const char *name,
	void *callback_data,
	KXLDDependency *dependencies,
	u_int num_dependencies,
	kxld_addr_t *kmod_info_kern)
__attribute__((nonnull(1, 2, 3, 5, 7), visibility("default")));


/*******************************************************************************
*******************************************************************************/
boolean_t kxld_validate_copyright_string(const char *str)
__attribute__((pure, nonnull, visibility("default")));

#endif // _KXLD_H_
