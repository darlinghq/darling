/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
 * Copyright (c) 2004-2006 Apple Computer, Inc. All rights reserved.
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

#include <architecture/i386/asm_help.h>
#include "os/internal_asm.h"

.text

#define ATOMIC_RET_ORIG	0
#define ATOMIC_RET_NEW	1

// compare and exchange 32-bit
// xchg32 <new> <dst>
.macro xchg32
	lock
	cmpxchgl	$0, ($1)
.endm

// xchg64 <new> <dst>
.macro xchg64
	lock
	cmpxchg		$0, ($1)
.endm

#define	ATOMIC_ARITHMETIC(instr, orig)	\
	movl	(%rsi), %eax	/* get 2nd arg -> eax */ ;\
1:	movl	%eax, %edx	/* copy value to new reg */ ;\
	instr	%edi, %edx	/* apply instr to %edx with arg2 */ ;\
	xchg32	%edx, %rsi	/* do the compare swap (see macro above) */ ;\
	jnz	1b		/* jump if failed */ ;\
	.if orig == 1		/* to return the new value, overwrite eax */ ;\
	movl	%edx, %eax	/* return the new value */ ;\
	.endif

// Used in OSAtomicTestAndSet( uint32_t n, void *value ), assumes ABI parameter loctions
// Manpage says bit to test/set is (0x80 >> (n & 7)) of byte (addr + (n >> 3))
#define	ATOMIC_BIT_OP(instr)	\
	xorl	$7, %edi	/* bit position is numbered big endian so convert to little endian */ ;\
	shlq	$3, %rsi 	;\
	addq	%rdi, %rsi	/* generate bit address */ ;\
	movq	%rsi, %rdi	;\
	andq	$31, %rdi	/* keep bit offset in range 0..31 */ ;\
	xorq	%rdi, %rsi	/* 4-byte align address */ ;\
	shrq	$3, %rsi	/* get 4-byte aligned address */ ;\
	lock			/* lock the bit test */ ;\
	instr	%edi, (%rsi)	/* do the bit test, supplied into the macro */ ;\
	setc	%al		;\
	movzbl	%al,%eax	/* widen in case caller assumes we return an int */

// uint32_t OSAtomicAnd32( uint32_t mask, uint32_t *value);
OS_ATOMIC_FUNCTION_START(OSAtomicAnd32, 2)
OS_ATOMIC_FUNCTION_START(OSAtomicAnd32Barrier, 2)
	ATOMIC_ARITHMETIC(andl, ATOMIC_RET_NEW)
	ret

// uint32_t OSAtomicOr32( uint32_t mask, uint32_t *value);
OS_ATOMIC_FUNCTION_START(OSAtomicOr32, 2)
OS_ATOMIC_FUNCTION_START(OSAtomicOr32Barrier, 2)
	ATOMIC_ARITHMETIC(orl, ATOMIC_RET_NEW)
	ret

// uint32_t OSAtomicXor32( uint32_t mask, uint32_t *value);
OS_ATOMIC_FUNCTION_START(OSAtomicXor32, 2)
OS_ATOMIC_FUNCTION_START(OSAtomicXor32Barrier, 2)
	ATOMIC_ARITHMETIC(xorl, ATOMIC_RET_NEW)
	ret

// uint32_t OSAtomicAnd32Orig( uint32_t mask, uint32_t *value);
OS_ATOMIC_FUNCTION_START(OSAtomicAnd32Orig, 2)
OS_ATOMIC_FUNCTION_START(OSAtomicAnd32OrigBarrier, 2)
	ATOMIC_ARITHMETIC(andl, ATOMIC_RET_ORIG)
	ret
	
// uint32_t OSAtomicOr32Orig( uint32_t mask, uint32_t *value);
OS_ATOMIC_FUNCTION_START(OSAtomicOr32Orig, 2)
OS_ATOMIC_FUNCTION_START(OSAtomicOr32OrigBarrier, 2)
	ATOMIC_ARITHMETIC(orl, ATOMIC_RET_ORIG)
	ret

// uint32_t OSAtomicXor32Orig( uint32_t mask, uint32_t *value);
OS_ATOMIC_FUNCTION_START(OSAtomicXor32Orig, 2)
OS_ATOMIC_FUNCTION_START(OSAtomicXor32OrigBarrier, 2)
	ATOMIC_ARITHMETIC(xorl, ATOMIC_RET_ORIG)
	ret

