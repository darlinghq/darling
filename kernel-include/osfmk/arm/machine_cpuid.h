/*
 * Copyright (c) 2017 Apple Inc. All rights reserved.
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
#ifndef _ARM_MACHINE_CPUID_H_
#define _ARM_MACHINE_CPUID_H_

/* CPU feature identification */

typedef struct {
	uint32_t        arm_32bit_isa   : 4,
	    arm_thumb_ver   : 4,
	    arm_jazelle             : 4,
	    arm_thumb2              : 4,
	    reserved                : 16;
} arm_feature_bits_t;

typedef union {
	arm_feature_bits_t      field;
	uint32_t                        value;
} arm_feature0_reg_t;

// Register 0, subtype 21: Instruction Set Features #1
typedef struct{
	uint32_t endianness_support     : 4;
	uint32_t exception_1_support    : 4;
	uint32_t exception_2_support    : 4;
	uint32_t sign_zero_ext_support  : 4;
	uint32_t if_then_support        : 4;
	uint32_t immediate_support      : 4;
	uint32_t interworking_support   : 4;
	uint32_t jazelle_support        : 4;
}
syscp_ID_instructions_feat_1_reg;

typedef union {
	uint32_t value;
	syscp_ID_instructions_feat_1_reg field;
} arm_isa_feat1_reg;

arm_isa_feat1_reg machine_read_isa_feat1(void);

/* Debug identification */

/* ID_DFR0 */
typedef union {
	struct {
		uint32_t    coprocessor_core_debug      : 4,
		    coprocessor_secure_debug    : 4,
		    memory_mapped_core_debug    : 4,
		    coprocessor_trace_debug     : 4,
		    memory_mapped_trace_debug   : 4,
		    microcontroller_debug       : 4;
	} debug_feature;
	uint32_t value;
} arm_cpuid_id_dfr0;

/* DBGDIDR */
typedef union {
	struct {
		uint32_t    revision                    : 4,
		    variant                     : 4,
		: 4,
		    se_imp                      : 1,
		    pcsr_imp                    : 1,
		    nsuhd_imp                   : 1,
		: 1,
		    version                     : 4,
		    ctx_cmps                    : 4,
		    brps                        : 4,
		    wrps                        : 4;
	} debug_id;
	uint32_t value;
} arm_debug_dbgdidr;

typedef struct {
	boolean_t               memory_mapped_core_debug;
	boolean_t               coprocessor_core_debug;
	uint32_t                num_watchpoint_pairs;
	uint32_t                num_breakpoint_pairs;
} arm_debug_info_t;

#endif /* _ARM_MACHINE_CPUID_H_ */
