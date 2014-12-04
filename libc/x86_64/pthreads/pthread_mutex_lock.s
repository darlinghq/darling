/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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
#include <architecture/i386/asm_help.h>

#define	PTHRW_LVAL    0
#define	PTHRW_UVAL    4

	.text
	.align	2
	.globl	__commpage_pthread_mutex_lock
__commpage_pthread_mutex_lock:
	pushq	%rbp		    // set up frame for backtrace
	movq	%rsp,%rbp
	pushq	%rbx
	xorl	%ebx,%ebx	    // clear "preemption pending" flag
	movl	$(_COMM_PAGE_SPIN_COUNT), %eax
1:
	testl	PTHRW_LVAL(%rdi),%ecx // is mutex available?
	jz	2f		    // yes, it is available
	pause
	decl	%eax		    // decrement max spin count
	jnz	1b		    // keep spinning
2:
	movq    _commpage_pfz_base(%rip),%rcx
	addq	$(_COMM_TEXT_PFZ_MUTEX_LOCK_OFFSET), %rcx
	call	*%rcx
	testl	%ebx,%ebx	    // pending preemption?
	jz	1f		    // no
	call	_preempt
1:
	popq	%rbx
	popq	%rbp
	ret
