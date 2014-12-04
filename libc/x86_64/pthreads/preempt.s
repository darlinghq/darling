/*
 * Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#include <machine/cpu_capabilities.h>
#include <mach/i386/syscall_sw.h>

/* Subroutine to make a preempt syscall.  Called when we notice %ebx is
 * nonzero after returning from a PFZ subroutine.  Not in PFZ.
 *
 * All registers preserved (but does clear the %ebx preemption flag).
 */
	.align 2
	.private_extern _preempt
_preempt:
	pushq	%rax
	pushq	%rcx
	pushq	%r11
	movl	$(SYSCALL_CONSTRUCT_MACH(58)),%eax	/* 58 = pfz_exit */
	xorl	%ebx,%ebx
	syscall
	popq	%r11
	popq	%rcx
	popq	%rax
	ret

/* Subroutine to back off if we cannot get the spinlock.  Called
 * after a few attempts inline in the PFZ subroutines.  This code is
 * not in the PFZ.
 *	%rdi = ptr to queue head structure
 *	%ebx = preemption flag (nonzero if preemption pending)
 * Uses: %rax.
 */
	.align 2
	.private_extern _backoff
_backoff:
	testl	%ebx,%ebx	// does kernel want to preempt us?
	jz	1f		// no
	call    _preempt
1:
	pause			// SMT-friendly backoff
	cmpl	$0,16(%rdi)	// sniff the lockword
	jnz	1b		// loop if still taken
	ret			// lockword is free, so reenter PFZ
