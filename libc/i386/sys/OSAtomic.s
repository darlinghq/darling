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
#include <platfunc.h>
#include <architecture/i386/asm_help.h>

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

// compare and exchange 64-bit
// xchg64 <dst> <mp>
.macro xchg64
	.if $1 == ATOMIC_MP
	lock
	.endif
	cmpxchg8b	($0)
.endm


// int32_t OSAtomicAdd32(int32_t theAmount, volatile int32_t *theValue);
#define ATOMIC_ARITHMETIC(instr, orig, mp) \
	movl	8(%esp), %ecx	/* load 2nd arg ptr into ecx */ ;\
	movl	(%ecx), %eax	/* load contents of ecx into eax */ ;\
1:	movl	4(%esp), %edx	/* load 1st arg into edx */ ;\
	instr	%eax, %edx	/* do the operation */ ;\
	xchg32	%edx, %ecx, mp	/* old in %eax, new in %edx, exchange into %ecx */ ;\
	jnz	1b		/* go back if we failed to exchange */ ;\
	.if orig == ATOMIC_RET_NEW ;\
	movl	%edx, %eax	/* return new value */ ;\
	.endif

// bool OSAtomicTestAndSet(uint32_t n, volatile void *theAddress);
#define ATOMIC_BIT_OP(instr, mp) \
	movl	4(%esp), %eax	;\
	movl	8(%esp), %edx	;\
	shldl	$3,%edx,%ecx    /* save top 3 bits of address in %ecx */ ;\
	shll	$3,%edx		;\
	xorl	$7,%eax         /* bit position is numbered big endian so convert to little endian */ ;\
	addl	%eax,%edx       /* generate bit address */ ;\
	adcl	$0,%ecx         /* handle carry out of lower half of address */ ;\
	movl	%edx,%eax       /* copy lower half of bit address */ ;\
	andl	$31,%eax        /* keep bit offset in range 0..31 */ ;\
	xorl	%eax,%edx       /* 4-byte align address */ ;\
	shrdl	$3,%ecx,%edx    /* restore 32-bit byte address in %edx */ ;\
	.if mp == ATOMIC_MP	;\
	lock			;\
	.endif			;\
	instr	%eax, (%edx)	;\
	setc	%al		;\
	movzbl	%al,%eax	// widen in case caller assumes we return an int

// int64_t OSAtomicAdd64(int64_t theAmount, volatile int64_t *theValue);
#define ATOMIC_ADD64(mp) \
	pushl	%ebx		;\
	pushl	%esi		;\
	movl	20(%esp), %esi	;\
	movl	0(%esi), %eax	;\
	movl	4(%esi), %edx	;\
1:	movl	12(%esp), %ebx	;\
	movl	16(%esp), %ecx	;\
	addl	%eax, %ebx	;\
	adcl	%edx, %ecx	;\
	xchg64	%esi, mp	;\
	jnz	1b		;\
	movl	%ebx, %eax	;\
	movl	%ecx, %edx	;\
	popl	%esi		;\
	popl	%ebx

	.text

PLATFUNC_FUNCTION_START(OSAtomicAnd32, up, 32, 2)
PLATFUNC_FUNCTION_START(OSAtomicAnd32Barrier, up, 32, 2)
	ATOMIC_ARITHMETIC(andl, ATOMIC_RET_NEW, ATOMIC_UP)
	ret

PLATFUNC_FUNCTION_START_GENERIC(OSAtomicAnd32, mp, 32, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicAnd32Barrier, mp, 32, 2)
	ATOMIC_ARITHMETIC(andl, ATOMIC_RET_NEW, ATOMIC_MP)
	ret

PLATFUNC_FUNCTION_START(OSAtomicOr32, up, 32, 2)
PLATFUNC_FUNCTION_START(OSAtomicOr32Barrier, up, 32, 2)
	ATOMIC_ARITHMETIC(orl, ATOMIC_RET_NEW, ATOMIC_UP)
	ret

PLATFUNC_FUNCTION_START_GENERIC(OSAtomicOr32, mp, 32, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicOr32Barrier, mp, 32, 2)
	ATOMIC_ARITHMETIC(orl, ATOMIC_RET_NEW, ATOMIC_MP)
	ret

PLATFUNC_FUNCTION_START(OSAtomicXor32, up, 32, 2)
PLATFUNC_FUNCTION_START(OSAtomicXor32Barrier, up, 32, 2)
	ATOMIC_ARITHMETIC(xorl, ATOMIC_RET_NEW, ATOMIC_UP)
	ret

