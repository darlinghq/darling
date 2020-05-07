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
#ifndef _ARM64_MACHINE_CPUID_H_
#define _ARM64_MACHINE_CPUID_H_

typedef struct {
	uint64_t        el0_not_implemented             : 1,
	    el0_aarch64_only                : 1,
	    el0_aarch32_and_64              : 1,
	    el1_not_implemented             : 1,
	    el1_aarch64_only                : 1,
	    el1_aarch32_and_64              : 1,
	    el2_not_implemented             : 1,
	    el2_aarch64_only                : 1,
	    el2_aarch32_and_64              : 1,
	    el3_not_implemented             : 1,
	    el3_aarch64_only                : 1,
	    el3_aarch32_and_64              : 1,
	    reserved                                : 52;
} arm_feature_bits_t;

/* Debug identification */

/* ID_AA64DFR0_EL1 */
typedef union {
	struct {
		uint64_t debug_arch_version             : 4,
		    trace_extn_version             : 4,
		    perf_extn_version              : 4,
		    brps                                   : 4,
		    reserved0                              : 4,
		    wrps                                   : 4,
		    reserved1                              : 4,
		    ctx_cmps                               : 4,
		    reserved32                             : 32;
	} debug_feature;
	uint64_t value;
} arm_cpuid_id_aa64dfr0_el1;

typedef struct {
	uint32_t        num_watchpoint_pairs;
	uint32_t        num_breakpoint_pairs;
} arm_debug_info_t;

#endif /* _MACHINE_CPUID_H_ */
