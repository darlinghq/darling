/*
 * Copyright (c) 2019 Apple Inc. All rights reserved.
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

#include <pexpert/arm64/board_config.h>
#include "assym.s"

#ifndef __ASSEMBLER__
#error "This header should only be used in .s files"
#endif

/**
 * Loads the following values from the thread_kernel_state pointer in x0:
 *
 * x1: $x0->ss_64.pc
 * w2: $x0->ss_64.cpsr
 * x16: $x0->ss_64.x16
 * x17: $x0->ss_64.x17
 * lr: $x0->ss_64.lr
 *
 * On CPUs with PAC support, this macro will auth the above values with ml_check_signed_state().
 *
 * arg0 - scratch register 1
 * arg1 - scratch register 2
 * arg2 - scratch register 3
 * arg3 - scratch register 4
 * arg4 - scratch register 5
 */
/* BEGIN IGNORE CODESTYLE */
.macro  AUTH_THREAD_STATE_IN_X0
	ldr		x1, [x0, SS64_PC]
	ldr		w2, [x0, SS64_CPSR]
	ldp		x16, x17, [x0, SS64_X16]

#if defined(HAS_APPLE_PAC)
	// Save x3-x5 to preserve across call
	mov		$2, x3
	mov		$3, x4
	mov		$4, x5

	/*
	* Arg0: The ARM context pointer (already in x0)
	* Arg1: PC to check (loaded above)
	* Arg2: CPSR to check (loaded above)
	* Arg3: the LR to check
	*
	* Stash saved state PC and CPSR in other registers to avoid reloading potentially unauthed
	* values from memory.  (ml_check_signed_state will clobber x1 and x2.)
	*/
	mov		$0, x1
	mov		$1, x2
	ldr		x3, [x0, SS64_LR]
	mov		x4, x16
	mov		x5, x17
	bl		EXT(ml_check_signed_state)
	mov		x1, $0
	mov		x2, $1

	// LR was already loaded/authed earlier, if we reload it we might be loading a potentially unauthed value
	mov		lr, x3
	mov		x3, $2
	mov		x4, $3
	mov		x5, $4
#else
	ldr		lr, [x0, SS64_LR]
#endif /* defined(HAS_APPLE_PAC) */
.endmacro
/* END IGNORE CODESTYLE */

/* vim: set ft=asm: */
