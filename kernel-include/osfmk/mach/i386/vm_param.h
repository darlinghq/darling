/*
 * Copyright (c) 2000-2012 Apple Inc. All rights reserved.
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
 * Copyright (c) 1991,1990,1989,1988 Carnegie Mellon University
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
 * Copyright (c) 1994 The University of Utah and
 * the Computer Systems Laboratory at the University of Utah (CSL).
 * All rights reserved.
 *
 * Permission to use, copy, modify and distribute this software is hereby
 * granted provided that (1) source code retains these copyright, permission,
 * and disclaimer notices, and (2) redistributions including binaries
 * reproduce the notices in supporting documentation, and (3) all advertising
 * materials mentioning features or use of this software display the following
 * acknowledgement: ``This product includes software developed by the
 * Computer Systems Laboratory at the University of Utah.''
 *
 * THE UNIVERSITY OF UTAH AND CSL ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS
 * IS" CONDITION.  THE UNIVERSITY OF UTAH AND CSL DISCLAIM ANY LIABILITY OF
 * ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * CSL requests users of this software to return to csl-dist@cs.utah.edu any
 * improvements that they make and grant CSL redistribution rights.
 *
 */

/*
 *	File:	vm_param.h
 *	Author:	Avadis Tevanian, Jr.
 *	Date:	1985
 *
 *	I386 machine dependent virtual memory parameters.
 *	Most of the declarations are preceeded by I386_ (or i386_)
 *	which is OK because only I386 specific code will be using
 *	them.
 */

#ifndef _MACH_I386_VM_PARAM_H_
#define _MACH_I386_VM_PARAM_H_

#define BYTE_SIZE               8               /* byte size in bits */

#define I386_PGBYTES            4096            /* bytes per 80386 page */
#define I386_PGSHIFT            12              /* bitshift for pages */

#define PAGE_SIZE               I386_PGBYTES
#define PAGE_SHIFT              I386_PGSHIFT
#define PAGE_MASK               (PAGE_SIZE - 1)

#define PAGE_MAX_SHIFT          PAGE_SHIFT
#define PAGE_MAX_SIZE           PAGE_SIZE
#define PAGE_MAX_MASK           PAGE_MASK

#define PAGE_MIN_SHIFT          PAGE_SHIFT
#define PAGE_MIN_SIZE           PAGE_SIZE
#define PAGE_MIN_MASK           PAGE_MASK

#define I386_LPGBYTES           2*1024*1024     /* bytes per large page */
#define I386_LPGSHIFT           21              /* bitshift for large pages */
#define I386_LPGMASK            (I386_LPGBYTES-1)

/*
 *	Convert bytes to pages and convert pages to bytes.
 *	No rounding is used.
 */

#define i386_btop(x)            ((ppnum_t)((x) >> I386_PGSHIFT))
#define machine_btop(x)         i386_btop(x)
#define i386_ptob(x)            (((pmap_paddr_t)(x)) << I386_PGSHIFT)
#define machine_ptob(x)         i386_ptob(x)

/*
 *	Round off or truncate to the nearest page.  These will work
 *	for either addresses or counts.  (i.e. 1 byte rounds to 1 page
 *	bytes.
 */

#define i386_round_page(x)      ((((pmap_paddr_t)(x)) + I386_PGBYTES - 1) & \
	                                ~(I386_PGBYTES-1))
#define i386_trunc_page(x)      (((pmap_paddr_t)(x)) & ~(I386_PGBYTES-1))



#define VM_MIN_ADDRESS64        ((user_addr_t) 0x0000000000000000ULL)
/*
 * default top of user stack... it grows down from here
 */
#define VM_USRSTACK64           ((user_addr_t) 0x00007FFEEFC00000ULL)

/*
 * XXX TODO: Obsolete?
 */
#define VM_DYLD64               ((user_addr_t) 0x00007FFF5FC00000ULL)
#define VM_LIB64_SHR_DATA       ((user_addr_t) 0x00007FFF60000000ULL)
#define VM_LIB64_SHR_TEXT       ((user_addr_t) 0x00007FFF80000000ULL)
/*
 * the end of the usable user address space , for now about 47 bits.
 * the 64 bit commpage is past the end of this
 */
#define VM_MAX_PAGE_ADDRESS     ((user_addr_t) 0x00007FFFFFE00000ULL)
/*
 * canonical end of user address space for limits checking
 */
#define VM_MAX_USER_PAGE_ADDRESS ((user_addr_t)0x00007FFFFFFFF000ULL)


/* system-wide values */
#define MACH_VM_MIN_ADDRESS             ((mach_vm_offset_t) 0)
#define MACH_VM_MAX_ADDRESS             ((mach_vm_offset_t) VM_MAX_PAGE_ADDRESS)

/* process-relative values (all 32-bit legacy only for now) */
#define VM_MIN_ADDRESS          ((vm_offset_t) 0)
#define VM_USRSTACK32           ((vm_offset_t) 0xC0000000)      /* ASLR slides stack down by up to 1 MB */
#define VM_MAX_ADDRESS          ((vm_offset_t) 0xFFE00000)


#ifdef  KERNEL_PRIVATE

#define TEST_PAGE_SIZE_16K      FALSE
#define TEST_PAGE_SIZE_4K       TRUE

/* Kernel-wide values */
#define KB              (1024ULL)
#define MB              (1024*KB)
#define GB              (1024*MB)

/*
 * Maximum physical memory supported.
 */
#define K32_MAXMEM      (32*GB)
#define K64_MAXMEM      (1536*GB)
#define KERNEL_MAXMEM   K64_MAXMEM

