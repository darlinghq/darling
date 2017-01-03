/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
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


// *****************
// * S T R L C P Y *
// *****************
//
// size_t  strlcpy(char *dst, const char *src, size_t size);
//
// We optimize the move by doing it quadword parallel.  This introduces
// a complication: if we blindly did quadword load/stores until finding
// a 0, we might get a spurious page fault by touching bytes past it.
// To avoid this, we never do a load that crosses a page boundary,
// or store unnecessary bytes.
//
// The test for 0s relies on the following inobvious but very efficient
// word-parallel test:
//		x =  dataWord + 0xFEFEFEFF
//		y = ~dataWord & 0x80808080
//		if (x & y) == 0 then no zero found
// The test maps any non-zero byte to zero, and any zero byte to 0x80,
// with one exception: 0x01 bytes preceeding the first zero are also
// mapped to 0x80.

        .text
        .globl _strlcpy

// When initially entered:
//	%rdi = dest buffer ptr
//	%rsi = source ptr
//	%rdx = length

        .align 	4
_strlcpy:				// size_t *strlcpy(char *dst, const char *src, size_t size);
	movl	%esi,%ecx		// copy source ptr
	movq	%rdi,%r10		// copy dest ptr
	negl	%ecx
	andl	$7,%ecx			// how many bytes to align source ptr?
	jz	LAligned		// already aligned


// Loop over bytes.
//	%rdi = dest ptr
//	%rsi = source ptr
//	%rdx = length remaining in buffer
//	%ecx = number of bytes to copy (>0, may not fit in buffer)
//	%r10 = original dest ptr

LLoopOverBytes:
	movzb	(%rsi),%eax		// get source byte before checking buffer length
	testq	%rdx,%rdx		// buffer full?
	jz	L0NotFound		// yes
	incq	%rsi
	decq	%rdx
	movb	%al,(%rdi)		// pack into dest
	incq	%rdi
	testl	%eax,%eax		// 0?
	jz	LDone			// yes, done
	decl	%ecx			// more to go?
	jnz	LLoopOverBytes
	

// Source is aligned.  Loop over quadwords until end of buffer.  We
// align the source, rather than the dest, to avoid getting spurious page faults.
//	%rdi = dest ptr (unaligned)
//	%rsi = source ptr (quadword aligned)
//	%rdx = length remaining in buffer
//	%r10 = original dest ptr

LAligned:
	movl	$9,%ecx			// if buffer almost exhausted, prepare to copy rest byte-by-byte
	cmpq	$8,%rdx			// enough for at least one word?
	jb	LLoopOverBytes
	movq	$0xFEFEFEFEFEFEFEFF,%rcx // load magic constants
	movq	$0x8080808080808080,%r11
	

// Loop over quadwords.
//	%rdi = dest ptr (unaligned)
//	%rsi = source ptr (word aligned)
//	%rdx = length remaining in buffer (>=8)
//	%rcx = 0xFEFEFEFEFEFEFEFF
//	%r11 = 0x8080808080808080
//	%r10 = original dest ptr

LLoopOverQuads:
	movq	(%rsi),%rax		// get next 8 bytes of source
	subq	$8,%rdx
	addq	$8,%rsi
	movq	%rax,%r8		// make 2 copies of quadword
	movq	%rax,%r9
	notq	%r8			// use magic word-parallel test for 0s
	addq	%rcx,%r9
	andq	%r11,%r8
	testq	%r8,%r9
	jnz	L0Found			// one of the bytes of %rax is a 0
	movq	%rax,(%rdi)		// pack 8 bytes into destination
	addq	$8,%rdi
	cmpq	$8,%rdx			// room in buffer for another quadword?
	jae	LLoopOverQuads		// yes
	
	movl	%edx,%ecx		// copy leftovers in byte loop
	jmp	LLoopOverBytes
	
// Found a 0-byte in the quadword of source.  Store a byte at a time until the 0.
//	%rdi = dest ptr (unaligned)
//	%rax = last quadword of source, known to have a 0-byte
//	%r10 = original dest ptr

LNextByte:
	shrq	$8,%rax			// next byte
L0Found:
	movb	%al,(%rdi)		// pack in next byte
	incq	%rdi
	testb	%al,%al			// 0?
	jnz	LNextByte
	
// Done storing string.
//	%rdi = ptr to byte after 0-byte
//	%r10 = original dest ptr

LDone:
	subq	%r10,%rdi		// subtract original dest ptr to get length stored
	lea	-1(%rdi),%rax		// minus one for 0-byte, and move to return value
	ret

// Buffer filled but 0-byte not found.  We return the length of the source string.
// This is not optimized, as it is an error condition.
//	%rdi = dest ptr (ie, 1 past end of buffer)
//	%rsi = source ptr (ptr to 1st byte that does not fit)
//	%r10 = original dest ptr
	
L0NotFound:
	movq	%rdi,%rax		// end of buffer...
	subq	%r10,%rax		// ...minus start is buffer length
	jz	1f			// 0-length buffer, cannot store a 0
	xorl	%edx,%edx		// get a 0
	movb	%dl,-1(%rdi)		// store a 0 at end of buffer to delimit string
1:
	movzb	(%rsi),%ecx		// get next byte of source
	incq	%rsi
	incq	%rax
	testl	%ecx,%ecx		// 0?
	jnz	1b
	decq	%rax			// don't count the 0-byte
	ret
