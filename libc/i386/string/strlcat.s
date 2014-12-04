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
// * S T R L C A T *
// *****************
//
// size_t  strlcat(char *dst, const char *src, size_t size);
//
// We use SSE to do the initial strlen(), and word-parallel copies
// to do the move.  This appears to be faster than either all SSE
// or all word-parallel, at least on Core2 class machines.
//
// Using 4- or 16-byte parallel loops introduce a complication:
// if we blindly did parallel load/stores until finding
// a 0, we might get a spurious page fault by touching bytes past it.
// To avoid this, we never do a load that crosses a page boundary,
// or store unnecessary bytes.
//
// The word parallel test for 0s relies on the following inobvious
// but very efficient test:
//		x =  dataWord + 0xFEFEFEFF
//		y = ~dataWord & 0x80808080
//		if (x & y) == 0 then no zero found
// The test maps any non-zero byte to zero, and any zero byte to 0x80,
// with one exception: 0x01 bytes preceeding the first zero are also
// mapped to 0x80.
//
// On Core2 class machines, this algorithm seems to be faster than the
// naive byte-by-byte version for operands longer than about 10 bytes.  

        .text
        .globl _strlcat

        .align 	4
_strlcat:				// size_t *strlcat(char *dst, const char *src, size_t size);
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	movl	16(%esp),%edi		// get dest ptr
	movl	20(%esp),%esi		// get source ptr
	movl	24(%esp),%ebx		// get length of buffer


// Use SSE to find the 0-byte at current end of buffer.
// This is just a minor variant of strlen().

	movl	%edi,%ecx		// copy buffer ptr
	andl	$(-16),%edi		// 16-byte align buffer ptr
	pxor	%xmm0,%xmm0		// get some 0s
	andl	$15,%ecx		// get #bytes in dq before start of buffer
	movl	$16,%edx
	orl	$(-1),%eax
	subl	%ecx,%edx		// #bytes from buffer start to end of dq
	subl	%edx,%ebx		// does buffer end before end of dq?
	jb	LShortBuf1		// yes, drop into byte-by-byte mode
	movdqa	(%edi),%xmm1		// get first aligned chunk of buffer
	addl	$16,%edi
	pcmpeqb	%xmm0,%xmm1		// check for 0s
	shl	%cl,%eax		// create mask for the bytes of aligned dq in operand
	pmovmskb %xmm1,%ecx		// collect mask of 0-bytes
	andl	%eax,%ecx		// mask out any 0s that occur before buffer start
	jnz	2f			// found end of buffer	
1:
	subl	$16,%ebx		// another dq in buffer?
	jb	LShortBuf2		// no, drop into byte-by-byte mode
	movdqa	(%edi),%xmm1		// get next chunk
	addl	$16,%edi
	pcmpeqb	%xmm0,%xmm1		// check for 0s
	pmovmskb %xmm1,%ecx		// collect mask of 0-bytes
	testl	%ecx,%ecx		// any 0-bytes?
	jz	1b			// no
2:
	bsf	%ecx,%edx		// find first 1-bit (ie, first 0-byte)
	subl	$16,%edi		// back up ptr into buffer
	addl	$16,%ebx		// recover length remaining as of start of dq
	addl	%edx,%edi		// point to 0-byte
	subl	%edx,%ebx		// compute #bytes remaining in buffer


// Copy byte-by-byte until source is 4-byte aligned.
//	%edi = points to 1st byte available in buffer
//	%esi = src ptr
//	%ebx = buffer length remaining (ie, starting at %edi)
//
// NB: the rest of this code is cut-and-pasted from strlcpy().

	movl	%esi,%edx		// copy source ptr
	negl	%edx
	andl	$3,%edx			// how many bytes to align source ptr?
	jz	LAligned		// already aligned


// Loop over bytes.
//	%edi = dest ptr
//	%esi = source ptr
//	%ebx = length remaining in buffer
//	%edx = number of bytes to copy (>0, may not fit in buffer)

LLoopOverBytes:
	movzb	(%esi),%eax		// get source byte before checking buffer length
	testl	%ebx,%ebx		// buffer full?
	jz	L0NotFound		// yes
	inc	%esi
	dec	%ebx
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
//	%ebx = length remaining in buffer

LAligned:
	movl	$5,%edx			// if buffer almost exhausted, prepare to copy rest byte-by-byte
	cmpl	$4,%ebx			// enough for at least one word?
	jb	LLoopOverBytes
	

// Loop over words.
//	%edi = dest ptr (unaligned)
//	%esi = source ptr (word aligned)
//	%ebx = length remaining in buffer (>=4)

LLoopOverWords:
	movl	(%esi),%eax		// get next 4 bytes of source
	subl	$4,%ebx
	addl	$4,%esi
	movl	%eax,%edx		// make 2 copies of word
	movl	%eax,%ecx
	notl	%edx			// use magic word-parallel test for 0s
	addl	$0xFEFEFEFF,%ecx
	andl	$0x80808080,%edx
	testl	%ecx,%edx
	jnz	L0Found			// one of the bytes of %eax is a 0
	movl	%eax,(%edi)		// pack 4 bytes into destination
	addl	$4,%edi
	cmpl	$4,%ebx			// room in buffer for another word?
	jae	LLoopOverWords		// yes
	
	movl	%ebx,%edx		// copy leftovers in byte loop
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

// Buffer filled but 0-byte not found.  We return the length of the buffer plus the length
// of the source string.  This is not optimized, as it is an error condition.
//	%edi = dest ptr (ie, 1 past end of buffer)
//	%esi = source ptr (ptr to 1st byte that does not fit)
	
L0NotFound:
	movl	24(%esp),%eax		// reload buffer length
	testl	%eax,%eax		// null?
	jz	LScanSourceTo0		// yes, cannot store a 0
	xorl	%edx,%edx		// get a 0
	movb	%dl,-1(%edi)		// store a 0 at end of buffer to delimit string
LScanSourceTo0:
	movzb	(%esi),%edx		// get next byte of source
	incl	%esi
	incl	%eax
	testl	%edx,%edx		// 0?
	jnz	LScanSourceTo0
	decl	%eax			// don't count the 0-byte
	jmp	LExit


// Buffer too short to reach end of even one 16-byte aligned chunk.
//	%esi = src ptr

LShortBuf1:
	movl	16(%esp),%edi		// recover ptr to start of buffer
	movl	24(%esp),%ebx		// recover buffer length
	jmp	LShortBuf3
	

// Out of aligned dq's of buffer, 0-byte still not found.
//	%esi = src ptr
//	%edi = 1st buffer byte not checked for 0
//	%ebx = length remaining - 16

LShortBuf2:
	addl	$16,%ebx		// length remaining
LShortBuf3:
	movl	24(%esp),%eax		// recover original buffer length, in case 0-byte not found
	movl	$17,%edx		// buffer almost exhausted, prepare to copy byte-by-byte
1:
	testl	%ebx,%ebx		// no 0s in buffer at all?
	jz	LScanSourceTo0		// yes, cannot store a 0
	cmpb	$0,(%edi)		// is this the 0?
	jz	LLoopOverBytes		// yes, append source
	incl	%edi
	decl	%ebx
	jmp	1b			// loop looking for 0
