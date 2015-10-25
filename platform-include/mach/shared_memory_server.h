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
 *
 *	File: mach/shared_memory_server.h
 *
 * 	protos and struct definitions for shared library
 *	server and interface
 */

/*
 * XXX
 * 
 * NOTE: this file is deprecated and will be removed in the near future.
 * Any project that includes this file should be changed to:
 * 1. use <mach/shared_region.h> instead of this file,
 * 2. handle the new shared regions, now available on more platforms
 */

#ifndef _MACH_SHARED_MEMORY_SERVER_H_
#define _MACH_SHARED_MEMORY_SERVER_H_

#warning "<mach/shared_memory_server.h> is deprecated.  Please use <mach/shared_region.h> instead."

#include <sys/cdefs.h>
#include <mach/vm_prot.h>
#include <mach/vm_types.h>
#include <mach/mach_types.h>

#define VM_PROT_COW  0x8  /* must not interfere with normal prot assignments */
#define VM_PROT_ZF  0x10  /* must not interfere with normal prot assignments */

#define GLOBAL_SHARED_TEXT_SEGMENT	0x90000000U
#define GLOBAL_SHARED_DATA_SEGMENT	0xA0000000U
#define GLOBAL_SHARED_SEGMENT_MASK	0xF0000000U

#define	SHARED_TEXT_REGION_SIZE		0x10000000
#define	SHARED_DATA_REGION_SIZE		0x10000000

#if !defined(__LP64__)

#define	SHARED_LIBRARY_SERVER_SUPPORTED

#define	SHARED_ALTERNATE_LOAD_BASE	0x09000000

/* 
 *  Note: the two masks below are useful because the assumption is 
 *  made that these shared regions will always be mapped on natural boundaries 
 *  i.e. if the size is 0x10000000 the object can be mapped at 
 *  0x20000000, or 0x30000000, but not 0x1000000
 */
#define	SHARED_TEXT_REGION_MASK		0x0FFFFFFF
#define	SHARED_DATA_REGION_MASK		0x0FFFFFFF


/* flags field aliases for copyin_shared_file and load_shared_file */

/* IN */
#define ALTERNATE_LOAD_SITE 0x1
#define NEW_LOCAL_SHARED_REGIONS 0x2
#define	QUERY_IS_SYSTEM_REGION 0x4

/* OUT */
#define SF_PREV_LOADED    0x1
#define SYSTEM_REGION_BACKED 0x2


struct sf_mapping {
	vm_offset_t	mapping_offset;
	vm_size_t	size;
	vm_offset_t	file_offset;
	vm_prot_t	protection;  /* read/write/execute/COW/ZF */
	vm_offset_t	cksum;
};
typedef struct sf_mapping sf_mapping_t;

#endif  /* !defined(__LP64__) */

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

struct shared_region_range_np {
	mach_vm_address_t	srr_address;
	mach_vm_size_t		srr_size;
};


__BEGIN_DECLS
int	shared_region_map_file_np(int fd,
				  uint32_t mappingCount,
				  const struct shared_file_mapping_np *mappings,
				  int64_t *slide_p);
int	shared_region_make_private_np(uint32_t rangeCount,
				      const struct shared_region_range_np *ranges);
__END_DECLS


#endif /* _MACH_SHARED_MEMORY_SERVER_H_ */
