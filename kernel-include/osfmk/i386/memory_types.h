/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
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
#ifndef _I386_MEMORY_TYPES_H_
#define _I386_MEMORY_TYPES_H_

#define VM_WIMG_COPYBACK                  VM_MEM_COHERENT
#define VM_WIMG_COPYBACKLW                VM_WIMG_COPYBACK
#define VM_WIMG_DEFAULT                   VM_MEM_COHERENT
/* ?? intel ?? */
#define VM_WIMG_IO                        (VM_MEM_COHERENT |      \
	                                  VM_MEM_NOT_CACHEABLE | VM_MEM_GUARDED)
#define VM_WIMG_POSTED                    VM_WIMG_IO
#define VM_WIMG_POSTED_REORDERED          VM_WIMG_IO
#define VM_WIMG_POSTED_COMBINED_REORDERED VM_WIMG_IO
#define VM_WIMG_WTHRU                     (VM_MEM_WRITE_THROUGH | VM_MEM_COHERENT | VM_MEM_GUARDED)
/* write combining mode, aka store gather */
#define VM_WIMG_WCOMB                     (VM_MEM_NOT_CACHEABLE | VM_MEM_COHERENT)
#define VM_WIMG_INNERWBACK       VM_MEM_COHERENT
#define VM_WIMG_RT               VM_WIMG_WCOMB

#endif /* _I386_MEMORY_TYPES_H_ */
