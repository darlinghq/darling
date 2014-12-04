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

#include <machine/cpu_capabilities.h>
#include "platfunc.h"
#include <architecture/i386/asm_help.h>

#define DECLARE(x)   \
	.align 2, 0x90      ; \
	.globl x            ; \
	.globl x ## Barrier ; \
x:                  ; \
x ## Barrier:

.text

#define ATOMIC_UP	0
#define ATOMIC_MP	1
#define ATOMIC_RET_ORIG	0
#define ATOMIC_RET_NEW	1

// compare and exchange 32-bit
// xchg32 <new> <dst> <mp>
.macro xchg32
	.if $2 == ATOMIC_MP
	lock
	.endif
	cmpxchgl	$0, ($1)
.endm

// xchg64 <new> <dst> <mp>
.macro xchg64
	.if $2 == ATOMIC_MP
	lock
	.endif
	cmpxchg		$0, ($1)
.endm

#define	ATOMIC_ARITHMETIC(instr, orig, mp)	\
	movl	(%rsi), %eax	/* get 2nd arg -> eax */ ;\
1:	movl	%eax, %edx	/* copy value to new reg */ ;\
	instr	%edi, %edx	/* apply instr to %edx with arg2 */ ;\
	xchg32	%edx, %rsi, mp	/* do the compare swap (see macro above) */ ;\
	jnz	1b		/* jump if failed */ ;\
	.if orig == 1		/* to return the new value, overwrite eax */ ;\
	movl	%edx, %eax	/* return the new value */ ;\
	.endif

// Used in OSAtomicTestAndSet( uint32_t n, void *value ), assumes ABI parameter loctions
// Manpage says bit to test/set is (0x80 >> (n & 7)) of byte (addr + (n >> 3))
#define	ATOMIC_BIT_OP(instr, mp)	\
	xorl	$7, %edi	/* bit position is numbered big endian so convert to little endian */ ;\
	shlq	$3, %rsi 	;\
	addq	%rdi, %rsi	/* generate bit address */ ;\
	movq	%rsi, %rdi	;\
	andq	$31, %rdi	/* keep bit offset in range 0..31 */ ;\
	xorq	%rdi, %rsi	/* 4-byte align address */ ;\
	shrq	$3, %rsi	/* get 4-byte aligned address */ ;\
	.if mp == ATOMIC_MP 	/* don't plant the lock in UP code */ ;\
	lock			/* lock the bit test */ ;\
	.endif			;\
	instr	%edi, (%rsi)	/* do the bit test, supplied into the macro */ ;\
	setc	%al		;\
	movzbl	%al,%eax	/* widen in case caller assumes we return an int */

// uint32_t OSAtomicAnd32( uint32_t mask, uint32_t *value);
PLATFUNC_FUNCTION_START(OSAtomicAnd32, up, 64, 2)
PLATFUNC_FUNCTION_START(OSAtomicAnd32Barrier, up, 64, 2)
	ATOMIC_ARITHMETIC(andl, ATOMIC_RET_NEW, ATOMIC_UP)
	ret
	
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicAnd32, mp, 64, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicAnd32Barrier, mp, 64, 2)
	ATOMIC_ARITHMETIC(andl, ATOMIC_RET_NEW, ATOMIC_MP)
	ret

// uint32_t OSAtomicOr32( uint32_t mask, uint32_t *value);
PLATFUNC_FUNCTION_START(OSAtomicOr32, up, 64, 2)
PLATFUNC_FUNCTION_START(OSAtomicOr32Barrier, up, 64, 2)
	ATOMIC_ARITHMETIC(orl, ATOMIC_RET_NEW, ATOMIC_UP)
	ret
	
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicOr32, mp, 64, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicOr32Barrier, mp, 64, 2)
	ATOMIC_ARITHMETIC(orl, ATOMIC_RET_NEW, ATOMIC_MP)
	ret

// uint32_t OSAtomicXor32( uint32_t mask, uint32_t *value);
PLATFUNC_FUNCTION_START(OSAtomicXor32, up, 64, 2)
PLATFUNC_FUNCTION_START(OSAtomicXor32Barrier, up, 64, 2)
	ATOMIC_ARITHMETIC(xorl, ATOMIC_RET_NEW, ATOMIC_UP)
	ret

PLATFUNC_FUNCTION_START_GENERIC(OSAtomicXor32, mp, 64, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicXor32Barrier, mp, 64, 2)
	ATOMIC_ARITHMETIC(xorl, ATOMIC_RET_NEW, ATOMIC_MP)
	ret

