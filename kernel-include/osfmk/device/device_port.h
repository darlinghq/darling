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
 *	Author: David B. Golub, Carnegie Mellon University
 *	Date:   8/89
 */

#ifndef _DEVICE_DEVICE_PORT_H_
#define _DEVICE_DEVICE_PORT_H_

#include <device/device_types.h>

/*
 *	Master privileged I/O object for this host
 */
extern mach_port_t      master_device_port;

#define DEVICE_PAGER_GUARDED            0x1
#define DEVICE_PAGER_COHERENT           0x2
#define DEVICE_PAGER_CACHE_INHIB        0x4
#define DEVICE_PAGER_WRITE_THROUGH      0x8
#define DEVICE_PAGER_CONTIGUOUS         0x100
#define DEVICE_PAGER_NOPHYSCACHE        0x200

#ifdef XNU_KERNEL_PRIVATE
#include <vm/memory_types.h>

_Static_assert(((DEVICE_PAGER_CONTIGUOUS | DEVICE_PAGER_NOPHYSCACHE) & VM_WIMG_MASK) == 0,
    "device pager flags overlap WIMG mask");

/* Assert on the backwards-compatible DEVICE_PAGER* values */
_Static_assert(DEVICE_PAGER_GUARDED == VM_MEM_GUARDED, "DEVICE_PAGER_GUARDED != VM_MEM_GUARDED");
_Static_assert(DEVICE_PAGER_COHERENT == VM_MEM_COHERENT, "DEVICE_PAGER_COHERENT != VM_MEM_COHERENT");
_Static_assert(DEVICE_PAGER_CACHE_INHIB == VM_MEM_NOT_CACHEABLE, "DEVICE_PAGER_CACHE_INHIB != VM_MEM_NOT_CACHEABLE");
_Static_assert(DEVICE_PAGER_WRITE_THROUGH == VM_MEM_WRITE_THROUGH, "DEVICE_PAGER_WRITE_THROUGH != VM_MEM_WRITE_THROUGH");

#endif /* KERNEL_PRIVATE */

#endif  /* _DEVICE_DEVICE_PORT_H_ */
