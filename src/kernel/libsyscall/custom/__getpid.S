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

#include "SYS.h"

#if defined(__i386__)

	.data
	.private_extern __current_pid
L__current_pid_addr:
 __current_pid:
	.long 0

#if defined(__DYNAMIC__)
#define GET_CURRENT_PID				\
	call	0f				; \
0:						; \
	popl	%ecx				; \
	leal	L__current_pid_addr-0b(%ecx), %ecx

#define __current_pid (%ecx)

#else
#define GET_CURRENT_PID
#endif

/*
 * If __current_pid is > 0, return it, else make syscall.
 * If __current_pid is 0, cache result of syscall.
 */
TEXT
LEAF(___getpid, 0)
	GET_CURRENT_PID
	movl		__current_pid, %eax
	testl		%eax, %eax
	jle		1f
	ret
1:
	UNIX_SYSCALL_NONAME(getpid, 0, cerror_nocancel)
	movl		%eax, %edx
	xorl		%eax, %eax
	GET_CURRENT_PID
	lock
	cmpxchgl	%edx, __current_pid
	movl		%edx, %eax
	ret

#elif defined(__x86_64__)

	.data
	.private_extern __current_pid
__current_pid:
	.long 0

/*
 * If __current_pid is > 0, return it, else make syscall.
 * If __current_pid is 0, cache result of syscall.
 */
TEXT
LEAF(___getpid, 0)
	movl	__current_pid(%rip), %eax
	testl	%eax, %eax
	jle		1f
	ret
1:
	UNIX_SYSCALL_NONAME(getpid, 0, cerror_nocancel)
	movl		%eax, %edx
	xorl		%eax, %eax
	leaq		__current_pid(%rip), %rcx
	lock
	cmpxchgl	%edx, (%rcx)
	movl		%edx, %eax
	ret

#elif defined(__arm__)

#include <arm/arch.h>
	
	.data
	.globl	__current_pid
	.align 2
__current_pid:
	/* Cached pid.  Possible values:
	 *	0:		no value cached
	 *	> 0:		cached PID of current process
	 *	< 0:		negative number of vforks in progress
	 *	INT_MIN:	for pre-ARMv6, "looking" value (0x80000000)
	 */
	.long 0

MI_ENTRY_POINT(___getpid)
	ldr	r3, L__current_pid
L1:	add	r3, pc, r3		// r3 = &__current_pid
	ldr	r0, [r3]		// get the cached pid
	cmp	r0, #0
	bxgt	lr			// if positive, return it

	SYSCALL_NONAME(getpid, 0, cerror_nocancel)

#ifdef _ARM_ARCH_6
	ldrex	r2, [r3]		// see if we can cache it
	cmp	r2, #0			// we can't if there are any...
	bxlt	lr			// ...vforks in progress
	strex	r2, r0, [r3]		// ignore conflicts
#else
	mov	r1, #0x80000000		// load "looking" value
	swp	r2, r1, [r3]		// look at the value, lock others out
	cmp	r2, r1			// anyone else trying to look?
	bxeq	lr			// yes, so return immediately/	
	cmp	r2, #0			// see if we can cache it
	streq	r0, [r3]		// if zero, we can
	strne	r2, [r3]		// otherwise restore previous value
#endif
		
	bx	lr
				
L__current_pid:	
	.long	__current_pid - (L1+8)		

#elif defined(__arm64__)
	.data
	.globl	__current_pid
	.align 2
__current_pid:
	/* cached pid.  possible values:
	 *	0:		no value cached
	 *	> 0:		cached pid of current process
	 *	< 0:		negative number of vforks in progress
	 *	int_min:	for pre-armv6, "looking" value (0x80000000)
	 */
	.long 0

MI_ENTRY_POINT(___getpid)
	MI_GET_ADDRESS(x9, __current_pid)	// Get address of cached value
	ldr		w0, [x9]					// Load it
	cmp		w0, #0						// See if there's a cached value
	b.le	L_notcached					// If not, make syscall
	ret									// Else, we're done
L_notcached:
	SYSCALL_NONAME(getpid, 0, cerror_nocancel)
	ldxr	w10, [x9]					// Exclusive load
	cbnz	w10, L_done					// Unless unset, don't even try
	stxr	wzr, w0, [x9]				// Try to store, but don't care if we fail (someone will win, or not)
L_done:
	ret									// Done
#else
#error Unsupported architecture
#endif
