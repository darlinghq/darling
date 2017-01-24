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
 * This file implements strcpy( ) for the x86_64 architecture.
 */

.globl _strcpy

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
_strcpy:
//	char *strcpy(char * restrict d, const char * restrict s);
//
//	copies the string s to d, and returns d.  We look for NUL bytes using
//	pcmpeqb on 16-byte aligned blocks.  Although this may read past the
//	end of the string, because all access is aligned, it will never
//	read past the end of the string across a page boundary, or even
//	accross a cacheline.
	EstablishFrame
	mov       %rsi,	     %rcx

//	Load the 16-byte block containing the first byte of the string, and
//	compare each byte to zero.  If any NUL bytes are present in this
//	block, the corresponding *bit* in esi will be set to 1.
	and       $-16,      %rsi
	pxor      %xmm0,     %xmm0
	pcmpeqb  (%rsi),     %xmm0
	pmovmskb  %xmm0,     %eax

//	The 16 bytes that we checked for NUL included some bytes preceeding
//	the start of the string, if s is not 16-byte aligned.  We create a
//	mask based on the alignment of s which covers only those bits
//	corresponding to bytes that do not preceed s, and check for NULs
//	only in those bits.  If we find one, the string is too small to use
//  a vector copy, so jump to dedicated small-buffer implementation.
	and       $0xf,      %rcx
	or        $-1,       %rdx
	shl       %cl,       %rdx
	and       %edx,      %eax
	jnz       L_strcpyGPR

//	Check the next 16-byte block for NUL.  If none are found, that guarantees
//	that the string is at least 16 bytes long, which means that we can use a
//	single unaligned vector copy to handle any edging at the start of the
//	string.  If instead a NUL is found, fall into the byte-by-byte copy loop.
	movdqa 16(%rsi),     %xmm1
	pxor      %xmm0,     %xmm0
	pcmpeqb   %xmm1,     %xmm0
	pmovmskb  %xmm0,     %edx
	test      %edx,      %edx
	jz        L_strcpySSE

/*****************************************************************************
 *  GPR copy implementation                                                  *
 *****************************************************************************/

//	There is at least one NUL in the 32 aligned bytes containing the start
//	of the string being copied.  We assemble a bitmap for those 32 bytes from
//	eax and edx, then shift it right by cl to throw out any bits preceeding
//	the start of the string.  We can then identify the position of the
//	first NUL byte using BSF.
	shl       $16,       %edx
	or        %edx,      %eax
L_strcpyGPR:
	shr       %cl,       %eax
	bsf       %eax,      %edx
//	Restore the original source pointer, and copy the destination pointer
//	to rax so that it is returned on exit.
	add       %rcx,      %rsi
	mov       %rdi,      %rax
	add       $1,        %rdx
	call      _memcpy
	ClearFrameAndReturn
/*
//	At this point we simply need to copy rdx + 1 bytes from rsi to rdi.  If
//	the length is >= 8, start by doing a word-by-word copy; otherwise, use
//	a byte-by-byte copy loop.
	sub       $7,        %rdx // 7 instead of 8 to account for NUL
	jb        1f
0:	mov      (%rsi,%rdx),%rcx
	mov       %rcx,     (%rdi,%rdx)
	sub       $8,        %rdx
	jae       0b
1:	add       $8,        %rdx
	jz        3f
2:	movzb  -1(%rsi,%rdx),%rcx
	movb      %cl,    -1(%rdi,%rdx)
	sub       $1,        %rdx
	jnz       2b
3:	ClearFrameAndReturn
 */

/*****************************************************************************
 *  SSE copy implementation                                                  *
 *****************************************************************************/

L_strcpySSE:
//	Begin by doing a single unaligned vector copy for edging.  We no longer
//	have the original source pointer, but we can reconstruct it as rsi + rcx.
	movdqu   (%rsi,%rcx),%xmm0
	movdqu    %xmm0,    (%rdi)
//	Next copy the original destination pointer to rax so that it is returned
//	on exit, and adjust the destination pointer to correspond to rsi.
	mov       %rdi,      %rax
	sub       %rcx,      %rdi
	xor       %rcx,      %rcx
//	Main copy loop: store the 16 bytes loaded in the previous iteration of the
//	loop, as they are already known to not contain a NUL.  The load the next
//	16 bytes and check for NUL.
0:	movdqu    %xmm1,  16(%rdi,%rcx)
	add       $16,       %rcx
	movdqa 16(%rsi,%rcx),%xmm1
	pxor      %xmm0,     %xmm0
	pcmpeqb   %xmm1,     %xmm0
	pmovmskb  %xmm0,     %edx
	test      %edx,      %edx
	jz        0b

//	Cleanup: at least one of the bytes in the last 16 that were loaded was
//	NUL.  The corresponding bits of dx are set, and all other bits are zero.
//	Thus, we can use BSF to find the position of the first NUL.  Once we have
//	this information, we use an unaligned copy that runs precisely up to this
//	position to handle edging.
	bsf       %edx,      %edx
	add       %rdx,      %rcx
	movdqu  1(%rsi,%rcx),%xmm0      // offset is 1 so that we copy the trailing
	movdqu    %xmm0,   1(%rdi,%rcx) // NUL byte as well.
	ClearFrameAndReturn

