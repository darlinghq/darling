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

#if defined(__i386__)

/* return mach_absolute_time in %edx:%eax
 *
 * The algorithm we use is:
 *
 *	ns = ((((rdtsc - rnt_tsc_base)<<rnt_shift)*rnt_tsc_scale) / 2**32) + rnt_ns_base;
 *
 * rnt_shift, a constant computed during initialization, is the smallest value for which:
 *
 *  (tscFreq << rnt_shift) > SLOW_TSC_THRESHOLD
 *
 * Where SLOW_TSC_THRESHOLD is about 10e9.  Since most processor's tscFreq is greater
 * than 1GHz, rnt_shift is usually 0.  rnt_tsc_scale is also a 32-bit constant:
 *
 *	rnt_tsc_scale = (10e9 * 2**32) / (tscFreq << rnt_shift);
 */

	.globl _mach_absolute_time
_mach_absolute_time:
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
    
    /*
     * Prior to supporting "slow" processors, xnu always set _NT_SHIFT to 32.
     * Now it defaults to 0, unless the processor is slow.  The shifts
     * below implicitly mask the count down to 5 bits, handling either default.
     */
	movl    _COMM_PAGE_NT_SHIFT,%ecx
	shldl   %cl,%eax,%edx			/* shift %edx left, filling in from %eax */
	shll    %cl,%eax			/* finish shifting %edx:%eax left by _COMM_PAGE_NT_SHIFT bits */

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

#elif defined(__x86_64__)

/*
 * 64-bit version _mach_absolute_time.  We return the 64-bit nanotime in %rax.
 *
 * The algorithm we use is:
 *
 *	ns = ((((rdtsc - rnt_tsc_base)<<rnt_shift)*rnt_tsc_scale) / 2**32) + rnt_ns_base;
 *
 * rnt_shift, a constant computed during initialization, is the smallest value for which:
 *
 *	tscFreq << rnt_shift) > SLOW_TSC_THRESHOLD
 *
 * Where SLOW_TSC_THRESHOLD is about 10e9.  Since most processor's tscFreqs are greater
 * than 1GHz, rnt_shift is usually 0.  rnt_tsc_scale is also a 32-bit constant:
 *
 *	rnt_tsc_scale = (10e9 * 2**32) / (tscFreq << rnt_shift);
 *
 */
	.globl	_mach_absolute_time
_mach_absolute_time:
	pushq	%rbp				// set up a frame for backtraces
	movq	%rsp,%rbp
	movq	$(_COMM_PAGE_TIME_DATA_START),%rsi
1:
	movl	_NT_GENERATION(%rsi),%r8d	// get generation
	testl	%r8d,%r8d			// if 0, data is being changed...
	jz      1b				// ...so loop until stable
	lfence
	rdtsc					// edx:eax := tsc
	lfence
	shlq	$32,%rdx			// rax := ((edx << 32) | eax), ie 64-bit tsc
	orq     %rdx,%rax

    /*
     * Prior to supporting "slow" processors, xnu always set _NT_SHIFT to 32.
     * Now it defaults to 0, unless the processor is slow.  In order to maintain
     * compatibility with both old and new versions of xnu, we mask the shift
     * down to 0x1F, which maps the old default (32) into the new default (0).
     */
	movl    _NT_SHIFT(%rsi),%ecx
	andl    $0x1F,%ecx			// *** remove this line once 10.9 is GM ***
	subq	_NT_TSC_BASE(%rsi), %rax	// rax := (tsc - base_tsc)
	shlq    %cl,%rax			// rax := (tsc - base_tsc) << NT_SHIFT
	movl	_NT_SCALE(%rsi),%ecx
	mulq	%rcx				// rdx:rax := ((tsc - base_tsc)<<shift) * scale
	shrdq	$32,%rdx,%rax			// divide by 2**32
	addq	_NT_NS_BASE(%rsi),%rax		// (((tsc - base_tsc) * scale) >> 32) + ns_base
	
	cmpl	_NT_GENERATION(%rsi),%r8d	// did the data change during computation?
	jne     1b
	popq	%rbp
	ret

#elif defined(__arm__)

#include <mach/arm/syscall_sw.h>

/*
 * If userspace access to the timebase is supported (indicated through the commpage),
 * directly reads the timebase and uses it and the current timebase offset (also in
 * the commpage, and updated whenever the system wakes from sleep) to construct the
 * current time value; otherwise, traps to the kernel to handle this.
 *
 * If we do this in user mode, there are two cases where we may need to redrive the
 * read.  We do 3 reads (high-low-high) to the timebase, because we only have a
 * 32-bit interface to it (despite the use of mrrc).  If the high bits change, we
 * need to reread the register (as our returned value could otherwise be off by
 * 2^32 mach absolute time units).
 *
 * We do two reads of the offset, before and after the register reads.  If the offset
 * changes, we have gone to sleep in the midst of doing a read.  This case should be
 * exceedingly rare, but could result in a terribly inaccurate result, so we need
 * to get a fresh timebase value.
 */
	.text
	.align 2
	.globl _mach_absolute_time
