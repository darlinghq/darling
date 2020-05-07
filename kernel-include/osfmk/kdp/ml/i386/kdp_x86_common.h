/*
 * Copyright (c) 2011 Apple Inc. All rights reserved.
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

#ifndef _KDP_X86_COMMON_H_
#define _KDP_X86_COMMON_H_

#include <libsa/types.h>
#include <mach/machine/vm_types.h>
#include <i386/pmap.h>

/* data required for JTAG extraction of coredump */
typedef struct _kdp_jtag_coredump_t {
	uint64_t signature;
	uint64_t version;
	uint64_t kernel_map_start;
	uint64_t kernel_map_end;
	uint64_t kernel_pmap_pml4;
	uint64_t pmap_memory_regions;
	uint64_t pmap_memory_region_count;
	uint64_t pmap_memory_region_t_size;
	uint64_t physmap_base;
} kdp_jtag_coredump_t;

/* signature used to verify kdp_jtag_coredump_t structure */
#define KDP_JTAG_COREDUMP_SIGNATURE     0x434f524544554d50

/* version of kdp_jtag_coredump_t structure */
#define KDP_JTAG_COREDUMP_VERSION_1     1

void kdp_map_debug_pagetable_window(void);
void kdp_jtag_coredump_init(void);

#endif /* _KDP_X86_COMMON_H_ */
