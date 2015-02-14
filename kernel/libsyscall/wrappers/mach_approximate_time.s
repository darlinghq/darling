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

#if   defined(__i386__)

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