// uint32_t OSAtomicAnd32Orig( uint32_t mask, uint32_t *value);
PLATFUNC_FUNCTION_START(OSAtomicAnd32Orig, up, 64, 2)
PLATFUNC_FUNCTION_START(OSAtomicAnd32OrigBarrier, up, 64, 2)
	ATOMIC_ARITHMETIC(andl, ATOMIC_RET_ORIG, ATOMIC_UP)
	ret

PLATFUNC_FUNCTION_START_GENERIC(OSAtomicAnd32Orig, mp, 64, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicAnd32OrigBarrier, mp, 64, 2)
	ATOMIC_ARITHMETIC(andl, ATOMIC_RET_ORIG, ATOMIC_MP)
	ret
	
// uint32_t OSAtomicOr32Orig( uint32_t mask, uint32_t *value);
PLATFUNC_FUNCTION_START(OSAtomicOr32Orig, up, 64, 2)
PLATFUNC_FUNCTION_START(OSAtomicOr32OrigBarrier, up, 64, 2)
	ATOMIC_ARITHMETIC(orl, ATOMIC_RET_ORIG, ATOMIC_UP)
	ret

PLATFUNC_FUNCTION_START_GENERIC(OSAtomicOr32Orig, mp, 64, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicOr32OrigBarrier, mp, 64, 2)
	ATOMIC_ARITHMETIC(orl, ATOMIC_RET_ORIG, ATOMIC_MP)
	ret

// uint32_t OSAtomicXor32Orig( uint32_t mask, uint32_t *value);
PLATFUNC_FUNCTION_START(OSAtomicXor32Orig, up, 64, 2)
PLATFUNC_FUNCTION_START(OSAtomicXor32OrigBarrier, up, 64, 2)
	ATOMIC_ARITHMETIC(xorl, ATOMIC_RET_ORIG, ATOMIC_UP)
	ret

PLATFUNC_FUNCTION_START_GENERIC(OSAtomicXor32Orig, mp, 64, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicXor32OrigBarrier, mp, 64, 2)
	ATOMIC_ARITHMETIC(xorl, ATOMIC_RET_ORIG, ATOMIC_MP)
	ret

// bool OSAtomicCompareAndSwap32( int32_t old, int32_t new, int32_t *value);
PLATFUNC_FUNCTION_START(OSAtomicCompareAndSwapInt, up, 64, 2)
PLATFUNC_FUNCTION_START(OSAtomicCompareAndSwapIntBarrier, up, 64, 2)
PLATFUNC_FUNCTION_START(OSAtomicCompareAndSwap32, up, 64, 2)
PLATFUNC_FUNCTION_START(OSAtomicCompareAndSwap32Barrier, up, 64, 2)
	movl	%edi, %eax
	xchg32	%esi, %rdx, ATOMIC_UP
	sete	%al
	movzbl	%al,%eax	// widen in case caller assumes we return an int
	ret

PLATFUNC_FUNCTION_START_GENERIC(OSAtomicCompareAndSwapInt, mp, 64, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicCompareAndSwapIntBarrier, mp, 64, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicCompareAndSwap32, mp, 64, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicCompareAndSwap32Barrier, mp, 64, 2)
	movl	%edi, %eax
	xchg32	%esi, %rdx, ATOMIC_MP
	sete	%al
	movzbl	%al,%eax	// widen in case caller assumes we return an int
	ret

// bool OSAtomicCompareAndSwap64( int64_t old, int64_t new, int64_t *value);
PLATFUNC_FUNCTION_START(OSAtomicCompareAndSwapPtr, up, 64, 2)
PLATFUNC_FUNCTION_START(OSAtomicCompareAndSwapPtrBarrier, up, 64, 2)
PLATFUNC_FUNCTION_START(OSAtomicCompareAndSwapLong, up, 64, 2)
PLATFUNC_FUNCTION_START(OSAtomicCompareAndSwapLongBarrier, up, 64, 2)
PLATFUNC_FUNCTION_START(OSAtomicCompareAndSwap64, up, 64, 2)
PLATFUNC_FUNCTION_START(OSAtomicCompareAndSwap64Barrier, up, 64, 2)
	mov	%rdi, %rax
	xchg64	%rsi, %rdx, ATOMIC_UP
	sete	%al
	movzbl	%al,%eax	// widen in case caller assumes we return an int
	ret
	
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicCompareAndSwapPtr, mp, 64, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicCompareAndSwapPtrBarrier, mp, 64, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicCompareAndSwapLong, mp, 64, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicCompareAndSwapLongBarrier, mp, 64, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicCompareAndSwap64, mp, 64, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicCompareAndSwap64Barrier, mp, 64, 2)
	mov	%rdi, %rax
	xchg64	%rsi, %rdx, ATOMIC_MP
	sete	%al
	movzbl	%al,%eax	// widen in case caller assumes we return an int
	ret

