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

#ifndef _ARM_CPUID_INTERNAL_H_
#define _ARM_CPUID_INTERNAL_H_

void machine_do_debugid(void);
arm_debug_info_t *machine_arm_debug_info(void);

void machine_do_mvfpid(void);
arm_mvfp_info_t *machine_arm_mvfp_info(void);

uint32_t machine_read_midr(void);
uint32_t machine_read_clidr(void);
uint32_t machine_read_ccsidr(void);

typedef enum {
	CSSELR_L1 = 0x0,
	CSSELR_L2 = 0x2,
	CSSELR_L3 = 0x4
} csselr_cache_level;

typedef enum {
	CSSELR_DATA_UNIFIED = 0x0,
	CSSELR_INSTR = 0x1
} csselr_cache_type;

void machine_write_csselr(csselr_cache_level level, csselr_cache_type type);

#endif /* _ARM_CPUID_INTERNAL_H_ */
