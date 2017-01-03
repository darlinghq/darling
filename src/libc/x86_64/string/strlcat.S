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
// Using 8- or 16-byte parallel loops introduce a complication:
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
// naive byte-by-byte version for operands longer than about 11 bytes.  

        .text
        .globl _strlcat



// Use SSE to find the 0-byte at current end of buffer.
// This is just a minor variant of strlen().
// Initial registers:
//	%rdi = dest or buffer ptr
//	%rsi = source ptr
//	%rdx = size

        .align 	4
_strlcat:				// size_t *strlcat(char *dst, const char *src, size_t size);
	movl	%edi,%ecx		// copy buffer ptr
	movq	%rdi,%r10		// save copies of buffer ptr and length
	movq	%rdx,%r11
	andq	$(-16),%rdi		// 16-byte align buffer ptr
	pxor	%xmm0,%xmm0		// get some 0s
	andl	$15,%ecx		// get #bytes in dq before start of buffer
	movl	$16,%r8d
	orl	$(-1),%eax
	subl	%ecx,%r8d		// #bytes from buffer start to end of dq
	subq	%r8,%rdx		// does buffer end before end of dq?
	jb	LShortBuf1		// yes, drop into byte-by-byte mode
	movdqa	(%rdi),%xmm1		// get first aligned chunk of buffer
	addq	$16,%rdi
	pcmpeqb	%xmm0,%xmm1		// check for 0s
	shl	%cl,%eax		// create mask for the bytes of aligned dq in operand
	pmovmskb %xmm1,%ecx		// collect mask of 0-bytes
	andl	%eax,%ecx		// mask out any 0s that occur before buffer start
	jnz	2f			// found end of buffer	
1:
	subq	$16,%rdx		// another dq in buffer?
	jb	LShortBuf2		// no, drop into byte-by-byte mode
	movdqa	(%rdi),%xmm1		// get next chunk
	addq	$16,%rdi
	pcmpeqb	%xmm0,%xmm1		// check for 0s
	pmovmskb %xmm1,%ecx		// collect mask of 0-bytes
	testl	%ecx,%ecx		// any 0-bytes?
	jz	1b			// no
2:
	bsf	%ecx,%ecx		// find first 1-bit (ie, first 0-byte)
	subq	$16,%rdi		// back up ptr into buffer
	addq	$16,%rdx		// recover length remaining as of start of dq
	addq	%rcx,%rdi		// point to 0-byte
	subq	%rcx,%rdx		// compute #bytes remaining in buffer


// Copy byte-by-byte until source is 8-byte aligned.
//	%rdi = points to 1st byte available in buffer
//	%rsi = src ptr
//	%rdx = buffer length remaining (ie, starting at %rdi)
//	%r10 = original buffer ptr

	movl	%esi,%ecx		// copy source ptr
	negl	%ecx
	andl	$7,%ecx			// how many bytes to align source ptr?
	jz	LAligned		// already aligned


// Loop over bytes.
//	%rdi = dest ptr
//	%rsi = source ptr
//	%rdx = length remaining in buffer
//	%ecx = number of bytes to copy (>0, may not fit in buffer)
//	%r10 = original buffer ptr

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
//	%r10 = original buffer ptr

LAligned:
	movl	$9,%ecx			// if buffer almost exhausted, prepare to copy rest byte-by-byte
	cmpq	$8,%rdx			// enough for at least one quadword?
	jb	LLoopOverBytes
	movq	$0xFEFEFEFEFEFEFEFF,%r8	// get magic constants
	movq	$0x8080808080808080,%r9
	

// Loop over quadwords.
//	%rdi = dest ptr (unaligned)
//	%rsi = source ptr (quadword aligned)
//	%rdx = length remaining in buffer (>=8)
//	%r8  = 0xFEFEFEFEFEFEFEFF
//	%r9  = 0x8080808080808080
//	%r10 = original buffer ptr

LLoopOverQuads:
	movq	(%rsi),%rax		// get next 8 bytes of source
	subq	$8,%rdx
	addq	$8,%rsi
	movq	%rax,%r11		// make 2 copies of quadword
	movq	%rax,%rcx
	notq	%r11			// use magic word-parallel test for 0s
	addq	%r8,%rcx
	andq	%r9,%r11
	testq	%rcx,%r11
	jnz	L0Found			// one of the bytes of %rax is a 0
	movq	%rax,(%rdi)		// pack 8 bytes into destination
	addq	$8,%rdi
	cmpq	$8,%rdx			// room in buffer for another quadword?
	jae	LLoopOverQuads		// yes
	
	movl	%edx,%ecx		// copy leftovers in byte loop
	jmp	LLoopOverBytes
	
// Found a 0-byte in the quadword of source.  Store a byte at a time until the 0.
//	%rdi = dest ptr (unaligned)
//	%rax = last word of source, known to have a 0-byte
//	%r10 = original buffer ptr

LNextByte:
	shrq	$8,%rax			// next byte
L0Found:
	movb	%al,(%rdi)		// pack in next byte
	incq	%rdi
	testb	%al,%al			// 0?
	jnz	LNextByte


// Done storing string.
//	%rdi = ptr to byte after 0-byte
//	%r10 = original buffer ptr

LDone:
	subq	%r10,%rdi		// subtract original dest ptr to get length stored
	lea	-1(%rdi),%rax		// minus one for 0-byte, and move to return value
	ret

// Buffer filled but 0-byte not found.  We return the length of the buffer plus the length
// of the source string.  This is not optimized, as it is an error condition.
//	%edi = dest ptr (ie, 1 past end of buffer)
//	%esi = source ptr (ptr to 1st byte that does not fit)
//	%rdx = 0 (ie, length remaining in buffer)
//	%r10 = original buffer ptr
	
L0NotFound:
	movq	%rdi,%rax		// buffer end...
	subq	%r10,%rax		// ...minus start is buffer length
	jz	LScanSourceTo0		// buffer is null so cannot store a 0
	movb	%dl,-1(%rdi)		// store a 0 at end of buffer to delimit string


// Scan source to end.
//	%rsi = ptr to rest of source
//	%rax = return value so far

LScanSourceTo0:
	movzb	(%rsi),%ecx		// get next byte of source
	incq	%rsi
	incq	%rax			// increment length
	testl	%ecx,%ecx		// 0?
	jnz	LScanSourceTo0
	decq	%rax			// don't count the 0-byte
	ret


// Buffer too short to reach end of even one 16-byte aligned chunk.
//	%rsi = src ptr
//	%r10 = original buffer ptr
//	%r11 = original buffer length

LShortBuf1:
	movq	%r10,%rdi		// recover buffer ptr
	movq	%r11,%rdx		// recover buffer length
	jmp	LShortBuf3
	

// Out of aligned dq's of buffer, 0-byte still not found.
//	%rsi = src ptr
//	%rdi = ptr to 1st buffer byte not checked for 0
//	%rdx = length remaining - 16
//	%r10 = original buffer ptr
//	%r11 = original buffer length

LShortBuf2:
	addq	$16,%rdx		// recover length remaining
LShortBuf3:
	movq	%r11,%rax		// in case we goto LScanSourceTo0
	movl	$17,%ecx		// in case we goto LLoopOverBytes
1:
	testq	%rdx,%rdx		// no 0s in buffer at all?
	jz	LScanSourceTo0		// yes, cannot store a 0
	cmpb	$0,(%rdi)		// is this the 0?
	jz	LLoopOverBytes		// yes, append source
	incq	%rdi
	decq	%rdx
	jmp	1b			// loop looking for 0