// int32_t OSAtomicAdd32( int32_t amt, int32_t *value );
PLATFUNC_FUNCTION_START(OSAtomicAdd32, up, 64, 2)
PLATFUNC_FUNCTION_START(OSAtomicAdd32Barrier, up, 64, 2)
	movl	%edi, %eax	// save amt to add
	xaddl	%edi, (%rsi)	// swap and add value, returns old value in %edi
	addl	%edi, %eax	// add old value to amt as return value
	ret

PLATFUNC_FUNCTION_START_GENERIC(OSAtomicAdd32, mp, 64, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicAdd32Barrier, mp, 64, 2)
	movl	%edi, %eax	// save amt to add
	lock			// lock prefix breaks tabs ;)
	xaddl	%edi, (%rsi)	// swap and add value, returns old value in %edi
	addl	%edi, %eax	// add old value to amt as return value
	ret

// int64_t OSAtomicAdd64( int64_t amt, int64_t *value );
PLATFUNC_FUNCTION_START(OSAtomicAdd64, up, 64, 2)
PLATFUNC_FUNCTION_START(OSAtomicAdd64Barrier, up, 64, 2)
	movq	%rdi, %rax	// save amt to add
	xadd	%rdi, (%rsi)	// swap and add value, returns old value in %rsi
	addq	%rdi, %rax	// add old value to amt as return value
	ret

PLATFUNC_FUNCTION_START_GENERIC(OSAtomicAdd64, mp, 64, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicAdd64Barrier, mp, 64, 2)
	movq	%rdi, %rax	// save amt to add
	lock
	xadd	%rdi, (%rsi)	// swap and add value, returns old value in %rsi
	addq	%rdi, %rax	// add old value to amt as return value
	ret

// bool OSAtomicTestAndSet( uint32_t n, void *value );
PLATFUNC_FUNCTION_START(OSAtomicTestAndSet, up, 64, 2)
PLATFUNC_FUNCTION_START(OSAtomicTestAndSetBarrier, up, 64, 2)
	ATOMIC_BIT_OP(btsl, ATOMIC_UP)
	ret

PLATFUNC_FUNCTION_START_GENERIC(OSAtomicTestAndSet, mp, 64, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicTestAndSetBarrier, mp, 64, 2)
	ATOMIC_BIT_OP(btsl, ATOMIC_MP)
	ret

// bool OSAtomicTestAndClear( uint32_t n, void *value );
PLATFUNC_FUNCTION_START(OSAtomicTestAndClear, up, 64, 2)
PLATFUNC_FUNCTION_START(OSAtomicTestAndClearBarrier, up, 64, 2)
	ATOMIC_BIT_OP(btrl, ATOMIC_UP)
	ret

PLATFUNC_FUNCTION_START_GENERIC(OSAtomicTestAndClear, mp, 64, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicTestAndClearBarrier, mp, 64, 2)
	ATOMIC_BIT_OP(btrl, ATOMIC_MP)
	ret

// void OSMemoryBarrier( void );
	.align	2, 0x90
	.globl	_OSMemoryBarrier
_OSMemoryBarrier:
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
	.align	2
	.globl	_OSAtomicEnqueue
_OSAtomicEnqueue:		// %rdi == list head, %rsi == new, %rdx == offset
	pushq	%rbx
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
	.align	2
	.globl	_OSAtomicDequeue
_OSAtomicDequeue:		// %rdi == list head, %rsi == offset
	pushq	%rbx
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
	.align	2
	.globl	_OSAtomicFifoEnqueue
_OSAtomicFifoEnqueue:
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
	ret
	
	
/* void* OSAtomicFifoDequeue( OSFifoQueueHead *list, size_t offset); */
	.align	2
	.globl	_OSAtomicFifoDequeue
_OSAtomicFifoDequeue:
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
	ret			// ptr to 1st element in Q in %rax

// Local Variables:
// tab-width: 8
// End:
