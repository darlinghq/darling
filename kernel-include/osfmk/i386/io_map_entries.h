/*
 * Copyright (c) 2000-2005 Apple Computer, Inc. All rights reserved.
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
#ifdef  KERNEL_PRIVATE

#ifndef _I386_IO_MAP_ENTRIES
#define _I386_IO_MAP_ENTRIES

#include <sys/cdefs.h>
#include <sys/appleapiopts.h>

#ifdef  __APPLE_API_PRIVATE
__BEGIN_DECLS
extern vm_offset_t      io_map(
	vm_map_offset_t         phys_addr,
	vm_size_t               size,
	unsigned int            flags);
extern vm_offset_t io_map_spec(vm_map_offset_t phys_addr, vm_size_t size, unsigned int flags);
__END_DECLS
#endif  /* __APPLE_API_PRIVATE */

#endif  /* _I386_IO_MAP_ENTRIES */

#endif  /* KERNEL_PRIVATE */
