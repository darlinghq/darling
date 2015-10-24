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
#include <platfunc.h>

#if defined(VARIANT_DYLD)
/* For dyld, we need to decide upon call whether to jump to fast or slow */
	.globl _mach_absolute_time
	.align 2, 0x90
_mach_absolute_time:
	movl	_COMM_PAGE_CPU_CAPABILITIES, %eax
	andl	$(kSlow), %eax
	jnz	PLATFUNC_VARIANT_NAME(mach_absolute_time, slow)
	jmp	PLATFUNC_VARIANT_NAME(mach_absolute_time, fast)
#endif

/* return mach_absolute_time in %edx:%eax */

PLATFUNC_FUNCTION_START(mach_absolute_time, fast, 32, 4)
	.private_extern	_mach_absolute_time_direct
_mach_absolute_time_direct:
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%esi
	pushl	%ebx

0:
	movl	_COMM_PAGE_NT_GENERATION,%esi	/* get generation (0 if being changed) */
	testl	%esi,%esi			/* if being updated, loop until stable */
	jz	0b

	lfence
	rdtsc					/* get TSC in %edx:%eax */
	lfence

	subl	_COMM_PAGE_NT_TSC_BASE,%eax
	sbbl	_COMM_PAGE_NT_TSC_BASE+4,%edx

	movl	_COMM_PAGE_NT_SCALE,%ecx

	movl	%edx,%ebx
	mull	%ecx
	movl	%ebx,%eax
	movl	%edx,%ebx
	mull	%ecx
	addl	%ebx,%eax
	adcl	$0,%edx

	addl	_COMM_PAGE_NT_NS_BASE,%eax
	adcl	_COMM_PAGE_NT_NS_BASE+4,%edx

	cmpl	_COMM_PAGE_NT_GENERATION,%esi	/* have the parameters changed? */
	jne	0b				/* yes, loop until stable */

	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
PLATFUNC_DESCRIPTOR(mach_absolute_time,fast,0,kSlow)


/* mach_absolute_time routine for machines slower than ~1Gz (SLOW_TSC_THRESHOLD) */
PLATFUNC_FUNCTION_START(mach_absolute_time, slow, 32, 4)
	push	%ebp
	mov	%esp,%ebp
	push	%esi
	push	%edi
	push	%ebx

0:
	movl	_COMM_PAGE_NT_GENERATION,%esi
	testl	%esi,%esi			/* if generation is 0, data being changed */
	jz	0b				/* so loop until stable */

	lfence
	rdtsc					/* get TSC in %edx:%eax */
	lfence
	subl	_COMM_PAGE_NT_TSC_BASE,%eax
	sbbl	_COMM_PAGE_NT_TSC_BASE+4,%edx

	pushl	%esi				/* save generation */
	/*
	 * Do the math to convert tsc ticks to nanoseconds.  We first
	 * do long multiply of 1 billion times the tsc.  Then we do
	 * long division by the tsc frequency
	 */
	mov	$1000000000, %ecx		/* number of nanoseconds in a second */
	mov	%edx, %ebx
	mul	%ecx
	mov	%edx, %edi
	mov	%eax, %esi
	mov	%ebx, %eax
	mul	%ecx
	add	%edi, %eax
	adc	$0, %edx			/* result in edx:eax:esi */
	mov	%eax, %edi
	mov	_COMM_PAGE_NT_SHIFT,%ecx	/* overloaded as the low 32 tscFreq */
	xor	%eax, %eax
	xchg	%edx, %eax
	div	%ecx
	xor	%eax, %eax
	mov	%edi, %eax
	div	%ecx
	mov	%eax, %ebx
	mov	%esi, %eax
	div	%ecx
	mov	%ebx, %edx			/* result in edx:eax */
	popl	%esi				/* recover generation */

	add	_COMM_PAGE_NT_NS_BASE,%eax
	adc	_COMM_PAGE_NT_NS_BASE+4,%edx

	cmpl	_COMM_PAGE_NT_GENERATION,%esi	/* have the parameters changed? */
	jne	0b				/* yes, loop until stable */

	pop	%ebx
	pop	%edi
	pop	%esi
	pop	%ebp
	ret					/* result in edx:eax */
PLATFUNC_DESCRIPTOR(mach_absolute_time,slow,kSlow,0)
