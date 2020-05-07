/*
 * Copyright (c) 2000-2006 Apple Computer, Inc. All rights reserved.
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
 * Copyright (c) 1991,1990,1989,1988,1987 Carnegie Mellon University
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
 *	File:	mach/vm_param.h
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *	Date:	1985
 *
 *	Machine independent virtual memory parameters.
 *
 */

#ifndef _MACH_VM_PARAM_H_
#define _MACH_VM_PARAM_H_

#include <mach/machine/vm_param.h>

#ifdef  KERNEL

#ifndef ASSEMBLER
#include <mach/vm_types.h>
#endif  /* ASSEMBLER */

#include <os/base.h>
#include <os/overflow.h>

/*
 *	The machine independent pages are refered to as PAGES.  A page
 *	is some number of hardware pages, depending on the target machine.
 */

#ifndef ASSEMBLER

#define PAGE_SIZE_64 (unsigned long long)PAGE_SIZE              /* pagesize in addr units */
#define PAGE_MASK_64 (unsigned long long)PAGE_MASK              /* mask for off in page */

/*
 *	Convert addresses to pages and vice versa.  No rounding is used.
 *      The atop_32 and ptoa_32 macros should not be use on 64 bit types.
 *      The round_page_64 and trunc_page_64 macros should be used instead.
 */

#define atop_32(x) ((uint32_t)(x) >> PAGE_SHIFT)
#define ptoa_32(x) ((uint32_t)(x) << PAGE_SHIFT)
#define atop_64(x) ((uint64_t)(x) >> PAGE_SHIFT)
#define ptoa_64(x) ((uint64_t)(x) << PAGE_SHIFT)

#define atop_kernel(x) ((vm_address_t)(x) >> PAGE_SHIFT)
#define ptoa_kernel(x) ((vm_address_t)(x) << PAGE_SHIFT)

/*
 *      While the following block is enabled, the legacy atop and ptoa
 *      macros will behave correctly.  If not, they will generate
 *      invalid lvalue errors.
 */

#if 1
#define atop(x) ((vm_address_t)(x) >> PAGE_SHIFT)
#define ptoa(x) ((vm_address_t)(x) << PAGE_SHIFT)
#else
#define atop(x) (0UL = 0)
#define ptoa(x) (0UL = 0)
#endif

/*
 *	Page-size rounding macros for the Public fixed-width VM types.
 */
#define mach_vm_round_page(x) (((mach_vm_offset_t)(x) + PAGE_MASK) & ~((signed)PAGE_MASK))
#define mach_vm_trunc_page(x) ((mach_vm_offset_t)(x) & ~((signed)PAGE_MASK))

#define round_page_overflow(in, out) __os_warn_unused(({ \
	        bool __ovr = os_add_overflow(in, (__typeof__(*out))PAGE_MASK, out); \
	        *out &= ~((__typeof__(*out))PAGE_MASK); \
	        __ovr; \
	}))

static inline int OS_WARN_RESULT
mach_vm_round_page_overflow(mach_vm_offset_t in, mach_vm_offset_t *out)
{
	return round_page_overflow(in, out);
}

#define memory_object_round_page(x) (((memory_object_offset_t)(x) + PAGE_MASK) & ~((signed)PAGE_MASK))
#define memory_object_trunc_page(x) ((memory_object_offset_t)(x) & ~((signed)PAGE_MASK))

/*
 *	Rounding macros for the legacy (scalable with the current task's
 *	address space size) VM types.
 */

#define round_page(x) (((vm_offset_t)(x) + PAGE_MASK) & ~((vm_offset_t)PAGE_MASK))
#define trunc_page(x) ((vm_offset_t)(x) & ~((vm_offset_t)PAGE_MASK))

/*
 *	Round off or truncate to the nearest page.  These will work
 *	for either addresses or counts.  (i.e. 1 byte rounds to 1 page
 *	bytes.  The round_page_32 and trunc_page_32 macros should not be
 *      use on 64 bit types.  The round_page_64 and trunc_page_64 macros
 *      should be used instead.
 *
 *	These should only be used in the rare case the size of the address
 *	or length is hard-coded as 32 or 64 bit.  Otherwise, the macros
 *	associated with the specific VM type should be used.
 */

#define round_page_32(x) (((uint32_t)(x) + PAGE_MASK) & ~((uint32_t)PAGE_MASK))
#define trunc_page_32(x) ((uint32_t)(x) & ~((uint32_t)PAGE_MASK))
#define round_page_64(x) (((uint64_t)(x) + PAGE_MASK_64) & ~((uint64_t)PAGE_MASK_64))
#define trunc_page_64(x) ((uint64_t)(x) & ~((uint64_t)PAGE_MASK_64))

/*
 *      Enable the following block to find uses of xxx_32 macros that should
 *      be xxx_64.  These macros only work in C code, not C++.  The resulting
 *      binaries are not functional.  Look for invalid lvalue errors in
 *      the compiler output.
 *
 *      Enabling the following block will also find use of the xxx_64 macros
 *      that have been passed pointers.  The parameters should be case to an
 *      unsigned long type first.  Look for invalid operands to binary + error
 *      in the compiler output.
 */

