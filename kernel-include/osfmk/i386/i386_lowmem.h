/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
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

#ifndef _I386_LOWMEM_H_
#define _I386_LOWMEM_H_


#ifdef __APPLE_API_PRIVATE

/*
 * The kernel better be statically linked at VM_MIN_KERNEL_ADDRESS + 0x100000
 */
#define I386_KERNEL_IMAGE_BASE_PAGE     0x100

/* For K64, only 3 pages are reserved
 * - physical page zero, a gap page, and then real-mode-bootstrap/lowGlo.
 * Note that the kernel virtual address KERNEL_BASE+0x2000 is re-mapped
 * to the low globals and that physical page, 0x2000, is used by the bootstrap.
 */
#define I386_LOWMEM_RESERVED            3

#endif /* __APPLE_API_PRIVATE */

#endif /* !_I386_LOWMEM_H_ */
