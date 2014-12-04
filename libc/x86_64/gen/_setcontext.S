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
LABEL(__setcontext)
	/* struct mcontext_t * %rdi */
#if DEBUG
	movq  MCONTEXT_SS_RSI(%rdi),   %rsi
	movq  MCONTEXT_SS_RCX(%rdi),   %rcx
	movq  MCONTEXT_SS_R8+0(%rdi),  %r8
	movq  MCONTEXT_SS_R8+8(%rdi),  %r9
	movq  MCONTEXT_SS_R8+16(%rdi), %r10
	movq  MCONTEXT_SS_R8+24(%rdi), %r11
#endif
	movq  MCONTEXT_SS_RBX(%rdi),   %rbx
	movq  MCONTEXT_SS_R8+32(%rdi), %r12
	movq  MCONTEXT_SS_R8+40(%rdi), %r13
	movq  MCONTEXT_SS_R8+48(%rdi), %r14
	movq  MCONTEXT_SS_R8+56(%rdi), %r15

	movq  MCONTEXT_SS_RSP(%rdi), %rsp
	movq  MCONTEXT_SS_RBP(%rdi), %rbp

	xorl  %eax, %eax 	/* force x=getcontext(); ... setcontext(); to keep x==0 */

#if DEBUG
	movq  MCONTEXT_SS_RIP(%rdi), %rdx
	movq  MCONTEXT_SS_RDI(%rdi), %rdi
	jmp  *%rdx
#else
	jmp  *MCONTEXT_SS_RIP(%rdi)
#endif

#endif /* __x86_64__ */
