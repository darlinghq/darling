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
	addq		$1, (%rcx)
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
	addq		$1, (%rdx)
	jmp		*%rdi

#else
#error Unsupported architecture
#endif
