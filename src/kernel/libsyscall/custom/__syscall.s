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
/* Copyright (c) 1992 NeXT Computer, Inc.  All rights reserved. */

#include "SYS.h"

#if defined(__i386__)

LEAF(___syscall, 0)
	popl	%ecx		// ret addr
	popl	%eax		// syscall number
	pushl	%ecx
	UNIX_SYSCALL_TRAP
	movl	(%esp),%edx	// add one element to stack so
	pushl	%ecx		// caller "pop" will work
	jnb	2f
	BRANCH_EXTERN(tramp_cerror)
2:
END(___syscall)

#elif defined(__x86_64__)

// For x86-64, the kernel slides the argument list for us.
// The number of arguments here is variable, but our macros ignore
// that value anyway.
__SYSCALL(___syscall, syscall, 0);

#elif defined(__arm__)

__SYSCALL(___syscall, syscall, 7)

#elif defined(__arm64__)

/* 
 * Ignore nominal number of arguments: just pop from stack and let the kernel 
 * interpret.
 */
#include <mach/arm64/asm.h>
MI_ENTRY_POINT(___syscall)
		ldp x1, x2, [sp]
		ldp x3, x4, [sp, #16]
		ldp x5, x6, [sp, #32]
		ldr x7, [sp, #48]
		DO_SYSCALL(SYS_syscall, cerror)
		ret
#else
#error Unsupported architecture
#endif