PLATFUNC_FUNCTION_START_GENERIC(OSAtomicXor32, mp, 32, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicXor32Barrier, mp, 32, 2)
	ATOMIC_ARITHMETIC(xorl, ATOMIC_RET_NEW, ATOMIC_MP)
	ret

PLATFUNC_FUNCTION_START(OSAtomicAnd32Orig, up, 32, 2)
PLATFUNC_FUNCTION_START(OSAtomicAnd32OrigBarrier, up, 32, 2)
	ATOMIC_ARITHMETIC(andl, ATOMIC_RET_ORIG, ATOMIC_UP)
	ret

PLATFUNC_FUNCTION_START_GENERIC(OSAtomicAnd32Orig, mp, 32, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicAnd32OrigBarrier, mp, 32, 2)
	ATOMIC_ARITHMETIC(andl, ATOMIC_RET_ORIG, ATOMIC_MP)
	ret

PLATFUNC_FUNCTION_START(OSAtomicOr32Orig, up, 32, 2)
PLATFUNC_FUNCTION_START(OSAtomicOr32OrigBarrier, up, 32, 2)
	ATOMIC_ARITHMETIC(orl, ATOMIC_RET_ORIG, ATOMIC_UP)
	ret

PLATFUNC_FUNCTION_START_GENERIC(OSAtomicOr32Orig, mp, 32, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicOr32OrigBarrier, mp, 32, 2)
	ATOMIC_ARITHMETIC(orl, ATOMIC_RET_ORIG, ATOMIC_MP)
	ret

PLATFUNC_FUNCTION_START(OSAtomicXor32Orig, up, 32, 2)
PLATFUNC_FUNCTION_START(OSAtomicXor32OrigBarrier, up, 32, 2)
	ATOMIC_ARITHMETIC(xorl, ATOMIC_RET_ORIG, ATOMIC_UP)
	ret

PLATFUNC_FUNCTION_START_GENERIC(OSAtomicXor32Orig, mp, 32, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicXor32OrigBarrier, mp, 32, 2)
	ATOMIC_ARITHMETIC(xorl, ATOMIC_RET_ORIG, ATOMIC_MP)
	ret

// bool OSAtomicCompareAndSwapInt(int oldValue, int newValue, volatile int *theValue);
PLATFUNC_FUNCTION_START(OSAtomicCompareAndSwapPtr, up, 32, 2)
PLATFUNC_FUNCTION_START(OSAtomicCompareAndSwapPtrBarrier, up, 32, 2)
PLATFUNC_FUNCTION_START(OSAtomicCompareAndSwapInt, up, 32, 2)
PLATFUNC_FUNCTION_START(OSAtomicCompareAndSwapIntBarrier, up, 32, 2)
PLATFUNC_FUNCTION_START(OSAtomicCompareAndSwapLong, up, 32, 2)
PLATFUNC_FUNCTION_START(OSAtomicCompareAndSwapLongBarrier, up, 32, 2)
PLATFUNC_FUNCTION_START(OSAtomicCompareAndSwap32, up, 32, 2)
PLATFUNC_FUNCTION_START(OSAtomicCompareAndSwap32Barrier, up, 32, 2)
	movl	4(%esp), %eax
	movl	8(%esp), %edx
	movl	12(%esp), %ecx
	xchg32	%edx, %ecx, ATOMIC_UP
	sete	%al
	movzbl	%al,%eax	// widen in case caller assumes we return an int
	ret

PLATFUNC_FUNCTION_START_GENERIC(OSAtomicCompareAndSwapPtr, mp, 32, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicCompareAndSwapPtrBarrier, mp, 32, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicCompareAndSwapInt, mp, 32, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicCompareAndSwapIntBarrier, mp, 32, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicCompareAndSwapLong, mp, 32, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicCompareAndSwapLongBarrier, mp, 32, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicCompareAndSwap32, mp, 32, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicCompareAndSwap32Barrier, mp, 32, 2)
	movl	4(%esp), %eax
	movl	8(%esp), %edx
	movl	12(%esp), %ecx
	xchg32	%edx, %ecx, ATOMIC_MP
	sete	%al
	movzbl	%al,%eax	// widen in case caller assumes we return an int
	ret

// bool OSAtomicCompareAndSwap64(int64_t oldValue, int64_t newValue, volatile int64_t *theValue);
PLATFUNC_FUNCTION_START(OSAtomicCompareAndSwap64, up, 32, 2)
PLATFUNC_FUNCTION_START(OSAtomicCompareAndSwap64Barrier, up, 32, 2)
	pushl	%ebx		// push out spare stuff for space
	pushl	%esi
	movl    12(%esp), %eax	// load in 1st 64-bit parameter
	movl    16(%esp), %edx
	movl    20(%esp), %ebx	// load in 2nd 64-bit parameter
	movl    24(%esp), %ecx
	movl	28(%esp), %esi	// laod in destination address
	xchg64	%esi, ATOMIC_UP	// compare and swap 64-bit
	sete	%al
	movzbl	%al,%eax	// widen in case caller assumes we return an int
	popl	%esi
	popl	%ebx
	ret