#if 0
#undef atop_32
#undef ptoa_32
#undef round_page_32
#undef trunc_page_32
#undef atop_64
#undef ptoa_64
#undef round_page_64
#undef trunc_page_64

#ifndef __cplusplus

#define atop_32(x) \
    (__builtin_choose_expr (sizeof(x) != sizeof(uint64_t), \
	(*(long *)0), \
	(0UL)) = 0)

#define ptoa_32(x) \
    (__builtin_choose_expr (sizeof(x) != sizeof(uint64_t), \
	(*(long *)0), \
	(0UL)) = 0)

#define round_page_32(x) \
    (__builtin_choose_expr (sizeof(x) != sizeof(uint64_t), \
	(*(long *)0), \
	(0UL)) = 0)

#define trunc_page_32(x) \
    (__builtin_choose_expr (sizeof(x) != sizeof(uint64_t), \
	(*(long *)0), \
	(0UL)) = 0)
#else

#define atop_32(x) (0)
#define ptoa_32(x) (0)
#define round_page_32(x) (0)
#define trunc_page_32(x) (0)

#endif /* ! __cplusplus */

#define atop_64(x) ((uint64_t)((x) + (uint8_t *)0))
#define ptoa_64(x) ((uint64_t)((x) + (uint8_t *)0))
#define round_page_64(x) ((uint64_t)((x) + (uint8_t *)0))
#define trunc_page_64(x) ((uint64_t)((x) + (uint8_t *)0))

#endif

/*
 *	Determine whether an address is page-aligned, or a count is
 *	an exact page multiple.
 */

#define page_aligned(x) (((x) & PAGE_MASK) == 0)

extern vm_size_t        mem_size;               /* 32-bit size of memory - limited by maxmem - deprecated */
extern uint64_t         max_mem;                /* 64-bit size of memory - limited by maxmem */

/*
 * The default pager does not handle 64-bit offsets inside its objects,
 * so this limits the size of anonymous memory objects to 4GB minus 1 page.
 * When we need to allocate a chunk of anonymous memory over that size,
 * we have to allocate more than one chunk.
 */
#define ANON_MAX_SIZE   0xFFFFF000ULL
/*
 * Work-around for <rdar://problem/6626493>
 * Break large anonymous memory areas into 128MB chunks to alleviate
 * the cost of copying when copy-on-write is not possible because a small
 * portion of it being wired.
 */
#define ANON_CHUNK_SIZE (128ULL * 1024 * 1024) /* 128MB */

/*
 * The 'medium' malloc allocator would like its regions
 * to be chunked up into MALLOC_MEDIUM_CHUNK_SIZE chunks
 * and backed by different objects. This avoids contention
 * on a single large object and showed solid improvements on high
 * core machines with workloads involving video and graphics processing.
 */
#define MALLOC_MEDIUM_CHUNK_SIZE (8ULL * 1024 * 1024) /* 8 MB */

#ifdef  XNU_KERNEL_PRIVATE

#include <kern/debug.h>

extern uint64_t         mem_actual;             /* 64-bit size of memory - not limited by maxmem */
extern uint64_t         sane_size;              /* Memory size to use for defaults calculations */
extern addr64_t         vm_last_addr;   /* Highest kernel virtual address known to the VM system */

extern const vm_offset_t        vm_min_kernel_address;
extern const vm_offset_t        vm_max_kernel_address;

extern vm_offset_t              vm_kernel_stext;
extern vm_offset_t              vm_kernel_etext;
extern vm_offset_t              vm_kernel_slid_base;
extern vm_offset_t              vm_kernel_slid_top;
extern vm_offset_t              vm_kernel_slide;
extern vm_offset_t              vm_kernel_addrperm;
extern vm_offset_t              vm_kext_base;
extern vm_offset_t              vm_kext_top;
extern vm_offset_t              vm_kernel_base;
extern vm_offset_t              vm_kernel_top;
extern vm_offset_t              vm_hib_base;

extern vm_offset_t              vm_kernel_builtinkmod_text;
extern vm_offset_t              vm_kernel_builtinkmod_text_end;

#define VM_KERNEL_IS_SLID(_o)                                             \
	(((vm_offset_t)VM_KERNEL_STRIP_PTR(_o) >= vm_kernel_slid_base) && \
	 ((vm_offset_t)VM_KERNEL_STRIP_PTR(_o) <  vm_kernel_slid_top))

#define VM_KERNEL_SLIDE(_u) ((vm_offset_t)(_u) + vm_kernel_slide)

