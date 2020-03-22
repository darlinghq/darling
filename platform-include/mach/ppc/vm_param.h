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

#ifndef	_MACH_PPC_VM_PARAM_H_
#define _MACH_PPC_VM_PARAM_H_

/*
 * These are the global definitions
 */

#define BYTE_SIZE		8		/* byte size in bits */

#define PPC_PGBYTES		4096	/* bytes per ppc page */
#define PPC_PGSHIFT		12		/* number of bits to shift for pages */

#define	PAGE_SIZE		PPC_PGBYTES
#define	PAGE_SHIFT		PPC_PGSHIFT
#define PAGE_MASK		(PAGE_SIZE - 1)

#if 0
#define VM_MAX_PAGE_ADDRESS 0xFFFFFFFFFFFFF000ULL
#else
/* 
 * LP64todo - For now, we are limited to 51-bits of user addressing
 */
#define VM_MAX_PAGE_ADDRESS 0x0007FFFFFFFFF000ULL
#endif

#define MACH_VM_MIN_ADDRESS	((mach_vm_offset_t) 0)
#define MACH_VM_MAX_ADDRESS	((mach_vm_offset_t) VM_MAX_PAGE_ADDRESS)

/*
 * These are the values relative to the local process.
 */
#if defined (__ppc64__)
/*
 * LP64todo - We don't have the 64-bit address space layout yet.
 * Use the 32-bit stack layout for now.
 */
#define VM_MIN_ADDRESS	((vm_offset_t) MACH_VM_MIN_ADDRESS)
#define VM_MAX_ADDRESS	((vm_offset_t) MACH_VM_MAX_ADDRESS)
#define USER_STACK_END  	((vm_offset_t) 0x00000000ffff0000ULL)
#else
#define VM_MIN_ADDRESS	((vm_offset_t) 0)
#define VM_MAX_ADDRESS	((vm_offset_t) (VM_MAX_PAGE_ADDRESS & 0xFFFFFFFF))
#define USER_STACK_END  	((vm_offset_t) 0xffff0000U)
#endif /* defined(__ppc64__) */


#endif	/* _MACH_PPC_VM_PARAM_H_ */