PLATFUNC_FUNCTION_START_GENERIC(OSAtomicCompareAndSwap64, mp, 32, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicCompareAndSwap64Barrier, mp, 32, 2)
	pushl	%ebx		// push out spare stuff for space
	pushl	%esi
	movl    12(%esp), %eax	// load in 1st 64-bit parameter
	movl    16(%esp), %edx
	movl    20(%esp), %ebx	// load in 2nd 64-bit parameter
	movl    24(%esp), %ecx
	movl	28(%esp), %esi	// laod in destination address
	xchg64	%esi, ATOMIC_MP	// compare and swap 64-bit
	sete	%al
	movzbl	%al,%eax	// widen in case caller assumes we return an int
	popl	%esi
	popl	%ebx
	ret

PLATFUNC_FUNCTION_START(OSAtomicAdd32, up, 32, 2)
PLATFUNC_FUNCTION_START(OSAtomicAdd32Barrier, up, 32, 2)
	movl	4(%esp), %eax
	movl	8(%esp), %edx
	movl	%eax, %ecx
	xaddl	%eax, (%edx)
	addl	%ecx, %eax
	ret

PLATFUNC_FUNCTION_START_GENERIC(OSAtomicAdd32, mp, 32, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicAdd32Barrier, mp, 32, 2)
	movl	4(%esp), %eax
	movl	8(%esp), %edx
	movl	%eax, %ecx
	lock
	xaddl	%eax, (%edx)
	addl	%ecx, %eax
	ret

PLATFUNC_FUNCTION_START(OSAtomicAdd64, up, 32, 2)
PLATFUNC_FUNCTION_START(OSAtomicAdd64Barrier, up, 32, 2)
	ATOMIC_ADD64(ATOMIC_UP)
	ret

PLATFUNC_FUNCTION_START_GENERIC(OSAtomicAdd64, mp, 32, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicAdd64Barrier, mp, 32, 2)
	ATOMIC_ADD64(ATOMIC_MP)
	ret

PLATFUNC_FUNCTION_START(OSAtomicTestAndSet, up, 32, 2)
PLATFUNC_FUNCTION_START(OSAtomicTestAndSetBarrier, up, 32, 2)
	ATOMIC_BIT_OP(btsl, ATOMIC_UP)
	ret

PLATFUNC_FUNCTION_START_GENERIC(OSAtomicTestAndSet, mp, 32, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicTestAndSetBarrier, mp, 32, 2)
	ATOMIC_BIT_OP(btsl, ATOMIC_MP)
	ret

PLATFUNC_FUNCTION_START(OSAtomicTestAndClear, up, 32, 2)
PLATFUNC_FUNCTION_START(OSAtomicTestAndClearBarrier, up, 32, 2)
	ATOMIC_BIT_OP(btrl, ATOMIC_UP)
	ret

PLATFUNC_FUNCTION_START_GENERIC(OSAtomicTestAndClear, mp, 32, 2)
PLATFUNC_FUNCTION_START_GENERIC(OSAtomicTestAndClearBarrier, mp, 32, 2)
	ATOMIC_BIT_OP(btrl, ATOMIC_MP)
	ret
	
// OSMemoryBarrier()
// These are used both in 32 and 64-bit mode.  We use a fence even on UP
// machines, so this function can be used with nontemporal stores.

PLATFUNC_FUNCTION_START_GENERIC(OSMemoryBarrier, all, 32, 4)
	lock
	addl	$0,(%esp)
	ret
PLATFUNC_DESCRIPTOR(OSMemoryBarrier,all,0,kHasSSE2);

PLATFUNC_FUNCTION_START(OSMemoryBarrier, sse2, 32, 4)
	mfence
	ret
PLATFUNC_DESCRIPTOR(OSMemoryBarrier,sse2,kHasSSE2,0);

	/*
	 *	typedef	volatile struct {
	 *		void	*opaque1;  <-- ptr to 1st queue element or null
	 *		long	 opaque2;  <-- generation count
	 *	} OSQueueHead;
	 *
	 * void  OSAtomicEnqueue( OSQueueHead *list, void *new, size_t offset);
	 */