_mach_absolute_time:
	movw	ip, #((_COMM_PAGE_TIMEBASE_OFFSET) & 0x0000FFFF)
	movt	ip, #(((_COMM_PAGE_TIMEBASE_OFFSET) >> 16) & 0x0000FFFF)
	ldrb	r0, [ip, #((_COMM_PAGE_USER_TIMEBASE) - (_COMM_PAGE_TIMEBASE_OFFSET))]
	cmp	r0, #USER_TIMEBASE_NONE		// Are userspace reads supported?
	beq	_mach_absolute_time_kernel	// If not, go to the kernel
	isb					// Prevent speculation on CNTPCT across calls
						// (see ARMV7C.b section B8.1.2, ARMv8 section D6.1.2)
	push	{r4, r5, r7, lr}		// Push a frame
	add	r7, sp, #8
L_mach_absolute_time_user:
	ldr	r4, [ip]			// Load offset low bits
	ldr	r5, [ip, #4]			// Load offset high bits
	mrrc	p15, 0, r3, r1, c14		// Read timebase high to r1
	mrrc	p15, 0, r0, r3, c14		// Read timebase low to r0
	mrrc	p15, 0, r3, r2, c14		// Read timebase high to r2
	cmp	r1, r2				// Did the high bits change?
	bne	L_mach_absolute_time_user	// Loop if timebase high changed
	ldr	r2, [ip]			// Load offset low bits
	ldr	r3, [ip, #4]			// Load offset high bits
	eor	r4, r2				// Compare our offset values...
	eor	r5, r3
	orrs	r5, r4
	bne	L_mach_absolute_time_user	// If they changed, try again
	adds	r0, r0, r2			// Construct mach_absolute_time
	adcs	r1, r1, r3
	pop	{r4, r5, r7, pc}		// Pop the frame

	.text
	.align 2
	.globl _mach_absolute_time_kernel
_mach_absolute_time_kernel:
	mov	r12, #-3			// Load the magic MAT number
	swi	#SWI_SYSCALL
	bx	lr

	.text
	.align 2
	.globl _mach_continuous_time_kernel
_mach_continuous_time_kernel:
	mov	r12, #-4			// Load the magic MCT number
	swi	#SWI_SYSCALL
	bx	lr

#elif defined(__arm64__)

#include <mach/arm/syscall_sw.h>

/*
 * If userspace access to the timebase is supported (indicated through the commpage),
 * directly reads the timebase and uses it and the current timebase offset (also in
 * the commpage, and updated whenever the system wakes from sleep) to construct the
 * current time value; otherwise, traps to the kernel to handle this.
 *
 * If we do this in user mode, we do two reads of the offset, before and after we
 * read the register.  If the offset changes, we have gone to sleep in the midst of
 * doing a read.  This case should be exceedingly rare, but could result in a terribly
 * inaccurate result, so we need to get a fresh timebase value.
 */
	.text
	.align 2
	.globl _mach_absolute_time
_mach_absolute_time:
	movk	x3, #(((_COMM_PAGE_TIMEBASE_OFFSET) >> 48) & 0x000000000000FFFF), lsl #48
	movk	x3, #(((_COMM_PAGE_TIMEBASE_OFFSET) >> 32) & 0x000000000000FFFF), lsl #32
	movk	x3, #(((_COMM_PAGE_TIMEBASE_OFFSET) >> 16) & 0x000000000000FFFF), lsl #16
	movk	x3, #((_COMM_PAGE_TIMEBASE_OFFSET) & 0x000000000000FFFF)
	ldrb	w2, [x3, #((_COMM_PAGE_USER_TIMEBASE) - (_COMM_PAGE_TIMEBASE_OFFSET))]
	cmp	x2, #USER_TIMEBASE_NONE		// Are userspace reads supported?
	b.eq	_mach_absolute_time_kernel	// If not, go to the kernel
	isb					// Prevent speculation on CNTPCT across calls
						// (see ARMV7C.b section B8.1.2, ARMv8 section D6.1.2)
L_mach_absolute_time_user:
	ldr	x1, [x3]			// Load the offset
	mrs	x0, CNTPCT_EL0			// Read the timebase
	ldr	x2, [x3]			// Load the offset
	cmp	x1, x2				// Compare our offset values...
	b.ne	L_mach_absolute_time_user	// If they changed, try again
	add	x0, x0, x1			// Construct mach_absolute_time
	ret



	.text
	.align 2
	.globl _mach_absolute_time_kernel
_mach_absolute_time_kernel:
	mov	w16, #-3			// Load the magic MAT number
	svc	#SWI_SYSCALL
	ret

	.text
	.align 2
	.globl _mach_continuous_time_kernel
_mach_continuous_time_kernel:
	mov	w16, #-4			// Load the magic MCT number
	svc	#SWI_SYSCALL
	ret

#else
#error Unsupported architecture
#endif