// bool OSAtomicCompareAndSwap32( int32_t old, int32_t new, int32_t *value);
OS_ATOMIC_FUNCTION_START(OSAtomicCompareAndSwapInt, 2)
OS_ATOMIC_FUNCTION_START(OSAtomicCompareAndSwapIntBarrier, 2)
OS_ATOMIC_FUNCTION_START(OSAtomicCompareAndSwap32, 2)
OS_ATOMIC_FUNCTION_START(OSAtomicCompareAndSwap32Barrier, 2)
	movl	%edi, %eax
	xchg32	%esi, %rdx
	sete	%al
	movzbl	%al,%eax	// widen in case caller assumes we return an int
	ret

// bool OSAtomicCompareAndSwap64( int64_t old, int64_t new, int64_t *value);
OS_ATOMIC_FUNCTION_START(OSAtomicCompareAndSwapPtr, 2)
OS_ATOMIC_FUNCTION_START(OSAtomicCompareAndSwapPtrBarrier, 2)
OS_ATOMIC_FUNCTION_START(OSAtomicCompareAndSwapLong, 2)
OS_ATOMIC_FUNCTION_START(OSAtomicCompareAndSwapLongBarrier, 2)
OS_ATOMIC_FUNCTION_START(OSAtomicCompareAndSwap64, 2)
OS_ATOMIC_FUNCTION_START(OSAtomicCompareAndSwap64Barrier, 2)
	mov	%rdi, %rax
	xchg64	%rsi, %rdx
	sete	%al
	movzbl	%al,%eax	// widen in case caller assumes we return an int
	ret

// int32_t OSAtomicAdd32( int32_t amt, int32_t *value );
OS_ATOMIC_FUNCTION_START(OSAtomicAdd32, 2)
OS_ATOMIC_FUNCTION_START(OSAtomicAdd32Barrier, 2)
	movl	%edi, %eax	// save amt to add
	lock			// lock prefix breaks tabs ;)
	xaddl	%edi, (%rsi)	// swap and add value, returns old value in %edi
	addl	%edi, %eax	// add old value to amt as return value
	ret

// int32_t OSAtomicIncrement32(int32_t *value );
OS_ATOMIC_FUNCTION_START(OSAtomicIncrement32, 2)
OS_ATOMIC_FUNCTION_START(OSAtomicIncrement32Barrier, 2)
	movl	$1, %eax	// load increment
	lock			// lock prefix breaks tabs ;)
	xaddl	%eax, (%rdi)	// swap and add value, returns old value in %eax
	incl	%eax	// increment old value as return value
	ret

// int32_t OSAtomicDecrement32(int32_t *value );
OS_ATOMIC_FUNCTION_START(OSAtomicDecrement32, 2)
OS_ATOMIC_FUNCTION_START(OSAtomicDecrement32Barrier, 2)
	movl	$-1, %eax	// load decrement
	lock			// lock prefix breaks tabs ;)
	xaddl	%eax, (%rdi)	// swap and add value, returns old value in %eax
	decl	%eax	// decrement old value as return value
	ret

// int64_t OSAtomicAdd64( int64_t amt, int64_t *value );
OS_ATOMIC_FUNCTION_START(OSAtomicAdd64, 2)
OS_ATOMIC_FUNCTION_START(OSAtomicAdd64Barrier, 2)
	movq	%rdi, %rax	// save amt to add
	lock
	xaddq	%rdi, (%rsi)	// swap and add value, returns old value in %rsi
	addq	%rdi, %rax	// add old value to amt as return value
	ret

// int64_t OSAtomicIncrement64(int64_t *value );
OS_ATOMIC_FUNCTION_START(OSAtomicIncrement64, 2)
OS_ATOMIC_FUNCTION_START(OSAtomicIncrement64Barrier, 2)
	movq	$1, %rax	// load increment
	lock			// lock prefix breaks tabs ;)
	xaddq	%rax, (%rdi)	// swap and add value, returns old value in %eax
	incq	%rax	// increment old value as return value
	ret

// int64_t OSAtomicDecrement64(int64_t *value );
OS_ATOMIC_FUNCTION_START(OSAtomicDecrement64, 2)
OS_ATOMIC_FUNCTION_START(OSAtomicDecrement64Barrier, 2)
	movq	$-1, %rax	// load decrement
	lock			// lock prefix breaks tabs ;)
	xaddq	%rax, (%rdi)	// swap and add value, returns old value in %eax
	decq	%rax	// decrement old value as return value
	ret

