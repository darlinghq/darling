/*
 * Copyright (c) 2010 Apple Inc. All rights reserved.
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
 * nonzero after returning from a PFZ subroutine.
 * When we enter kernel:
 *	%edx = return address
 *	%ecx = stack ptr
 * Destroys %eax, %ecx, and %edx.
 */
	.align	4
	.private_extern _preempt
_preempt:
	popl	%edx		// get return address
	movl	%esp,%ecx	// save stack ptr here
	movl	$(-58),%eax	/* 58 = pfz_exit */
	xorl	%ebx,%ebx	// clear "preemption pending" flag
	sysenter


/* Subroutine to back off if we cannot get the spinlock.  Called
 * after a few attempts inline in the PFZ subroutines.  This code is
 * not in the PFZ.
 *	%edi = ptr to queue head structure
 *	%ebx = preemption flag (nonzero if preemption pending)
 * Destroys %eax.
 */

	.align 	4
	.private_extern _backoff
_backoff:
	testl	%ebx,%ebx	// does kernel want to preempt us?
	jz	1f		// no
	xorl	%ebx,%ebx	// yes, clear flag
	pushl	%edx		// preserve regs used by preempt syscall
	pushl	%ecx
	call	_preempt
	popl	%ecx
	popl	%edx
1:
	pause			// SMT-friendly backoff
	cmpl	$0,8(%edi)	// sniff the lockword
	jnz	1b		// loop if still taken
	ret			// lockword is free, so reenter PFZ
