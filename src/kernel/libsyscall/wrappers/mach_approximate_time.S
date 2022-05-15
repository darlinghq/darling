/*
 * Copyright (c) 2003-2007 Apple Inc. All rights reserved.
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

#include <sys/appleapiopts.h>
#include <machine/cpu_capabilities.h>

#if defined(__arm__)

	.text
	.align	2
	.globl	_mach_approximate_time
_mach_approximate_time:

	movw	r0, #((_COMM_PAGE_APPROX_TIME_SUPPORTED>>0)&0x0FFFF)
	movt	r0, #((_COMM_PAGE_APPROX_TIME_SUPPORTED>>16)&0x0FFFF)
	ldrb	r0, [r0]		// load COMM_PAGE_APPROX_TIME_SUPPORTED
	cmp	r0, #1			// check if approx time is supported
        
        bne      _mach_absolute_time	// if not supported, fall through to
					//   absolute_time

	movw	r2, #((_COMM_PAGE_APPROX_TIME>>0)&0x0FFFF)
	movt	r2, #((_COMM_PAGE_APPROX_TIME>>16)&0x0FFFF)

        // at this point, r2->COMM_PAGE_APPROX_TIME, which is a 64-bit value.
        // Since this is a 32-bit architecture, and the commpage is readonly,
        // there is no "guaranteed" atomic way to read all 64-bits with
        // hardware protection. Even the ldrd instruction is not guaranteed to
        // complete atomically. The solution is to use a 32-bit high/low/high
        // read with a consistency check on the high bits. To further
        // complicate things, reading the same location in memory back to back
        // could trigger a predictive read, which would defeat the purpose of
        // doing the consistency check so we insert a data memory barrier to
        // prevent this.
_consistency_check:
        ldr     r1, [r2,#4]		// load high
        ldr     r0, [r2]		// load low
        dsb				// avoid predictive reads that could
					//   be invalid if interrupted
        ldr     r3, [r2,#4]		// load high again
        cmp     r1, r3			// check that high1 == high2
	bne	_consistency_check	// try again if not
        bx      lr

#elif defined(__i386__)

	.text
	.align	4, 0x90
	.globl	_mach_approximate_time
_mach_approximate_time:
	pushl	%ebp				// push a frame
	movl	%esp, %ebp
	cmpb	$0, _COMM_PAGE_APPROX_TIME_SUPPORTED // check commpage
						//   "is supported" flag.
	je	_approx_time_not_supported	// if zero, fall through to
						//   absolute_time
        // note the comment above for 32-bit ARM applies equally to 32-bit i386.
	.align	4, 0x90
_approx_time_consistency_check:
	movl	_COMM_PAGE_APPROX_TIME+4, %edx	// load high
	movl	_COMM_PAGE_APPROX_TIME, %eax	// load low
        lfence					// avoid predictive reads that
						//   could be invalid if
						//   interrupted 
	cmpl	_COMM_PAGE_APPROX_TIME+4, %edx  // load high and check if equal
						// to the first read
	jne	_approx_time_consistency_check	// if not, try again.
	popl	%ebp
	ret

_approx_time_not_supported:
	popl	%ebp
	jmp	_mach_absolute_time

#endif
