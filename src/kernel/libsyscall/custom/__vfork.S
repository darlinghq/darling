/*
 * Copyright (c) 1999-2007 Apple Inc. All rights reserved.
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
/* Copyright (c) 1998 Apple Computer, Inc.  All rights reserved.
 *
 *	File:	libc/ppc/sys/vfork.s
 *
 * HISTORY
 * 23-Jun-1998	Umesh Vaishampayan (umeshv@apple.com)
 *	Created from fork.s
 *
 */

#include "SYS.h"

#if defined(__i386__)

#if defined(__DYNAMIC__)
#define GET_CURRENT_PID	PICIFY(__current_pid)

        NON_LAZY_STUB(__current_pid)
#define __current_pid	(%edx)
#else
#define GET_CURRENT_PID
#endif

/*
 * If __current_pid >= 0, we want to put a -1 in there
 * otherwise we just decrement it
 */

LEAF(___vfork, 0)
	GET_CURRENT_PID
	movl		__current_pid, %eax
0:
	xorl		%ecx, %ecx
	testl		%eax, %eax
	cmovs		%eax, %ecx
	decl		%ecx
	lock
	cmpxchgl	%ecx, __current_pid
	jne		0b
	popl		%ecx
	movl		$(SYS_vfork), %eax	// code for vfork -> eax
	UNIX_SYSCALL_TRAP			// do the system call
	jnb		L1                     	// jump if CF==0
	GET_CURRENT_PID
	lock
	incl		__current_pid
	pushl		%ecx
	BRANCH_EXTERN(tramp_cerror)

L1:
	testl		%edx, %edx		// CF=OF=0,  ZF set if zero result
	jz		L2			// parent, since r1 == 0 in parent, 1 in child
	xorl		%eax, %eax		// zero eax
	jmp		*%ecx

L2:
	GET_CURRENT_PID
	lock
	incl		__current_pid
	jmp		*%ecx

#elif defined(__x86_64__)

/*
 * If __current_pid >= 0, we want to put a -1 in there
 * otherwise we just decrement it
 */

LEAF(___vfork, 0)
	movq		__current_pid@GOTPCREL(%rip), %rax
	movl		(%rax), %eax
0:
	xorl		%ecx, %ecx
	testl		%eax, %eax
	cmovs		%eax, %ecx
	subl		$1, %ecx
	movq		__current_pid@GOTPCREL(%rip), %rdx
	lock
	cmpxchgl	%ecx, (%rdx)
	jne		0b
	popq		%rdi			// return address in %rdi
	movq		$ SYSCALL_CONSTRUCT_UNIX(SYS_vfork), %rax	// code for vfork -> rax
	UNIX_SYSCALL_TRAP			// do the system call
	jnb		L1			// jump if CF==0
	pushq		%rdi			// put return address back on stack for cerror
	movq		__current_pid@GOTPCREL(%rip), %rcx
	lock
	addl		$1, (%rcx)
	movq		%rax, %rdi
	BRANCH_EXTERN(_cerror)

L1:
	testl		%edx, %edx		// CF=OF=0,  ZF set if zero result
	jz		L2			// parent, since r1 == 0 in parent, 1 in child
	xorq		%rax, %rax		// zero rax
	jmp		*%rdi

L2:
	movq		__current_pid@GOTPCREL(%rip), %rdx
	lock
	addl		$1, (%rdx)
	jmp		*%rdi

#elif defined(__arm__)

#include <arm/arch.h>
		
	.globl	cerror
	MI_ENTRY_POINT(___vfork)

	MI_GET_ADDRESS(r3, __current_pid)	// get address of __current_pid
#ifdef _ARM_ARCH_6
L0:	
	ldrex	r1, [r3]
	subs	r1, r1, #1			// if __current_pid <= 0, decrement it
	movpl	r1, #-1				// otherwise put -1 in there
	strex	r2, r1, [r3]
	cmp	r2, #0
	bne	L0
#else
	mov	r2, #0x80000000			// load "looking" value
L0:	
	swp	r1, r2, [r3]			// look at the value, lock others out
	cmp	r1, r2				// anyone else trying to look?
	beq	L0				// yes, so wait our turn
        subs    r1, r1, #1                      // if __current_pid <= 0, decrement it
	movpl   r1, #-1                         // otherwise put -1 in there
	str	r1, [r3]
#endif
		
	mov	r1, #1					// prime results
	mov	r12, #SYS_vfork
	swi	#SWI_SYSCALL				// make the syscall
	bcs	Lbotch					// error?
	cmp	r1, #0					// parent (r1=0) or child(r1=1)
	beq	Lparent

	//child here...
	mov	r0, #0
	bx	lr					// return

Lbotch:
	stmfd	sp!, {lr}
	MI_CALL_EXTERNAL(_cerror)			// jump here on error
	mov	r0,#-1					// set the error
	// reload values clobbered by cerror (so we can treat them as live in Lparent)
	MI_GET_ADDRESS(r3, __current_pid)		// get address of __current_pid
	ldmfd   sp!, {lr}
#ifndef _ARM_ARCH_6
	mov	r2, #0x80000000			// load "looking" value
#endif
	// fall thru
	
Lparent:	
#ifdef _ARM_ARCH_6
	ldrex	r1, [r3]
	add	r1, r1, #1			// we're back, decrement vfork count
	strex	r2, r1, [r3]
	cmp	r2, #0
	bne	Lparent
#else
	swp	r1, r2, [r3]			// look at the value, lock others out
	cmp	r1, r2				// anyone else trying to look?
	beq	Lparent				// yes, so wait our turn
	add	r1, r1, #1			// we're back, decrement vfork count
	str	r1, [r3]
#endif

	bx	lr					// return

#elif defined(__arm64__)

	MI_ENTRY_POINT(___vfork)
	ARM64_STACK_PROLOG

	MI_GET_ADDRESS(x9, __current_pid)
Ltry_set_vfork:
	ldxr	w10, [x9]			// Get old current pid value (exclusive)
	mov		w11, #-1			// Will be -1 if current value is positive
	subs	w10, w10, #1		// Subtract one
	csel	w12, w11, w10, pl	// If >= 0, set to -1, else set to (current - 1)
	stxr	w13, w12, [x9]		// Attempt exclusive store to current pid
	cbnz	w13, Ltry_set_vfork	// If store failed, retry
	
	// ARM sets r1 to 1 here.  I don't see why.
	mov		w16, #SYS_vfork		// Set syscall code
	svc		#SWI_SYSCALL
	b.cs 	Lbotch
	cbz		w1, Lparent

	// Child
	mov		w0, #0
	ARM64_STACK_EPILOG

	// Error case
Lbotch:
	PUSH_FRAME
	bl 		_cerror				// Update errno
	mov		w0, #-1				// Set return value
	MI_GET_ADDRESS(x9, __current_pid) // Reload current pid address
	POP_FRAME
	// Fall through	
Lparent:
	ldxr	w10, [x9]			// Exclusive load current pid value
	add		w10, w10, #1		// Increment (i.e. decrement vfork count)
	stxr	w11, w10, [x9]		// Attempt exclusive store of updated vfork count
	cbnz	w11, Lparent		// If exclusive store failed, retry
	ARM64_STACK_EPILOG		// Done, return

#else
#error Unsupported architecture
#endif
