/*
 * Copyright (c) 2007-2016 Apple Inc. All rights reserved.
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
 * ARM CPU identification
 */

#ifndef _MACHINE_CPUID_H_
#define _MACHINE_CPUID_H_

#include <stdint.h>
#include <mach/boolean.h>
#include <machine/machine_cpuid.h>

typedef struct {
	uint32_t arm_rev : 4,  /* 00:03 revision number */
	    arm_part         : 12,/* 04:15 primary part number */
	    arm_arch         : 4,/* 16:19 architecture */
	    arm_variant      : 4,/* 20:23 variant  */
	    arm_implementor  : 8;/* 24:31 implementor (0x41) */
} arm_cpuid_bits_t;

typedef union {
	arm_cpuid_bits_t arm_info; /* ARM9xx,  ARM11xx, and later processors */
	uint32_t         value;
} arm_cpu_info_t;

/* Implementor codes */
#define CPU_VID_ARM      0x41 // ARM Limited
#define CPU_VID_DEC      0x44 // Digital Equipment Corporation
#define CPU_VID_MOTOROLA 0x4D // Motorola - Freescale Semiconductor Inc.
#define CPU_VID_MARVELL  0x56 // Marvell Semiconductor Inc.
#define CPU_VID_INTEL    0x69 // Intel ARM parts.
#define CPU_VID_APPLE    0x61 // Apple Inc.


/* ARM Architecture Codes */

#define CPU_ARCH_ARMv4    0x1 /* ARMv4 */
#define CPU_ARCH_ARMv4T   0x2 /* ARMv4 + Thumb */
#define CPU_ARCH_ARMv5    0x3 /* ARMv5 */
#define CPU_ARCH_ARMv5T   0x4 /* ARMv5 + Thumb */
#define CPU_ARCH_ARMv5TE  0x5 /* ARMv5 + Thumb + Extensions(?) */
#define CPU_ARCH_ARMv5TEJ 0x6 /* ARMv5 + Thumb + Extensions(?) + //Jazelle(?) XXX */
#define CPU_ARCH_ARMv6    0x7 /* ARMv6 */
#define CPU_ARCH_ARMv7    0x8 /* ARMv7 */
#define CPU_ARCH_ARMv7f   0x9 /* ARMv7 for Cortex A9 */
#define CPU_ARCH_ARMv7s   0xa /* ARMv7 for Swift */
#define CPU_ARCH_ARMv7k   0xb /* ARMv7 for Cortex A7 */

#define CPU_ARCH_ARMv8    0xc /* Subtype for CPU_TYPE_ARM64 */

#define CPU_ARCH_ARMv8E   0xd /* ARMv8.3a + Apple Private ISA Subtype for CPU_TYPE_ARM64 */

/* special code indicating we need to look somewhere else for the architecture version */
#define CPU_ARCH_EXTENDED 0xF

/* ARM Part Numbers */
/*
 * XXX: ARM Todo
 * Fill out these part numbers more completely
 */

/* ARM9 (ARMv4T architecture) */
#define CPU_PART_920T               0x920
#define CPU_PART_926EJS             0x926 /* ARM926EJ-S */

/* ARM11  (ARMv6 architecture) */
#define CPU_PART_1136JFS            0xB36 /* ARM1136JF-S or ARM1136J-S */
#define CPU_PART_1176JZFS           0xB76 /* ARM1176JZF-S */

/* G1 (ARMv7 architecture) */
#define CPU_PART_CORTEXA5           0xC05

/* M7 (ARMv7 architecture) */
#define CPU_PART_CORTEXA7           0xC07

/* H2 H3 (ARMv7 architecture) */
#define CPU_PART_CORTEXA8           0xC08

/* H4 (ARMv7 architecture) */
#define CPU_PART_CORTEXA9           0xC09

/* H5 (SWIFT architecture) */
#define CPU_PART_SWIFT              0x0

/* H6 (ARMv8 architecture) */
#define CPU_PART_CYCLONE            0x1

/* H7 (ARMv8 architecture) */
#define CPU_PART_TYPHOON            0x2

/* H7G (ARMv8 architecture) */
#define CPU_PART_TYPHOON_CAPRI      0x3

/* H8 (ARMv8 architecture) */
#define CPU_PART_TWISTER            0x4

/* H8G H8M (ARMv8 architecture) */
#define CPU_PART_TWISTER_ELBA_MALTA 0x5

/* H9 (ARMv8 architecture) */
#define CPU_PART_HURRICANE          0x6

