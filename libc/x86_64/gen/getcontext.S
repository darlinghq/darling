/*
 * Copyright (c) 2007,2009 Apple Inc. All rights reserved.
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

#if defined(__x86_64__)

#include <architecture/i386/asm_help.h>

#define MCONTEXT_SS_RAX     16
#define MCONTEXT_SS_RBX     24
#define MCONTEXT_SS_RCX     32
#define MCONTEXT_SS_RDX     40
#define MCONTEXT_SS_RDI     48
#define MCONTEXT_SS_RSI     56
#define MCONTEXT_SS_RBP     64
#define MCONTEXT_SS_RSP     72
#define MCONTEXT_SS_R8      80
#define MCONTEXT_SS_RIP     144
#define MCONTEXT_SS_RFLAGS  152

TEXT
LABEL(_getcontext)
	/* struct ucontext_t * $rdi */
	push  %rbp
	movq  %rsp, %rbp
	movq  %rsp, %rsi
	CALL_EXTERN(_getmcontext) /* getmcontext(uctx, sp) */
	pop   %rbp

#if DEBUG
	movq  $0, MCONTEXT_SS_RAX(%rax)
	movq  $0, MCONTEXT_SS_RDX(%rax)
	movq  $0, MCONTEXT_SS_RCX(%rax)
	movq  $0, MCONTEXT_SS_RDI(%rax)
	movq  $0, MCONTEXT_SS_RSI(%rax)
	movq  $0, MCONTEXT_SS_R8(%rax)
	movq  $0, MCONTEXT_SS_R8+8(%rax)
	movq  $0, MCONTEXT_SS_R8+16(%rax)
	movq  $0, MCONTEXT_SS_R8+24(%rax)
	movq  $0, MCONTEXT_SS_RFLAGS(%rax)
#endif	

	movq  %rbp, MCONTEXT_SS_RBP(%rax)
	movq  %rbx, MCONTEXT_SS_RBX(%rax)
	movq  %r12, MCONTEXT_SS_R8+32(%rax)
	movq  %r13, MCONTEXT_SS_R8+40(%rax)
	movq  %r14, MCONTEXT_SS_R8+48(%rax)
	movq  %r15, MCONTEXT_SS_R8+56(%rax)
	movq  (%rsp), %rcx		/* return address */
	movq  %rcx, MCONTEXT_SS_RIP(%rax)
	leaq  8(%rsp), %rcx
	movq  %rcx, MCONTEXT_SS_RSP(%rax)
	xorl  %eax, %eax
	ret

#endif /* __x86_64__ */