// bool OSAtomicTestAndSet( uint32_t n, void *value );
OS_ATOMIC_FUNCTION_START(OSAtomicTestAndSet, 2)
OS_ATOMIC_FUNCTION_START(OSAtomicTestAndSetBarrier, 2)
	ATOMIC_BIT_OP(btsl)
	ret

// bool OSAtomicTestAndClear( uint32_t n, void *value );
OS_ATOMIC_FUNCTION_START(OSAtomicTestAndClear, 2)
OS_ATOMIC_FUNCTION_START(OSAtomicTestAndClearBarrier, 2)
	ATOMIC_BIT_OP(btrl)
	ret

// void OSMemoryBarrier( void );
OS_ATOMIC_FUNCTION_START(OSMemoryBarrier, 2)
	mfence
	ret

/*
 *	typedef	volatile struct {
 *		void	*opaque1;  <-- ptr to 1st queue element or null
 *		long	 opaque2;  <-- generation count
 *	} OSQueueHead;
 *
 * void  OSAtomicEnqueue( OSQueueHead *list, void *new, size_t offset);
 */
OS_ATOMIC_FUNCTION_START(OSAtomicEnqueue, 2)
	pushq	%rbx		// %rdi == list head, %rsi == new, %rdx == offset
	movq	%rsi,%rbx	// %rbx == new
	movq	%rdx,%rsi	// %rsi == offset
	movq	(%rdi),%rax	// %rax == ptr to 1st element in Q
	movq	8(%rdi),%rdx	// %rdx == current generation count
1:
	movq	%rax,(%rbx,%rsi)// link to old list head from new element
	movq	%rdx,%rcx
	incq	%rcx		// increment generation count
	lock			// always lock for now...
	cmpxchg16b (%rdi)	// ...push on new element
	jnz	1b
	popq	%rbx
	ret


	/* void* OSAtomicDequeue( OSQueueHead *list, size_t offset); */
OS_ATOMIC_FUNCTION_START(OSAtomicDequeue, 2)
	pushq	%rbx		// %rdi == list head, %rsi == offset
	movq	(%rdi),%rax	// %rax == ptr to 1st element in Q
	movq	8(%rdi),%rdx	// %rdx == current generation count
1:
	testq	%rax,%rax	// list empty?
	jz	2f		// yes
	movq	(%rax,%rsi),%rbx // point to 2nd in Q
	movq	%rdx,%rcx
	incq	%rcx		// increment generation count
	lock			// always lock for now...
	cmpxchg16b (%rdi)	// ...pop off 1st element
	jnz	1b
2:
	popq	%rbx
	ret			// ptr to 1st element in Q still in %rax

/*
 *	typedef	volatile struct {
 *		void	*opaque1;  <-- ptr to first queue element or null
 *		void	*opaque2;  <-- ptr to last queue element or null
 *		int	 opaque3;  <-- spinlock
 *	} OSFifoQueueHead;
 *
 * void  OSAtomicFifoEnqueue( OSFifoQueueHead *list, void *new, size_t offset);
 */
OS_ATOMIC_FUNCTION_START(OSAtomicFifoEnqueue, 2)
#if 0 // Linux has not preemption free zone (PFZ), this needs to be rewritten
	pushq	%rbx
	xorl	%ebx,%ebx	// clear "preemption pending" flag
	movq 	_commpage_pfz_base(%rip),%rcx
	addq	$(_COMM_TEXT_PFZ_ENQUEUE_OFFSET), %rcx
	call	*%rcx
	testl	%ebx,%ebx	// pending preemption?
	jz	1f
	call	_preempt	// call into the kernel to pfz_exit
1:	
	popq	%rbx
#endif
	ret
	
	
/* void* OSAtomicFifoDequeue( OSFifoQueueHead *list, size_t offset); */
OS_ATOMIC_FUNCTION_START(OSAtomicFifoDequeue, 2)
#if 0
	pushq	%rbx
	xorl	%ebx,%ebx	// clear "preemption pending" flag
	movq	_commpage_pfz_base(%rip), %rcx
	movq	%rsi,%rdx	// move offset to %rdx to be like the Enqueue case
	addq	$(_COMM_TEXT_PFZ_DEQUEUE_OFFSET), %rcx
	call	*%rcx
	testl	%ebx,%ebx	// pending preemption?
	jz	1f
	call	_preempt	// call into the kernel to pfz_exit
1:	
	popq	%rbx
#endif
	ret			// ptr to 1st element in Q in %rax

// Local Variables:
// tab-width: 8
// End:
