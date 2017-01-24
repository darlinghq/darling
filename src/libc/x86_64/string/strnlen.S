/*
 * Copyright (c) 2012 Apple Computer, Inc. All rights reserved.
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
 * This file implements strnlen( ) for the x86_64 architecture.
 */

.globl _strnlen

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
.align 4
_strnlen:
//	size_t strnlen(char *s, size_t maxlen);

//	If maxlen is larger than any object that can be allocated, we know a priori
//	that it does not effect the operation of the function in any way; we can
//	simply call strlen instead, which is more efficient and makes handling the
//	edge cases here much cleaner.
	test      %rsi,      %rsi
	js        _strlen

//	The strnlen() function attempts to compute the length of s, but never
//	scans beyond the first maxlen bytes of s.
//
//	Thus, we need to early-out without doing any reads at all if maxlen == 0.
	EstablishFrame
	mov       %rsi,      %rax
	jz        L_maxlenExhausted

//	We are going to check the string in aligned 16-byte blocks.  The first such
//	block may contain characters that preceed the start of the string, so we
//	construct a mask based on the string's alignment to use in processing this
//	initial block.  We also need to account for these characters in maxlen.
	mov       %rdi,      %rcx
	and       $0xf,      %rcx
	or        $-1,       %rdx
	shl       %cl,       %rdx // mask
	add       %rcx,      %rsi // adjust maxlen

//	Load the 16-byte block containing the start of the string.  If any NUL
//	bytes are present in this block, the corresponding *bit* in ecx will be 1.
//	We check only the bits that are set in the mask, to avoid detecting NULs
//	that preceed the start of the string.
	and       $-16,      %rdi
	pxor      %xmm0,     %xmm0
	pcmpeqb  (%rdi),     %xmm0
	pmovmskb  %xmm0,     %ecx
	and       %rdx,      %rcx
	jnz       L_foundNUL

//	Now subtract 16 from maxlen.  If this causes a borrow, then we exhausted
//	maxlen somewhere in this 16-byte block (formally, we have read past maxlen
//	bytes, but that is not a problem; because the accesses are all aligned, we
//	cannot read accross a page--or even cacheline--boundary, so the observable
//	behavior is not different from if we had stopped at maxlen).  If the result
//	is exactly zero, we need to stop before reading the *next* 16 bytes.
	sub       $16,       %rsi
	jbe       L_maxlenExhausted

L_loop:
	add       $16,       %rdi
	pxor      %xmm0,     %xmm0
	pcmpeqb  (%rdi),     %xmm0
	pmovmskb  %xmm0,     %ecx
	test      %rcx,      %rcx
	jnz       L_foundNUL
	sub       $16,       %rsi
	ja        L_loop

L_maxlenExhausted:
//	If we exhaust maxlen bytes without finding a NUL, we return maxlen.
	ClearFrameAndReturn

L_foundNUL:
//	The last 16-byte block that we searched contained at least one NUL.
//	We use bsf to identify the first NUL.
	bsf       %rcx,      %rdx
//	Handle the case where the NUL that we found is preceeded by the maxlen'th
//	byte of the string, returning maxlen.
	cmp       %rdx,      %rsi
	jb        L_maxlenExhausted
//	Otherwise, return the length of the string.
	sub       %rsi,      %rax
	add       %rdx,      %rax
	ClearFrameAndReturn