PLATFUNC_FUNCTION_START(OSAtomicEnqueue, up, 32, 2)
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	movl	16(%esp),%edi	// %edi == ptr to list head
	movl	20(%esp),%ebx	// %ebx == new
	movl	24(%esp),%esi	// %esi == offset
	movl	(%edi),%eax	// %eax == ptr to 1st element in Q
	movl	4(%edi),%edx	// %edx == current generation count
1:	movl	%eax,(%ebx,%esi)// link to old list head from new element
	movl	%edx,%ecx
	incl	%ecx		// increment generation count
	xchg64	%edi, ATOMIC_UP	// ...push on new element
	jnz	1b
	popl	%ebx
	popl	%esi
	popl	%edi
	ret

PLATFUNC_FUNCTION_START_GENERIC(OSAtomicEnqueue, mp, 32, 2)
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	movl	16(%esp),%edi	// %edi == ptr to list head
	movl	20(%esp),%ebx	// %ebx == new
	movl	24(%esp),%esi	// %esi == offset
	movl	(%edi),%eax	// %eax == ptr to 1st element in Q
	movl	4(%edi),%edx	// %edx == current generation count
1:	movl	%eax,(%ebx,%esi)// link to old list head from new element
	movl	%edx,%ecx
	incl	%ecx		// increment generation count
	xchg64	%edi, ATOMIC_MP	// ...push on new element
	jnz	1b
	popl	%ebx
	popl	%esi
	popl	%edi
	ret

/* void* OSAtomicDequeue( OSQueueHead *list, size_t offset); */
PLATFUNC_FUNCTION_START(OSAtomicDequeue, up, 32, 2)
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	movl	16(%esp),%edi	// %edi == ptr to list head
	movl	20(%esp),%esi	// %esi == offset
	movl	(%edi),%eax	// %eax == ptr to 1st element in Q
	movl	4(%edi),%edx	// %edx == current generation count
1:	testl	%eax,%eax	// list empty?
	jz	2f		// yes
	movl	(%eax,%esi),%ebx // point to 2nd in Q
	movl	%edx,%ecx
	incl	%ecx		// increment generation count
	xchg64	%edi, ATOMIC_UP	// ...pop off 1st element
	jnz	1b
2:	popl	%ebx
	popl	%esi
	popl	%edi
	ret			// ptr to 1st element in Q still in %eax

PLATFUNC_FUNCTION_START_GENERIC(OSAtomicDequeue, mp, 32, 2)
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	movl	16(%esp),%edi	// %edi == ptr to list head
	movl	20(%esp),%esi	// %esi == offset
	movl	(%edi),%eax	// %eax == ptr to 1st element in Q
	movl	4(%edi),%edx	// %edx == current generation count
1:	testl	%eax,%eax	// list empty?
	jz	2f		// yes
	movl	(%eax,%esi),%ebx // point to 2nd in Q
	movl	%edx,%ecx
	incl	%ecx		// increment generation count
	xchg64	%edi, ATOMIC_MP	// ...pop off 1st element
	jnz	1b
2:	popl	%ebx
	popl	%esi
	popl	%edi
	ret			// ptr to 1st element in Q still in %eax

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
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	xorl	%ebx,%ebx	// clear "preemption pending" flag
	movl	16(%esp),%edi	// %edi == ptr to list head
	movl	20(%esp),%esi	// %esi == new
	EXTERN_TO_REG(_commpage_pfz_base,%ecx)
	movl	(%ecx), %ecx
	addl	$(_COMM_TEXT_PFZ_ENQUEUE_OFFSET), %ecx
	movl	24(%esp),%edx	// %edx == offset
	call	*%ecx
	testl	%ebx,%ebx	// pending preemption?
	jz	1f
	call	_preempt
1:	popl	%ebx
	popl	%esi
	popl	%edi
	ret
	
/* void* OSAtomicFifoDequeue( OSFifoQueueHead *list, size_t offset); */
	.align	2
	.globl	_OSAtomicFifoDequeue
_OSAtomicFifoDequeue:
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	xorl	%ebx,%ebx	// clear "preemption pending" flag
	movl	16(%esp),%edi	// %edi == ptr to list head
	PICIFY(_commpage_pfz_base)
	movl	(%edx),%ecx
	movl	20(%esp),%edx	// %edx == offset
	addl	$(_COMM_TEXT_PFZ_DEQUEUE_OFFSET), %ecx
	call	*%ecx
	testl	%ebx,%ebx	// pending preemption?
	jz	1f
	pushl	%eax		// save return value across sysenter
	call	_preempt
	popl	%eax
1:	popl	%ebx
	popl	%esi
	popl	%edi
	ret			// ptr to 1st element in Q still in %eax

// Local Variables:
// tab-width: 8
// End:
