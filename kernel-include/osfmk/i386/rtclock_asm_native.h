/*
 * Copyright (c) 2010-2012 Apple Inc. All rights reserved.
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
 * @APPLE_FREE_COPYRIGHT@
 */
/*
 *	File:		rtclock_asm_native.h
 *	Purpose:	Native routines for reading nanotime
 */

#ifndef _PAL_RTCLOCK_ASM_NATIVE_H_
#define _PAL_RTCLOCK_ASM_NATIVE_H_

/*
 * Assembly snippet included in exception handlers and rtc_nanotime_read()
 *
 *
 * Warning!  There are several copies of this code in the trampolines found in
 * osfmk/x86_64/idt64.s, coming from the various TIMER macros in rtclock_asm.h.
 * They're all kept in sync by using the RTC_NANOTIME_READ() macro.
 *
 * The algorithm we use is:
 *
 *	ns = ((((rdtsc - rnt_tsc_base)<<rnt_shift)*rnt_tsc_scale) / 2**32) + rnt_ns_base;
 *
 * rnt_shift, a constant computed during initialization, is the smallest value for which:
 *
 *	(tscFreq << rnt_shift) > SLOW_TSC_THRESHOLD
 *
 * Where SLOW_TSC_THRESHOLD is about 10e9.  Since most processor's tscFreqs are greater
 * than 1GHz, rnt_shift is usually 0.  rnt_tsc_scale is also a 32-bit constant:
 *
 *	rnt_tsc_scale = (10e9 * 2**32) / (tscFreq << rnt_shift);
 *
 * %rdi points to nanotime info struct.
 * %rax returns nanotime
 */
#define PAL_RTC_NANOTIME_READ_FAST()					  \
0:	movl	RNT_GENERATION(%rdi),%esi				; \
	test        %esi,%esi		/* info updating? */		; \
        jz        0b			/* - wait if so */		; \
	lfence								; \
	rdtsc								; \
	shlq	$32,%rdx						; \
	movl    RNT_SHIFT(%rdi),%ecx					; \
	orq	%rdx,%rax			/* %rax := tsc */	; \
	subq	RNT_TSC_BASE(%rdi),%rax		/* tsc - tsc_base */	; \
	shlq    %cl,%rax						; \
	movl	RNT_SCALE(%rdi),%ecx					; \
	mulq	%rcx				/* delta * scale */	; \
	shrdq	$32,%rdx,%rax			/* %rdx:%rax >>= 32 */	; \
	addq	RNT_NS_BASE(%rdi),%rax		/* add ns_base */	; \
	cmpl	RNT_GENERATION(%rdi),%esi	/* repeat if changed */ ; \
	jne	0b

#define PAL_RTC_NANOTIME_READ_NOBARRIER()					  \
0:	movl	RNT_GENERATION(%rdi),%esi				; \
	test        %esi,%esi		/* info updating? */		; \
        jz        0b			/* - wait if so */		; \
	rdtsc								; \
	shlq	$32,%rdx						; \
	movl    RNT_SHIFT(%rdi),%ecx					; \
	orq	%rdx,%rax			/* %rax := tsc */	; \
	subq	RNT_TSC_BASE(%rdi),%rax		/* tsc - tsc_base */	; \
	shlq    %cl,%rax						; \
	movl	RNT_SCALE(%rdi),%ecx					; \
	mulq	%rcx				/* delta * scale */	; \
	shrdq	$32,%rdx,%rax			/* %rdx:%rax >>= 32 */	; \
	addq	RNT_NS_BASE(%rdi),%rax		/* add ns_base */	; \
	cmpl	RNT_GENERATION(%rdi),%esi	/* repeat if changed */ ; \
	jne	0b

#endif /* _PAL_RTCLOCK_ASM_NATIVE_H_ */
