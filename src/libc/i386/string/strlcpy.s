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
// We optimize the move by doing it word parallel.  This introduces
// a complication: if we blindly did word load/stores until finding
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
//
// On Core2 class machines, this word-parallel implementation seems to
// be slightly faster than using SSE up to about 100 bytes.
// It is faster than the naive byte-by-byte implementation for
// operands longer than about 8 bytes.

        .text
        .globl _strlcpy

        .align 	4
_strlcpy:				// size_t *strlcpy(char *dst, const char *src, size_t size);
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	movl	16(%esp),%edi		// get dest ptr
	movl	20(%esp),%esi		// get source ptr
	movl	24(%esp),%ecx		// get length of buffer
	movl	%esi,%edx		// copy source ptr
	negl	%edx
	andl	$3,%edx			// how many bytes to align source ptr?
	jz	LAligned		// already aligned


// Loop over bytes.
//	%edi = dest ptr
//	%esi = source ptr
//	%ecx = length remaining in buffer
//	%edx = number of bytes to copy (>0, may not fit in buffer)

LLoopOverBytes:
	movzb	(%esi),%eax		// get source byte before checking buffer length
	testl	%ecx,%ecx		// buffer full?
	jz	L0NotFound		// yes
	inc	%esi
	dec	%ecx
	movb	%al,(%edi)		// pack into dest
	inc	%edi
	testl	%eax,%eax		// 0?
	jz	LDone			// yes, done
	dec	%edx			// more to go?
	jnz	LLoopOverBytes
	

// Source is aligned.  Loop over words until end of buffer.  We
// align the source, rather than the dest, to avoid getting spurious page faults.
//	%edi = dest ptr (unaligned)
//	%esi = source ptr (word aligned)
//	%ecx = length remaining in buffer

LAligned:
	movl	$5,%edx			// if buffer almost exhausted, prepare to copy rest byte-by-byte
	cmpl	$4,%ecx			// enough for at least one word?
	jb	LLoopOverBytes
	

// Loop over words.
//	%edi = dest ptr (unaligned)
//	%esi = source ptr (word aligned)
//	%ecx = length remaining in buffer (>=4)

LLoopOverWords:
	movl	(%esi),%eax		// get next 4 bytes of source
	subl	$4,%ecx
	addl	$4,%esi
	movl	%eax,%edx		// make 2 copies of word
	movl	%eax,%ebx
	notl	%edx			// use magic word-parallel test for 0s
	addl	$0xFEFEFEFF,%ebx
	andl	$0x80808080,%edx
	testl	%ebx,%edx
	jnz	L0Found			// one of the bytes of %eax is a 0
	movl	%eax,(%edi)		// pack 4 bytes into destination
	addl	$4,%edi
	cmpl	$4,%ecx			// room in buffer for another word?
	jae	LLoopOverWords		// yes
	
	movl	%ecx,%edx		// copy leftovers in byte loop
	jmp	LLoopOverBytes
	
// Found a 0-byte in the word of source.  Store a byte at a time until the 0.
//	%edi = dest ptr (unaligned)
//	%eax = last word of source, known to have a 0-byte

LNextByte:
	shrl	$8,%eax			// next byte
L0Found:
	movb	%al,(%edi)		// pack in next byte
	incl	%edi
	testb	%al,%al			// 0?
	jnz	LNextByte
	
// Done storing string.
//	%edi = ptr to byte after 0-byte

LDone:
	subl	16(%esp),%edi		// subtract original dest ptr to get length stored
	decl	%edi			// don't count the 0-byte
	movl	%edi,%eax		// copy to return value
LExit:
	popl	%ebx
	popl	%esi
	popl	%edi
	ret

// Buffer filled but 0-byte not found.  We return the length of the source string.
// This is not optimized, as it is an error condition.
//	%edi = dest ptr (ie, 1 past end of buffer)
//	%esi = source ptr (ptr to 1st byte that does not fit)
	
L0NotFound:
	movl	24(%esp),%eax		// reload buffer length
	testl	%eax,%eax		// null?
	jz	1f			// yes, cannot store a 0
	xorl	%edx,%edx		// get a 0
	movb	%dl,-1(%edi)		// store a 0 at end of buffer to delimit string
1:
	movzb	(%esi),%edx		// get next byte of source
	incl	%esi
	incl	%eax
	testl	%edx,%edx		// 0?
	jnz	1b
	decl	%eax			// don't count the 0-byte
	jmp	LExit
