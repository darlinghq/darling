/*
 * Copyright (c) 2000-2018 Apple Inc. All rights reserved.
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
 *
 */
#ifndef _MACH_VM_TYPES_H_
#define _MACH_VM_TYPES_H_

#include <mach/port.h>
#include <mach/machine/vm_types.h>

#include <stdint.h>

typedef vm_offset_t             pointer_t;
typedef vm_offset_t             vm_address_t;

/*
 * We use addr64_t for 64-bit addresses that are used on both
 * 32 and 64-bit machines.  On PPC, they are passed and returned as
 * two adjacent 32-bit GPRs.  We use addr64_t in places where
 * common code must be useable both on 32 and 64-bit machines.
 */
typedef uint64_t addr64_t;              /* Basic effective address */

/*
 * We use reg64_t for addresses that are 32 bits on a 32-bit
 * machine, and 64 bits on a 64-bit machine, but are always
 * passed and returned in a single GPR on PPC.  This type
 * cannot be used in generic 32-bit c, since on a 64-bit
 * machine the upper half of the register will be ignored
 * by the c compiler in 32-bit mode.  In c, we can only use the
 * type in prototypes of functions that are written in and called
 * from assembly language.  This type is basically a comment.
 */
typedef uint32_t        reg64_t;

/*
 * To minimize the use of 64-bit fields, we keep some physical
 * addresses (that are page aligned) as 32-bit page numbers.
 * This limits the physical address space to 16TB of RAM.
 */
typedef uint32_t ppnum_t;               /* Physical page number */
#define PPNUM_MAX UINT32_MAX


#ifdef  KERNEL_PRIVATE

#include <sys/cdefs.h>

#ifndef MACH_KERNEL_PRIVATE
/*
 * Use specifically typed null structures for these in
 * other parts of the kernel to enable compiler warnings
 * about type mismatches, etc...  Otherwise, these would
 * be void*.
 */
__BEGIN_DECLS

struct pmap;
struct _vm_map;
struct vm_object;

__END_DECLS

#endif  /* MACH_KERNEL_PRIVATE */

typedef struct pmap             *pmap_t;
typedef struct _vm_map          *vm_map_t;
typedef struct vm_object        *vm_object_t;
typedef struct vm_object_fault_info     *vm_object_fault_info_t;

#define PMAP_NULL               ((pmap_t) NULL)
#define VM_OBJECT_NULL  ((vm_object_t) NULL)

#else   /* KERNEL_PRIVATE */

typedef mach_port_t             vm_map_t;

#endif  /* KERNEL_PRIVATE */

#ifdef KERNEL
#define VM_MAP_NULL             ((vm_map_t) NULL)
#else
#define VM_MAP_NULL             ((vm_map_t) 0)
#endif

/*
 * Evolving definitions, likely to change.
 */

typedef uint64_t                vm_object_offset_t;
typedef uint64_t                vm_object_size_t;


#ifdef XNU_KERNEL_PRIVATE

#define VM_TAG_ACTIVE_UPDATE    1

typedef uint16_t vm_tag_t;

#define VM_TAG_NAME_LEN_MAX     0x7F
#define VM_TAG_NAME_LEN_SHIFT   0
#define VM_TAG_BT               0x0080
#define VM_TAG_UNLOAD           0x0100
#define VM_TAG_KMOD             0x0200

#if DEBUG || DEVELOPMENT
#define VM_MAX_TAG_ZONES        28
#else
#define VM_MAX_TAG_ZONES        0
#endif

#if VM_MAX_TAG_ZONES
// must be multiple of 64
#define VM_MAX_TAG_VALUE        1536
#else
#define VM_MAX_TAG_VALUE        256
#endif


#define ARRAY_COUNT(a)  (sizeof((a)) / sizeof((a)[0]))

struct vm_allocation_total {
	vm_tag_t tag;
	uint64_t total;
};

struct vm_allocation_zone_total {
	uint64_t  total;
	uint64_t  peak;
	uint32_t  waste;
	uint32_t  wastediv;
};
typedef struct vm_allocation_zone_total vm_allocation_zone_total_t;

struct vm_allocation_site {
	uint64_t  total;
#if DEBUG || DEVELOPMENT
	uint64_t  peak;
#endif /* DEBUG || DEVELOPMENT */
	uint64_t  mapped;
	int16_t   refcount;
	vm_tag_t  tag;
	uint16_t  flags;
	uint16_t  subtotalscount;
	struct vm_allocation_total subtotals[0];
	/* char      name[0]; -- this is placed after subtotals, see KA_NAME() */
};
typedef struct vm_allocation_site vm_allocation_site_t;

#define VM_ALLOC_SITE_STATIC(iflags, itag)                                          \
	static vm_allocation_site_t site __attribute__((section("__DATA, __data"))) \
	 = { .refcount = 2, .tag = (itag), .flags = (iflags) };

extern int vmrtf_extract(uint64_t, boolean_t, int, void *, int *);
extern unsigned int vmrtfaultinfo_bufsz(void);

#endif /* XNU_KERNEL_PRIVATE */

#ifdef  KERNEL_PRIVATE

#ifndef MACH_KERNEL_PRIVATE

__BEGIN_DECLS

struct upl;
struct vm_map_copy;
struct vm_named_entry;

__END_DECLS

#endif  /* MACH_KERNEL_PRIVATE */

typedef struct upl              *upl_t;
typedef struct vm_map_copy      *vm_map_copy_t;
typedef struct vm_named_entry   *vm_named_entry_t;

#define VM_MAP_COPY_NULL        ((vm_map_copy_t) NULL)

#else   /* KERNEL_PRIVATE */

typedef mach_port_t             upl_t;
typedef mach_port_t             vm_named_entry_t;

#endif  /* KERNEL_PRIVATE */

#ifdef KERNEL
#define UPL_NULL                ((upl_t) NULL)
#define VM_NAMED_ENTRY_NULL     ((vm_named_entry_t) NULL)
#else
#define UPL_NULL                ((upl_t) 0)
#define VM_NAMED_ENTRY_NULL     ((vm_named_entry_t) 0)
#endif

#ifdef PRIVATE
typedef struct {
	uint64_t rtfabstime; // mach_continuous_time at start of fault
	uint64_t rtfduration; // fault service duration
	uint64_t rtfaddr; // fault address
	uint64_t rtfpc; // userspace program counter of thread incurring the fault
	uint64_t rtftid; // thread ID
	uint64_t rtfupid; // process identifier
	uint64_t rtftype; // fault type
} vm_rtfault_record_t;
#endif
#endif  /* _MACH_VM_TYPES_H_ */
