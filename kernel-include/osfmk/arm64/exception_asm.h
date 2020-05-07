/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
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

#ifndef _PEXPERT_ARM_BOARD_CONFIG_H
#include <pexpert/arm64/board_config.h>
#endif


/*
 * INIT_SAVED_STATE_FLAVORS
 *
 * Initializes the saved state flavors of a new saved state structure
 *  arg0 - saved state pointer
 *  arg1 - 32-bit scratch reg
 *  arg2 - 32-bit scratch reg
 */
.macro INIT_SAVED_STATE_FLAVORS
mov		$1, ARM_SAVED_STATE64                                   // Set saved state to 64-bit flavor
mov		$2, ARM_SAVED_STATE64_COUNT
stp		$1, $2, [$0, SS_FLAVOR]
mov		$1, ARM_NEON_SAVED_STATE64                              // Set neon state to 64-bit flavor
str		$1, [$0, NS_FLAVOR]
mov		$1, ARM_NEON_SAVED_STATE64_COUNT
str		$1, [$0, NS_COUNT]
.endmacro

/*
 * SPILL_REGISTERS
 *
 * Spills the current set of registers (excluding x0, x1, sp, fp) to the specified
 * save area.
 *   x0 - Address of the save area
 */

.macro SPILL_REGISTERS
stp		x2, x3, [x0, SS64_X2]                                   // Save remaining GPRs
stp		x4, x5, [x0, SS64_X4]
stp		x6, x7, [x0, SS64_X6]
stp		x8, x9, [x0, SS64_X8]
stp		x10, x11, [x0, SS64_X10]
stp		x12, x13, [x0, SS64_X12]
stp		x14, x15, [x0, SS64_X14]
stp		x16, x17, [x0, SS64_X16]
stp		x18, x19, [x0, SS64_X18]
stp		x20, x21, [x0, SS64_X20]
stp		x22, x23, [x0, SS64_X22]
stp		x24, x25, [x0, SS64_X24]
stp		x26, x27, [x0, SS64_X26]
str		x28, [x0, SS64_X28]

/* Save arm_neon_saved_state64 */

stp		q0, q1, [x0, NS64_Q0]
stp		q2, q3, [x0, NS64_Q2]
stp		q4, q5, [x0, NS64_Q4]
stp		q6, q7, [x0, NS64_Q6]
stp		q8, q9, [x0, NS64_Q8]
stp		q10, q11, [x0, NS64_Q10]
stp		q12, q13, [x0, NS64_Q12]
stp		q14, q15, [x0, NS64_Q14]
stp		q16, q17, [x0, NS64_Q16]
stp		q18, q19, [x0, NS64_Q18]
stp		q20, q21, [x0, NS64_Q20]
stp		q22, q23, [x0, NS64_Q22]
stp		q24, q25, [x0, NS64_Q24]
stp		q26, q27, [x0, NS64_Q26]
stp		q28, q29, [x0, NS64_Q28]
stp		q30, q31, [x0, NS64_Q30]

mrs		lr, ELR_EL1                                                     // Get exception link register
mrs		x23, SPSR_EL1                                                   // Load CPSR into var reg x23
mrs		x24, FPSR
mrs		x25, FPCR

#if defined(HAS_APPLE_PAC)
/* Save x1 and LR to preserve across call */
mov		x21, x1
mov		x20, lr

/*
 * Create thread state signature
 *
 * Arg0: The ARM context pointer
 * Arg1: The PC value to sign
 * Arg2: The CPSR value to sign
 * Arg3: The LR value to sign
 * Arg4: The X16 value to sign
 * Arg5: The X17 value to sign
 */
mov		x1, lr
mov		w2, w23
ldr		x3, [x0, SS64_LR]
mov		x4, x16
mov		x5, x17
bl		_ml_sign_thread_state

mov		lr, x20
mov		x1, x21
#endif /* defined(HAS_APPLE_PAC) */

str		lr, [x0, SS64_PC]                                               // Save ELR to PCB
str		w23, [x0, SS64_CPSR]                                    // Save CPSR to PCB
str		w24, [x0, NS64_FPSR]
str		w25, [x0, NS64_FPCR]

mrs		x20, FAR_EL1
mrs		x21, ESR_EL1

str		x20, [x0, SS64_FAR]
str		w21, [x0, SS64_ESR]
.endmacro

.macro DEADLOOP
b	.
.endmacro
