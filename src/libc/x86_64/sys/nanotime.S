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

/*
 * 64-bit version _mach_absolute_time.  We return the 64-bit nanotime in %rax,
 */
	.globl	_mach_absolute_time
_mach_absolute_time:
	pushq	%rbp				// set up a frame for backtraces
	movq	%rsp,%rbp
	movq	$(_COMM_PAGE_TIME_DATA_START),%rsi
1:
	movl	_NT_GENERATION(%rsi),%r8d	// get generation
	testl	%r8d,%r8d			// if 0, data is being changed...
	jz	1b				// ...so loop until stable
	lfence
	rdtsc					// edx:eax := tsc
	lfence
	shlq	$32,%rdx			// rax := ((edx << 32) | eax), ie 64-bit tsc
	orq	%rdx,%rax
	subq	_NT_TSC_BASE(%rsi), %rax	// rax := (tsc - base_tsc)
	movl	_NT_SCALE(%rsi),%ecx
	mulq	%rcx				// rdx:rax := (tsc - base_tsc) * scale
	shrdq	$32,%rdx,%rax			// _COMM_PAGE_NT_SHIFT is always 32
	addq	_NT_NS_BASE(%rsi),%rax		// (((tsc - base_tsc) * scale) >> 32) + ns_base
	
	cmpl	_NT_GENERATION(%rsi),%r8d	// did the data change during computation?
	jne	1b
	popq	%rbp
	ret