/*
 * XXX
 * The kernel max VM address is limited to 0xFF3FFFFF for now because
 * some data structures are explicitly allocated at 0xFF400000 without
 * VM's knowledge (see osfmk/i386/locore.s for the allocation of PTmap and co.).
 * We can't let VM allocate memory from there.
 */


#define KERNEL_IMAGE_TO_PHYS(x) (x)
#define VM_KERNEL_POINTER_SIGNIFICANT_BITS 39
#define VM_MIN_KERNEL_ADDRESS           ((vm_offset_t) 0xFFFFFF8000000000UL)
#define VM_MIN_KERNEL_PAGE              ((ppnum_t)0)
#define VM_MIN_KERNEL_AND_KEXT_ADDRESS  (VM_MIN_KERNEL_ADDRESS - 0x80000000ULL)
#define VM_MAX_KERNEL_ADDRESS           ((vm_offset_t) 0xFFFFFFFFFFFFEFFFUL)
#define VM_MAX_KERNEL_ADDRESS_EFI32     ((vm_offset_t) 0xFFFFFF80FFFFEFFFUL)
#define KEXT_ALLOC_MAX_OFFSET (2 * 1024 * 1024 * 1024UL)
#define KEXT_ALLOC_BASE(x)  ((x) - KEXT_ALLOC_MAX_OFFSET)
#define KEXT_ALLOC_SIZE(x)  (KEXT_ALLOC_MAX_OFFSET - (x))

#define VM_KERNEL_STRIP_PTR(_v) (_v)

#define VM_KERNEL_ADDRESS(va)   ((((vm_address_t)(va))>=VM_MIN_KERNEL_AND_KEXT_ADDRESS) && \
	                        (((vm_address_t)(va))<=VM_MAX_KERNEL_ADDRESS))

#define VM_MAP_MIN_ADDRESS      MACH_VM_MIN_ADDRESS
#define VM_MAP_MAX_ADDRESS      MACH_VM_MAX_ADDRESS

/* FIXME  - always leave like this? */
#if KASAN
/* Increase the stack sizes to account for the redzones that get added to every
 * stack object. */
# define INTSTACK_SIZE (I386_PGBYTES*4*4)
# define KERNEL_STACK_SIZE (I386_PGBYTES*4*4)
#elif DEBUG
# define INTSTACK_SIZE (I386_PGBYTES*4)
# define KERNEL_STACK_SIZE (I386_PGBYTES*6)
#else
# define INTSTACK_SIZE (I386_PGBYTES*4)
# define KERNEL_STACK_SIZE (I386_PGBYTES*4)
#endif

#ifdef  MACH_KERNEL_PRIVATE

/* For implementing legacy 32-bit interfaces */
#define VM32_SUPPORT                    1
#define VM32_MIN_ADDRESS                ((vm32_offset_t) 0)
#define VM32_MAX_ADDRESS                ((vm32_offset_t) (VM_MAX_PAGE_ADDRESS & 0xFFFFFFFF))

/*
 * kalloc() parameters:
 *
 * Historically kalloc's underlying zones were power-of-2 sizes, with a
 * KALLOC_MINSIZE of 16 bytes.  The allocator ensured that
 * (sizeof == alignof) >= 16 for all kalloc allocations.
 *
 * Today kalloc may use zones with intermediate sizes, constrained by
 * KALLOC_MINSIZE and a minimum alignment, expressed by KALLOC_LOG2_MINALIGN.
 *
 * The common alignment for LP64 is for longs and pointers i.e. 8 bytes.
 */


#define KALLOC_MINSIZE          16      /* minimum allocation size */
#define KALLOC_LOG2_MINALIGN    4       /* log2 minimum alignment */

#define LINEAR_KERNEL_ADDRESS   ((vm_offset_t) 0x00000000)

#define VM_MIN_KERNEL_LOADED_ADDRESS    ((vm_offset_t) 0xFFFFFF8000000000UL)
#define VM_MAX_KERNEL_LOADED_ADDRESS    ((vm_offset_t) 0xFFFFFF801FFFFFFFUL)

#define NCOPY_WINDOWS 0



/*
 *	Conversion between 80386 pages and VM pages
 */

#define trunc_i386_to_vm(p)     (atop(trunc_page(i386_ptob(p))))
#define round_i386_to_vm(p)     (atop(round_page(i386_ptob(p))))
#define vm_to_i386(p)           (i386_btop(ptoa(p)))


#define PMAP_SET_CACHE_ATTR(mem, object, cache_attr, batch_pmap_op)     \
	MACRO_BEGIN                                                     \
	        pmap_set_cache_attributes(VM_PAGE_GET_PHYS_PAGE(mem), (cache_attr));    \
	        (object)->set_cache_attr = TRUE;                                \
	        (void) batch_pmap_op;                                   \
	MACRO_END

#define PMAP_BATCH_SET_CACHE_ATTR(object, user_page_list, cache_attr, num_pages, batch_pmap_op) \
	MACRO_BEGIN                                                     \
	(void) user_page_list;                                          \
	(void) num_pages;                                               \
	(void) batch_pmap_op;                                           \
	MACRO_END

#define IS_USERADDR64_CANONICAL(addr)                   \
	((addr) < (VM_MAX_USER_PAGE_ADDRESS))

#endif  /* MACH_KERNEL_PRIVATE */

#endif  /* KERNEL_PRIVATE */

#endif  /* _MACH_I386_VM_PARAM_H_ */