/* H9G (ARMv8 architecture) */
#define CPU_PART_HURRICANE_MYST     0x7

/* H10 p-Core (ARMv8 architecture) */
#define CPU_PART_MONSOON            0x8

/* H10 e-Core (ARMv8 architecture) */
#define CPU_PART_MISTRAL            0x9

/* H11 p-Core (ARMv8 architecture) */
#define CPU_PART_VORTEX             0xB

/* H11 e-Core (ARMv8 architecture) */
#define CPU_PART_TEMPEST            0xC

/* M9 e-Core (ARMv8 architecture) */
#define CPU_PART_TEMPEST_M9         0xF

/* H11G p-Core (ARMv8 architecture) */
#define CPU_PART_VORTEX_ARUBA       0x10

/* H11G e-Core (ARMv8 architecture) */
#define CPU_PART_TEMPEST_ARUBA      0x11


/* Cache type identification */

/* Supported Cache Types */
typedef enum {
	CACHE_WRITE_THROUGH,
	CACHE_WRITE_BACK,
	CACHE_READ_ALLOCATION,
	CACHE_WRITE_ALLOCATION,
	CACHE_UNKNOWN
} cache_type_t;

typedef struct {
	boolean_t    c_unified;          /* unified I & D cache? */
	uint32_t     c_isize;            /* in Bytes (ARM caches can be 0.5 KB) */
	boolean_t    c_i_ppage;          /* protected page restriction for I cache
	                                  * (see B6-11 in ARM DDI 0100I document). */
	uint32_t     c_dsize;            /* in Bytes (ARM caches can be 0.5 KB) */
	boolean_t    c_d_ppage;          /* protected page restriction for I cache
	                                  * (see B6-11 in ARM DDI 0100I document). */
	cache_type_t c_type;             /* WB or WT */
	uint32_t     c_linesz;           /* number of bytes */
	uint32_t     c_assoc;            /* n-way associativity */
	uint32_t     c_l2size;           /* L2 size, if present */
	uint32_t     c_bulksize_op;      /* bulk operation size limit. 0 if disabled */
	uint32_t     c_inner_cache_size; /* inner dache size */
} cache_info_t;

typedef struct {
	uint32_t
	    RB:4, /* 3:0 - 32x64-bit media register bank supported: 0x2 */
	    SP:4, /* 7:4 - Single precision supported in VFPv3: 0x2 */
	    DP:4, /* 8:11 - Double precision supported in VFPv3: 0x2 */
	    TE:4, /* 12-15 - Only untrapped exception handling can be selected: 0x0 */
	    D:4, /* 19:16 - VFP hardware divide supported: 0x1 */
	    SR:4, /* 23:20 - VFP hardware square root supported: 0x1 */
	    SV:4, /* 27:24 - VFP short vector supported: 0x1 */
	    RM:4; /* 31:28 - All VFP rounding modes supported: 0x1 */
} arm_mvfr0_t;

typedef union {
	arm_mvfr0_t bits;
	uint32_t    value;
} arm_mvfr0_info_t;

typedef struct {
	uint32_t
	    FZ:4, /* 3:0 - Full denormal arithmetic supported for VFP: 0x1 */
	    DN:4, /* 7:4 - Propagation of NaN values supported for VFP: 0x1 */
	    LS:4, /* 11:8 - Load/store instructions supported for NEON: 0x1 */
	    I:4, /* 15:12 - Integer instructions supported for NEON: 0x1 */
	    SP:4, /* 19:16 - Single precision floating-point instructions supported for NEON: 0x1 */
	    HPFP:4, /* 23:20 - Half precision floating-point instructions supported */
	    RSVP:8; /* 31:24 - Reserved */
} arm_mvfr1_t;

typedef union {
	arm_mvfr1_t bits;
	uint32_t    value;
} arm_mvfr1_info_t;

typedef struct {
	uint32_t neon;
	uint32_t neon_hpfp;
	uint32_t neon_fp16;
} arm_mvfp_info_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern void do_cpuid(void);
extern arm_cpu_info_t *cpuid_info(void);
extern int cpuid_get_cpufamily(void);

extern void do_debugid(void);
extern arm_debug_info_t *arm_debug_info(void);

extern void do_cacheid(void);
extern cache_info_t *cache_info(void);

extern void do_mvfpid(void);
extern arm_mvfp_info_t *arm_mvfp_info(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // _MACHINE_CPUID_H_
