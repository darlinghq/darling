/*
 * Copyright (c) 1999-2006 Apple Computer, Inc. All rights reserved.
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
#import <architecture/i386/asm_help.h>

.text
	.globl mcount
mcount:
	pushq	%rbp             // setup mcount's frame
	movq	%rsp,    %rbp
	subq	$64,     %rsp    // allocate space for storage and alignment
	movq	%rax,  0(%rsp)
	movq	%rdi,  8(%rsp)
	movq	%rsi, 16(%rsp)
	movq	%rdx, 24(%rsp)
	movq	%rcx, 32(%rsp)
	movq	%r8,  40(%rsp)
	movq	%r9,  48(%rsp)
	movq	(%rbp),  %rsi    // load the frame pointer of mcount's caller
	movq	8(%rsi), %rsi    // load mcount's caller's return address
	movq	8(%rbp), %rdi    // set up the selfpc parameter for moncount()
	CALL_EXTERN(_moncount)   // call moncount()
	movq	48(%rsp), %r9
	movq	40(%rsp), %r8
	movq	32(%rsp), %rcx
	movq	24(%rsp), %rdx
	movq	16(%rsp), %rsi
	movq	 8(%rsp), %rdi
	movq	 0(%rsp), %rax

	movq	%rbp, %rsp
	popq	%rbp             // tear down frame
	ret
