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

#else
#error Unsupported architecture
#endif
