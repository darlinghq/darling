/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
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
 * FILE_ID: vm_param.h
 */

/*
 *	ARM machine dependent virtual memory parameters.
 */

#ifndef _MACH_ARM_VM_PARAM_H_
#define _MACH_ARM_VM_PARAM_H_

#if defined(KERNEL_PRIVATE) && __ARM_16K_PG__
#include <arm64/proc_reg.h>
#endif

#if !defined (KERNEL) && !defined (__ASSEMBLER__)
#include <mach/vm_page_size.h>
#endif

#define BYTE_SIZE       8       /* byte size in bits */

#if defined (KERNEL)

#ifndef __ASSEMBLER__

#ifdef  __arm__
#define PAGE_SHIFT_CONST        12
#elif defined(__arm64__)
extern unsigned         PAGE_SHIFT_CONST;
#else
#error Unsupported arch
#endif

#if defined(KERNEL_PRIVATE) && __ARM_16K_PG__
#define PAGE_SHIFT              ARM_PGSHIFT
#else
#define PAGE_SHIFT              PAGE_SHIFT_CONST
#endif
#define PAGE_SIZE               (1 << PAGE_SHIFT)
#define PAGE_MASK               (PAGE_SIZE-1)

#define VM_PAGE_SIZE            PAGE_SIZE

#define machine_ptob(x)         ((x) << PAGE_SHIFT)

/*
 * Defined for the purpose of testing the pmap advertised page
 * size; this does not necessarily match the hardware page size.
 */
#define TEST_PAGE_SIZE_16K      ((PAGE_SHIFT_CONST == 14))
#define TEST_PAGE_SIZE_4K       ((PAGE_SHIFT_CONST == 12))

#endif  /* !__ASSEMBLER__ */

#else

#define PAGE_SHIFT                      vm_page_shift
#define PAGE_SIZE                       vm_page_size
#define PAGE_MASK                       vm_page_mask

#define VM_PAGE_SIZE            vm_page_size

#define machine_ptob(x)         ((x) << PAGE_SHIFT)

#endif

#define PAGE_MAX_SHIFT          14
#define PAGE_MAX_SIZE           (1 << PAGE_MAX_SHIFT)
#define PAGE_MAX_MASK           (PAGE_MAX_SIZE-1)

#define PAGE_MIN_SHIFT          12
#define PAGE_MIN_SIZE           (1 << PAGE_MIN_SHIFT)
#define PAGE_MIN_MASK           (PAGE_MIN_SIZE-1)

#ifndef __ASSEMBLER__

#ifdef  MACH_KERNEL_PRIVATE

#define VM32_SUPPORT            1
#define VM32_MIN_ADDRESS        ((vm32_offset_t) 0)
#define VM32_MAX_ADDRESS        ((vm32_offset_t) (VM_MAX_PAGE_ADDRESS & 0xFFFFFFFF))
#define VM_MAX_PAGE_ADDRESS     VM_MAX_ADDRESS  /* ARM64_TODO: ?? */

/*
 * kalloc() parameters:
 *
 * Historically kalloc's underlying zones were power-of-2 sizes, with a
 * KALLOC_MINSIZE of 16 bytes.  Thus the allocator ensured that
 * (sizeof == alignof) >= 16 for all kalloc allocations.
 *
 * Today kalloc may use zones with intermediate (small) sizes, constrained by
 * KALLOC_MINSIZE and a minimum alignment, expressed by KALLOC_LOG2_MINALIGN.
 *
 * Note that most dynamically allocated data structures contain more than
 * one int/long/pointer member, so KALLOC_MINSIZE should probably start at 8.
 */

#if defined (__arm__)

#define KALLOC_MINSIZE          8       /* minimum allocation size */
#define KALLOC_LOG2_MINALIGN    3       /* log2 minimum alignment */

#elif defined(__arm64__)

#define KALLOC_MINSIZE          16      /* minimum allocation size */
#define KALLOC_LOG2_MINALIGN    4       /* log2 minimum alignment */

#else
#error Unsupported arch
#endif

#endif

#if defined (__arm__)

#define VM_MIN_ADDRESS          ((vm_address_t) 0x00000000)
#define VM_MAX_ADDRESS          ((vm_address_t) 0x80000000)

/* system-wide values */
#define MACH_VM_MIN_ADDRESS     ((mach_vm_offset_t) 0)
#define MACH_VM_MAX_ADDRESS     ((mach_vm_offset_t) VM_MAX_ADDRESS)

#elif defined (__arm64__)

