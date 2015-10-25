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

	.text
	.align	2
	.globl	__commpage_pthread_mutex_lock
__commpage_pthread_mutex_lock:
	pushl	%ebp			    // set up frame for backtrace
	movl	%esp,%ebp
	pushl	%esi
	pushl	%edi
	pushl	%ebx
	xorl	%ebx,%ebx		    // clear "preemption pending" flag
	movl	20(%esp),%edi		    // %edi == ptr to LVAL/UVAL structure
	lea	20(%esp),%esi		    // %esi == ptr to argument list
	movl	_COMM_PAGE_SPIN_COUNT, %edx
	movl	16(%esi),%ecx		    // get mask (ie, PTHRW_EBIT etc)
1:
	testl	0(%edi),%ecx		    // is mutex available?
	jz	2f			    // yes, it is available
	pause
	decl	%edx			    // decrement max spin count
	jnz	1b			    // keep spinning
2:
	EXTERN_TO_REG(_commpage_pfz_base, %ecx)
	movl	(%ecx), %ecx
	addl	$(_COMM_TEXT_PFZ_MUTEX_LOCK_OFFSET), %ecx
	call	*%ecx
	testl	%ebx,%ebx		    // pending preemption?
	jz	3f
	pushl	%eax			    // save return value across sysenter
	call	_preempt
	popl	%eax
3:
	popl	%ebx
	popl	%edi
	popl	%esi
	popl	%ebp
	ret