/*
 * The following macros are to be used when exposing kernel addresses to
 * userspace via any of the various debug or info facilities that might exist
 * (e.g. stackshot, proc_info syscall, etc.). It is important to understand
 * the goal of each macro and choose the right one depending on what you are
 * trying to do. Misuse of these macros can result in critical data leaks
 * which in turn lead to all sorts of system vulnerabilities. It is invalid to
 * call these macros on a non-kernel address (NULL is allowed).
 *
 * VM_KERNEL_UNSLIDE:
 *     Use this macro when you are exposing an address to userspace which is
 *     *guaranteed* to be a "static" kernel or kext address (i.e. coming from text
 *     or data sections). These are the addresses which get "slid" via ASLR on
 *     kernel or kext load, and it's precisely the slide value we are trying to
 *     protect from userspace.
 *
 * VM_KERNEL_ADDRHIDE:
 *     Use when exposing an address for internal purposes: debugging, tracing,
 *     etc. The address will be unslid if necessary. Other addresses will be
 *     hidden on customer builds, and unmodified on internal builds.
 *
 * VM_KERNEL_ADDRHASH:
 *     Use this macro when exposing a kernel address to userspace on customer
 *     builds. The address can be from the static kernel or kext regions, or the
 *     kernel heap. The address will be unslid or hashed as appropriate.
 *
 *
 * ** SECURITY WARNING: The following macros can leak kernel secrets.
 *                      Use *only* in performance *critical* code.
 *
 * VM_KERNEL_ADDRPERM:
 * VM_KERNEL_UNSLIDE_OR_PERM:
 *     Use these macros when exposing a kernel address to userspace on customer
 *     builds. The address can be from the static kernel or kext regions, or the
 *     kernel heap. The address will be unslid or permuted as appropriate.
 *
 * Nesting of these macros should be considered invalid.
 */

__BEGIN_DECLS
#if XNU_KERNEL_PRIVATE
extern vm_offset_t vm_kernel_addrhash(vm_offset_t addr)
__XNU_INTERNAL(vm_kernel_addrhash);
#else
extern vm_offset_t vm_kernel_addrhash(vm_offset_t addr);
#endif
__END_DECLS

#define __DO_UNSLIDE(_v) ((vm_offset_t)VM_KERNEL_STRIP_PTR(_v) - vm_kernel_slide)

#if DEBUG || DEVELOPMENT
#define VM_KERNEL_ADDRHIDE(_v) (VM_KERNEL_IS_SLID(_v) ? __DO_UNSLIDE(_v) : (vm_address_t)VM_KERNEL_STRIP_PTR(_v))
#else
#define VM_KERNEL_ADDRHIDE(_v) (VM_KERNEL_IS_SLID(_v) ? __DO_UNSLIDE(_v) : (vm_address_t)0)
#endif /* DEBUG || DEVELOPMENT */

#define VM_KERNEL_ADDRHASH(_v) vm_kernel_addrhash((vm_offset_t)(_v))

#define VM_KERNEL_UNSLIDE_OR_PERM(_v) ({ \
	        VM_KERNEL_IS_SLID(_v) ? __DO_UNSLIDE(_v) : \
	        VM_KERNEL_ADDRESS(_v) ? ((vm_offset_t)VM_KERNEL_STRIP_PTR(_v) + vm_kernel_addrperm) : \
	        (vm_offset_t)VM_KERNEL_STRIP_PTR(_v); \
	})

#define VM_KERNEL_UNSLIDE(_v) ({ \
	        VM_KERNEL_IS_SLID(_v) ? __DO_UNSLIDE(_v) : (vm_offset_t)0; \
	})

#define VM_KERNEL_ADDRPERM(_v) VM_KERNEL_UNSLIDE_OR_PERM(_v)

#undef mach_vm_round_page
#undef round_page
#undef round_page_32
#undef round_page_64

static inline mach_vm_offset_t
mach_vm_round_page(mach_vm_offset_t x)
{
	if (round_page_overflow(x, &x)) {
		panic("overflow detected");
	}
	return x;
}

static inline vm_offset_t
round_page(vm_offset_t x)
{
	if (round_page_overflow(x, &x)) {
		panic("overflow detected");
	}
	return x;
}

static inline mach_vm_offset_t
round_page_64(mach_vm_offset_t x)
{
	if (round_page_overflow(x, &x)) {
		panic("overflow detected");
	}
	return x;
}

static inline uint32_t
round_page_32(uint32_t x)
{
	if (round_page_overflow(x, &x)) {
		panic("overflow detected");
	}
	return x;
}

#endif  /* XNU_KERNEL_PRIVATE */

extern vm_size_t        page_size;
extern vm_size_t        page_mask;
extern int              page_shift;

/* We need a way to get rid of compiler warnings when we cast from   */
/* a 64 bit value to an address (which may be 32 bits or 64-bits).   */
/* An intptr_t is used convert the value to the right precision, and */
/* then to an address. This macro is also used to convert addresses  */
/* to 32-bit integers, which is a hard failure for a 64-bit kernel   */
#include <stdint.h>
#ifndef __CAST_DOWN_CHECK
#define __CAST_DOWN_CHECK

#define CAST_DOWN( type, addr ) \
    ( ((type)((uintptr_t) (addr)/(sizeof(type) < sizeof(uintptr_t) ? 0 : 1))) )

#define CAST_DOWN_EXPLICIT( type, addr )  ( ((type)((uintptr_t) (addr))) )

#endif /* __CAST_DOWN_CHECK */

#endif  /* ASSEMBLER */

#endif  /* KERNEL */

#endif  /* _MACH_VM_PARAM_H_ */
