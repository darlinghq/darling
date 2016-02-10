/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 *
 *	File: mach/shared_region.h
 *
 * 	protos and struct definitions for shared region
 */

#ifndef _MACH_SHARED_REGION_H_
#define _MACH_SHARED_REGION_H_

#include <sys/cdefs.h>
#include <mach/vm_prot.h>
#include <mach/vm_types.h>
#include <mach/mach_types.h>

#define SHARED_REGION_BASE_I386			0x90000000ULL
#define SHARED_REGION_SIZE_I386			0x20000000ULL
#define SHARED_REGION_NESTING_BASE_I386		0x90000000ULL
#define SHARED_REGION_NESTING_SIZE_I386		0x20000000ULL
#define SHARED_REGION_NESTING_MIN_I386		0x00200000ULL
#define SHARED_REGION_NESTING_MAX_I386		0xFFE00000ULL

#define SHARED_REGION_BASE_X86_64		0x00007FFF70000000ULL
#define SHARED_REGION_SIZE_X86_64		0x000000008FE00000ULL
#define SHARED_REGION_NESTING_BASE_X86_64	0x00007FFF80000000ULL
#define SHARED_REGION_NESTING_SIZE_X86_64	0x0000000040000000ULL
#define SHARED_REGION_NESTING_MIN_X86_64	0x0000000000200000ULL
#define SHARED_REGION_NESTING_MAX_X86_64	0xFFFFFFFFFFE00000ULL

#define SHARED_REGION_BASE_PPC			0x90000000ULL
#define SHARED_REGION_SIZE_PPC			0x20000000ULL
#define SHARED_REGION_NESTING_BASE_PPC		0x90000000ULL
#define SHARED_REGION_NESTING_SIZE_PPC		0x10000000ULL
#define SHARED_REGION_NESTING_MIN_PPC		0x10000000ULL
#define SHARED_REGION_NESTING_MAX_PPC		0x10000000ULL

#define SHARED_REGION_BASE_PPC64		0x00007FFF60000000ULL
#define SHARED_REGION_SIZE_PPC64		0x00000000A0000000ULL
#define SHARED_REGION_NESTING_BASE_PPC64	0x00007FFF60000000ULL
#define SHARED_REGION_NESTING_SIZE_PPC64	0x00000000A0000000ULL
#define SHARED_REGION_NESTING_MIN_PPC64		0x0000000010000000ULL
#define SHARED_REGION_NESTING_MAX_PPC64		0x0000000010000000ULL

#define SHARED_REGION_BASE_ARM			0x20000000ULL
#define SHARED_REGION_SIZE_ARM			0x20000000ULL
#define SHARED_REGION_NESTING_BASE_ARM		0x20000000ULL
#define SHARED_REGION_NESTING_SIZE_ARM		0x20000000ULL
#define SHARED_REGION_NESTING_MIN_ARM		?
#define SHARED_REGION_NESTING_MAX_ARM		?

#define SHARED_REGION_BASE_ARM64		0x180000000ULL
#define SHARED_REGION_SIZE_ARM64		0x20000000ULL
#define SHARED_REGION_NESTING_BASE_ARM64	0x180000000ULL
#define SHARED_REGION_NESTING_SIZE_ARM64	0x20000000ULL
#define SHARED_REGION_NESTING_MIN_ARM64		?
#define SHARED_REGION_NESTING_MAX_ARM64		?

#if defined(__i386__)
#define SHARED_REGION_BASE			SHARED_REGION_BASE_I386
#define SHARED_REGION_SIZE			SHARED_REGION_SIZE_I386
#define SHARED_REGION_NESTING_BASE		SHARED_REGION_NESTING_BASE_I386
#define SHARED_REGION_NESTING_SIZE		SHARED_REGION_NESTING_SIZE_I386
#define SHARED_REGION_NESTING_MIN		SHARED_REGION_NESTING_MIN_I386
#define SHARED_REGION_NESTING_MAX		SHARED_REGION_NESTING_MAX_I386
#elif defined(__x86_64__)
#define SHARED_REGION_BASE			SHARED_REGION_BASE_X86_64
#define SHARED_REGION_SIZE			SHARED_REGION_SIZE_X86_64
#define SHARED_REGION_NESTING_BASE		SHARED_REGION_NESTING_BASE_X86_64
#define SHARED_REGION_NESTING_SIZE		SHARED_REGION_NESTING_SIZE_X86_64
#define SHARED_REGION_NESTING_MIN		SHARED_REGION_NESTING_MIN_X86_64
#define SHARED_REGION_NESTING_MAX		SHARED_REGION_NESTING_MAX_X86_64
#endif

/* 
 * All shared_region_* declarations are a private interface
 * between dyld and the kernel.
 *
 */
struct shared_file_mapping_np {
	mach_vm_address_t	sfm_address;
	mach_vm_size_t		sfm_size;
	mach_vm_offset_t	sfm_file_offset;
	vm_prot_t		sfm_max_prot;
	vm_prot_t		sfm_init_prot;
};
#define VM_PROT_COW  0x8  /* must not interfere with normal prot assignments */
#define VM_PROT_ZF  0x10  /* must not interfere with normal prot assignments */
#define VM_PROT_SLIDE  0x20  /* must not interfere with normal prot assignments */


__BEGIN_DECLS
int	shared_region_check_np(uint64_t	*startaddress);
int	shared_region_map_np(int fd,
			     uint32_t mappingCount,
			     const struct shared_file_mapping_np *mappings);
int	shared_region_slide_np(void);
__END_DECLS


#endif /* _MACH_SHARED_REGION_H_ */