#define VM_MIN_ADDRESS          ((vm_address_t) 0x0000000000000000ULL)
#define VM_MAX_ADDRESS          ((vm_address_t) 0x0000000080000000ULL)

/* system-wide values */
#define MACH_VM_MIN_ADDRESS_RAW 0x0ULL
#define MACH_VM_MAX_ADDRESS_RAW 0x0000000FC0000000ULL
#define MACH_VM_MIN_ADDRESS     ((mach_vm_offset_t) MACH_VM_MIN_ADDRESS_RAW)
#define MACH_VM_MAX_ADDRESS     ((mach_vm_offset_t) MACH_VM_MAX_ADDRESS_RAW)


#else
#error architecture not supported
#endif

#define VM_MAP_MIN_ADDRESS      VM_MIN_ADDRESS
#define VM_MAP_MAX_ADDRESS      VM_MAX_ADDRESS

#ifdef  KERNEL

#if defined (__arm__)
#define VM_KERNEL_POINTER_SIGNIFICANT_BITS  32
#define VM_MIN_KERNEL_ADDRESS   ((vm_address_t) 0x80000000)
#define VM_MAX_KERNEL_ADDRESS   ((vm_address_t) 0xFFFEFFFF)
#define VM_HIGH_KERNEL_WINDOW   ((vm_address_t) 0xFFFE0000)
#elif defined (__arm64__)
/*
 * The minimum and maximum kernel address; some configurations may
 * constrain the address space further.
 */
#define VM_KERNEL_POINTER_SIGNIFICANT_BITS  37
#define VM_MIN_KERNEL_ADDRESS   ((vm_address_t) 0xffffffe000000000ULL)
#define VM_MAX_KERNEL_ADDRESS   ((vm_address_t) 0xfffffffbffffffffULL)
#else
#error architecture not supported
#endif

#define VM_MIN_KERNEL_AND_KEXT_ADDRESS  \
	                        VM_MIN_KERNEL_ADDRESS

#if __has_feature(ptrauth_calls)
#include <ptrauth.h>
#define VM_KERNEL_STRIP_PTR(_v) (ptrauth_strip((void *)(uintptr_t)(_v), ptrauth_key_asia))
#else /* !ptrauth_calls */
#define VM_KERNEL_STRIP_PTR(_v) (_v)
#endif /* ptrauth_calls */

#define VM_KERNEL_ADDRESS(_va)  \
	((((vm_address_t)VM_KERNEL_STRIP_PTR(_va)) >= VM_MIN_KERNEL_ADDRESS) && \
	 (((vm_address_t)VM_KERNEL_STRIP_PTR(_va)) <= VM_MAX_KERNEL_ADDRESS))

#ifdef  MACH_KERNEL_PRIVATE
/*
 *	Physical memory is mapped linearly at an offset virtual memory.
 */
extern unsigned long            gVirtBase, gPhysBase, gPhysSize;

#define isphysmem(a)            (((vm_address_t)(a) - gPhysBase) < gPhysSize)
#define physmap_enclosed(a)     isphysmem(a)

#if KASAN
/* Increase the stack sizes to account for the redzones that get added to every
 * stack object. */
# define KERNEL_STACK_SIZE      (4*4*4096)
#elif DEBUG
/**
 * Increase the stack size to account for less efficient use of stack space when
 * compiling with -O0.
 */
# define KERNEL_STACK_SIZE      (2*4*4096)
#else
# define KERNEL_STACK_SIZE      (4*4096)
#endif

#define INTSTACK_SIZE           (4*4096)

#ifdef __arm64__
#define EXCEPSTACK_SIZE         (4*4096)
#else
#define FIQSTACK_SIZE           (4096)
#endif

#if defined (__arm__)
#define HIGH_EXC_VECTORS        ((vm_address_t) 0xFFFF0000)
#endif

/*
 * TODO: We're hardcoding the expected virtual TEXT base here;
 * that gives us an ugly dependency on a linker argument in
 * the make files.  Clean this up, so we don't hardcode it
 * twice; this is nothing but trouble.
 */
#if defined (__arm__)
#define VM_KERNEL_LINK_ADDRESS  ((vm_address_t) 0x80000000)
#elif defined (__arm64__)
#define VM_KERNEL_LINK_ADDRESS  ((vm_address_t) 0xFFFFFFF007004000)
#else
#error architecture not supported
#endif

#endif  /* MACH_KERNEL_PRIVATE */
#endif  /* KERNEL */

#endif  /* !__ASSEMBLER__ */

#define SWI_SYSCALL     0x80

#endif  /* _MACH_ARM_VM_PARAM_H_ */
