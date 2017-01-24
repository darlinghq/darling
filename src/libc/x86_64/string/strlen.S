/*
 * Copyright (c) 2005-2012 Apple Computer, Inc. All rights reserved.
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
 *
 * This file implements strlen( ) for the x86_64 architecture.
 */

.globl _strlen

/*****************************************************************************
 *  Macros                                                                   *
 *****************************************************************************/

.macro EstablishFrame
	push      %rbp
	mov       %rsp,      %rbp
.endm

.macro ClearFrameAndReturn
	pop       %rbp
	ret
.endm

/*****************************************************************************
 *  Entrypoint                                                               *
 *****************************************************************************/

.text
.align 5
_strlen:
//	size_t strlen(const char *s);
//
//	returns the length of the string s (i.e. the distance in bytes from
//	s to the first NUL byte following s).  We look for NUL bytes using
//	pcmpeqb on 16-byte aligned blocks.  Although this may read past the
//	end of the string, because all access is aligned, it will never
//	read past the end of the string across a page boundary, or even
//	accross a cacheline.
	EstablishFrame
	mov       %rdi,	     %rcx
	mov       %rdi,      %rdx

//	Load the 16-byte block containing the first byte of the string, and
//	compare each byte to zero.  If any NUL bytes are present in this
//	block, the corresponding *bit* in esi will be set to 1.
	and       $-16,      %rdi
	pxor      %xmm0,     %xmm0
	pcmpeqb  (%rdi),     %xmm0
	pmovmskb  %xmm0,     %esi

//	The 16 bytes that we checked for NUL included some bytes preceeding
//	the start of the string, if s is not 16-byte aligned.  We create a
//	mask based on the alignment of s which covers only those bits
//	corresponding to bytes that do not preceed s, and check for NULs
//	only in those bits.  If we do not find one, we jump to our main
//	search loop.
	and       $0xf,      %rcx
	or        $-1,       %rax
	shl       %cl,       %rax
	and       %eax,      %esi
	jz        L_loop

L_foundNUL:
//	The last 16-byte block that we searched contained at least one NUL.
//	We use bsf to identify the first NUL, and compute the distance from
//	that byte to the start of the string.
	bsf       %esi,      %eax
	sub       %rdx,      %rdi
	add       %rdi,      %rax
	ClearFrameAndReturn

.align 4
L_loop:
//	Main search loop: check for NUL in a 16-byte block, continuing
//	loop until one is found.
	add       $16,       %rdi
	pxor      %xmm0,     %xmm0
	pcmpeqb  (%rdi),     %xmm0
	pmovmskb  %xmm0,     %esi
	test      %esi,      %esi
	jz        L_loop
	jmp       L_foundNUL
