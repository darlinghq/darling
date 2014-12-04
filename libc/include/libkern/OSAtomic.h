/*
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

#ifndef _OSATOMIC_H_
#define _OSATOMIC_H_

#include    <stddef.h>
#include    <sys/cdefs.h>
#include    <stdint.h>
#include    <stdbool.h>

#include    <Availability.h>

/*! @header
 * These are the preferred versions of the atomic and synchronization operations.
 * Their implementation is customized at boot time for the platform, including
 * late-breaking errata fixes as necessary.   They are thread safe.
 *
 * WARNING: all addresses passed to these functions must be "naturally aligned",
 * i.e.  * <code>int32_t</code> pointers must be 32-bit aligned (low 2 bits of
 * address are zeroes), and <code>int64_t</code> pointers must be 64-bit aligned
 * (low 3 bits of address are zeroes.)
 *
 * Note that some versions of the atomic functions incorporate memory barriers
 * and some do not.  Barriers strictly order memory access on weakly-ordered
 * architectures such as PPC.  All loads and stores that appear (in sequential
 * program order) before the barrier are guaranteed to complete before any
 * load or store that appears after the barrier.
 *
 * On a uniprocessor system, the barrier operation is typically a no-op.  On a
 * multiprocessor system, the barrier can be quite expensive on some platforms,
 * such as PPC.
 *
 * Most code should use the barrier functions to ensure that memory shared between
 * threads is properly synchronized.  For example, if you want to initialize
 * a shared data structure and then atomically increment a variable to indicate
 * that the initialization is complete, you must use {@link OSAtomicIncrement32Barrier}
 * to ensure that the stores to your data structure complete before the atomic
 * increment.
 *
 * Likewise, the consumer of that data structure must use {@link OSAtomicDecrement32Barrier},
 * in order to ensure that their loads of the structure are not executed before
 * the atomic decrement.  On the other hand, if you are simply incrementing a global
 * counter, then it is safe and potentially faster to use {@link OSAtomicIncrement32}.
 *
 * If you are unsure which version to use, prefer the barrier variants as they are
 * safer.
 *
 * The spinlock and queue operations always incorporate a barrier.
 *
 * For the kernel-space version of this header, see
 * {@link //apple_ref/doc/header/OSAtomic.h OSAtomic.h (Kernel Framework)}
 *
 * @apiuid //apple_ref/doc/header/user_space_OSAtomic.h
 */ 
__BEGIN_DECLS


/*! @group Arithmetic functions
    All functions in this group return the new value.
 */

/*! @abstract Atomically adds two 32-bit values.
    @discussion
	This function adds the value given by <code>__theAmount</code> to the
	value in the memory location referenced by <code>__theValue</code>,
 	storing the result back to that memory location atomically.
 @result Returns the new value.
 */
int32_t	OSAtomicAdd32( int32_t __theAmount, volatile int32_t *__theValue );


/*! @abstract Atomically adds two 32-bit values.
    @discussion
	This function adds the value given by <code>__theAmount</code> to the
	value in the memory location referenced by <code>__theValue</code>,
	storing the result back to that memory location atomically.

	This function is equivalent to {@link OSAtomicAdd32}
	except that it also introduces a barrier.
    @result Returns the new value.
 */
int32_t	OSAtomicAdd32Barrier( int32_t __theAmount, volatile int32_t *__theValue );


/*! @abstract Atomically increments a 32-bit value.
 */
__inline static
int32_t	OSAtomicIncrement32( volatile int32_t *__theValue )
            { return OSAtomicAdd32(  1, __theValue); }


/*! @abstract Atomically increments a 32-bit value with a barrier.
    @discussion
	This function is equivalent to {@link OSAtomicIncrement32}
	except that it also introduces a barrier.
    @result Returns the new value.
 */
__inline static
int32_t	OSAtomicIncrement32Barrier( volatile int32_t *__theValue )
            { return OSAtomicAdd32Barrier(  1, __theValue); }

/*! @abstract Atomically decrements a 32-bit value. */
__inline static
int32_t	OSAtomicDecrement32( volatile int32_t *__theValue )
            { return OSAtomicAdd32( -1, __theValue); }

/*! @abstract Atomically increments a 32-bit value with a barrier.
    @discussion
	This function is equivalent to {@link OSAtomicDecrement32}
	except that it also introduces a barrier.
    @result Returns the new value.
 */
__inline static
int32_t	OSAtomicDecrement32Barrier( volatile int32_t *__theValue )
            { return OSAtomicAdd32Barrier( -1, __theValue); }


/*! @abstract Atomically adds two 64-bit values.
    @discussion
	This function adds the value given by <code>__theAmount</code> to the
	value in the memory location referenced by <code>__theValue</code>,
	storing the result back to that memory location atomically.
 */
int64_t	OSAtomicAdd64( int64_t __theAmount, volatile int64_t *__theValue );


/*! @abstract Atomically adds two 64-bit values with a barrier.
    @discussion
	This function adds the value given by <code>__theAmount</code> to the
	value in the memory location referenced by <code>__theValue</code>,
	storing the result back to that memory location atomically.
 
	This function is equivalent to {@link OSAtomicAdd64}
	except that it also introduces a barrier.
    @result Returns the new value.
 */
int64_t	OSAtomicAdd64Barrier( int64_t __theAmount, volatile int64_t *__theValue ) __OSX_AVAILABLE_STARTING(__MAC_10_4, __IPHONE_3_2); 


/*! @abstract Atomically increments a 64-bit value. */
__inline static
int64_t	OSAtomicIncrement64( volatile int64_t *__theValue )
            { return OSAtomicAdd64(  1, __theValue); }

/*! @abstract Atomically increments a 64-bit value with a barrier.
    @discussion
	This function is equivalent to {@link OSAtomicIncrement64}
	except that it also introduces a barrier.
    @result Returns the new value.
 */
__inline static
int64_t	OSAtomicIncrement64Barrier( volatile int64_t *__theValue )
            { return OSAtomicAdd64Barrier(  1, __theValue); }


/*! @abstract Atomically decrements a 64-bit value.
    @discussion
	This function is equivalent to {@link OSAtomicIncrement64}
	except that it also introduces a barrier.
    @result Returns the new value.
 */
__inline static
int64_t	OSAtomicDecrement64( volatile int64_t *__theValue )
            { return OSAtomicAdd64( -1, __theValue); }


/*! @abstract Atomically decrements a 64-bit value with a barrier.
    @discussion
	This function is equivalent to {@link OSAtomicDecrement64}
	except that it also introduces a barrier.
    @result Returns the new value.
 */
__inline static
int64_t	OSAtomicDecrement64Barrier( volatile int64_t *__theValue )
            { return OSAtomicAdd64Barrier( -1, __theValue); }


/*! @group Boolean functions (AND, OR, XOR)
 * 
 * @discussion Functions in this group come in four variants for each operation:
 * with and without barriers, and functions that return the original value or
 * the result value of the operation.
 * 
 * The "Orig" versions return the original value, (before the operation); the non-Orig
 * versions return the value after the operation.  All are layered on top of
 * {@link OSAtomicCompareAndSwap32} and similar.
 */

/*! @abstract Atomic bitwise OR of two 32-bit values.
    @discussion
	This function performs the bitwise OR of the value given by <code>__theMask</code>
	with the value in the memory location referenced by <code>__theValue</code>,
	storing the result back to that memory location atomically.
    @result Returns the new value.
 */
int32_t	OSAtomicOr32( uint32_t __theMask, volatile uint32_t *__theValue );


/*! @abstract Atomic bitwise OR of two 32-bit values with barrier.
    @discussion
	This function performs the bitwise OR of the value given by <code>__theMask</code>
	with the value in the memory location referenced by <code>__theValue</code>,
	storing the result back to that memory location atomically.
 
	This function is equivalent to {@link OSAtomicOr32}
	except that it also introduces a barrier.
    @result Returns the new value.
 */
int32_t	OSAtomicOr32Barrier( uint32_t __theMask, volatile uint32_t *__theValue );


/*! @abstract Atomic bitwise OR of two 32-bit values returning original.
    @discussion
	This function performs the bitwise OR of the value given by <code>__theMask</code>
	with the value in the memory location referenced by <code>__theValue</code>,
	storing the result back to that memory location atomically.
    @result Returns the original value referenced by <code>__theValue</code>.
 */
int32_t	OSAtomicOr32Orig( uint32_t __theMask, volatile uint32_t *__theValue ) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_3_2);


/*! @abstract Atomic bitwise OR of two 32-bit values returning original with barrier.
    @discussion
	This function performs the bitwise OR of the value given by <code>__theMask</code>
	with the value in the memory location referenced by <code>__theValue</code>,
	storing the result back to that memory location atomically.
 
	This function is equivalent to {@link OSAtomicOr32Orig}
	except that it also introduces a barrier.
    @result Returns the original value referenced by <code>__theValue</code>.
 */
int32_t	OSAtomicOr32OrigBarrier( uint32_t __theMask, volatile uint32_t *__theValue ) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_3_2);




/*! @abstract Atomic bitwise AND of two 32-bit values.
    @discussion
	This function performs the bitwise AND of the value given by <code>__theMask</code>
	with the value in the memory location referenced by <code>__theValue</code>,
	storing the result back to that memory location atomically.
    @result Returns the new value.
 */
int32_t	OSAtomicAnd32( uint32_t __theMask, volatile uint32_t *__theValue ); 


/*! @abstract Atomic bitwise AND of two 32-bit values with barrier.
    @discussion
	This function performs the bitwise AND of the value given by <code>__theMask</code>
	with the value in the memory location referenced by <code>__theValue</code>,
	storing the result back to that memory location atomically.

	This function is equivalent to {@link OSAtomicAnd32}
	except that it also introduces a barrier.
    @result Returns the new value.
 */
int32_t	OSAtomicAnd32Barrier( uint32_t __theMask, volatile uint32_t *__theValue ); 


/*! @abstract Atomic bitwise AND of two 32-bit values returning original.
    @discussion
	This function performs the bitwise AND of the value given by <code>__theMask</code>
	with the value in the memory location referenced by <code>__theValue</code>,
	storing the result back to that memory location atomically.
    @result Returns the original value referenced by <code>__theValue</code>.
 */
int32_t	OSAtomicAnd32Orig( uint32_t __theMask, volatile uint32_t *__theValue ) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_3_2);


/*! @abstract Atomic bitwise AND of two 32-bit values returning original with barrier.
    @discussion
	This function performs the bitwise AND of the value given by <code>__theMask</code>
	with the value in the memory location referenced by <code>__theValue</code>,
	storing the result back to that memory location atomically.

	This function is equivalent to {@link OSAtomicAnd32Orig}
	except that it also introduces a barrier.
    @result Returns the original value referenced by <code>__theValue</code>.
 */
int32_t	OSAtomicAnd32OrigBarrier( uint32_t __theMask, volatile uint32_t *__theValue ) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_3_2);




/*! @abstract Atomic bitwise XOR of two 32-bit values.
    @discussion
	This function performs the bitwise XOR of the value given by <code>__theMask</code>
	with the value in the memory location referenced by <code>__theValue</code>,
	storing the result back to that memory location atomically.
    @result Returns the new value.
 */
int32_t	OSAtomicXor32( uint32_t __theMask, volatile uint32_t *__theValue );


/*! @abstract Atomic bitwise XOR of two 32-bit values with barrier.
    @discussion
	This function performs the bitwise XOR of the value given by <code>__theMask</code>
	with the value in the memory location referenced by <code>__theValue</code>,
	storing the result back to that memory location atomically.

	This function is equivalent to {@link OSAtomicXor32}
	except that it also introduces a barrier.
    @result Returns the new value.
 */
int32_t	OSAtomicXor32Barrier( uint32_t __theMask, volatile uint32_t *__theValue );


/*! @abstract Atomic bitwise XOR of two 32-bit values returning original.
    @discussion
	This function performs the bitwise XOR of the value given by <code>__theMask</code>
	with the value in the memory location referenced by <code>__theValue</code>,
	storing the result back to that memory location atomically.
    @result Returns the original value referenced by <code>__theValue</code>.
 */
int32_t	OSAtomicXor32Orig( uint32_t __theMask, volatile uint32_t *__theValue ) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_3_2);


/*! @abstract Atomic bitwise XOR of two 32-bit values returning original with barrier.
    @discussion
	This function performs the bitwise XOR of the value given by <code>__theMask</code>
	with the value in the memory location referenced by <code>__theValue</code>,
	storing the result back to that memory location atomically.

	This function is equivalent to {@link OSAtomicXor32Orig}
	except that it also introduces a barrier.
    @result Returns the original value referenced by <code>__theValue</code>.
 */
int32_t	OSAtomicXor32OrigBarrier( uint32_t __theMask, volatile uint32_t *__theValue ) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_3_2);
 

/*! @group Compare and swap
 * Functions in this group return true if the swap occured.  There are several versions,
 * depending on data type and on whether or not a barrier is used.
 */


/*! @abstract Compare and swap for 32-bit values.
    @discussion
	This function compares the value in <code>__oldValue</code> to the value
	in the memory location referenced by <code>__theValue</code>.  If the values
	match, this function stores the value from <code>__newValue</code> into
	that memory location atomically.
    @result Returns TRUE on a match, FALSE otherwise.
 */
bool    OSAtomicCompareAndSwap32( int32_t __oldValue, int32_t __newValue, volatile int32_t *__theValue );


/*! @abstract Compare and swap for 32-bit values with barrier.
    @discussion
	This function compares the value in <code>__oldValue</code> to the value
	in the memory location referenced by <code>__theValue</code>.  If the values
	match, this function stores the value from <code>__newValue</code> into
	that memory location atomically.

	This function is equivalent to {@link OSAtomicCompareAndSwap32}
	except that it also introduces a barrier.
    @result Returns TRUE on a match, FALSE otherwise.
 */
bool    OSAtomicCompareAndSwap32Barrier( int32_t __oldValue, int32_t __newValue, volatile int32_t *__theValue );


/*! @abstract Compare and swap pointers.
    @discussion
	This function compares the pointer stored in <code>__oldValue</code> to the pointer
	in the memory location referenced by <code>__theValue</code>.  If the pointers
	match, this function stores the pointer from <code>__newValue</code> into
	that memory location atomically.
    @result Returns TRUE on a match, FALSE otherwise.
 */
bool	OSAtomicCompareAndSwapPtr( void *__oldValue, void *__newValue, void * volatile *__theValue ) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);


/*! @abstract Compare and swap pointers with barrier.
    @discussion
	This function compares the pointer stored in <code>__oldValue</code> to the pointer
	in the memory location referenced by <code>__theValue</code>.  If the pointers
	match, this function stores the pointer from <code>__newValue</code> into
	that memory location atomically.

	This function is equivalent to {@link OSAtomicCompareAndSwapPtr}
	except that it also introduces a barrier.
    @result Returns TRUE on a match, FALSE otherwise.
 */
bool	OSAtomicCompareAndSwapPtrBarrier( void *__oldValue, void *__newValue, void * volatile *__theValue ) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);


/*! @abstract Compare and swap for <code>int</code> values.
    @discussion
	This function compares the value in <code>__oldValue</code> to the value
	in the memory location referenced by <code>__theValue</code>.  If the values
	match, this function stores the value from <code>__newValue</code> into
	that memory location atomically.

	This function is equivalent to {@link OSAtomicCompareAndSwap32}.
    @result Returns TRUE on a match, FALSE otherwise.
 */
bool	OSAtomicCompareAndSwapInt( int __oldValue, int __newValue, volatile int *__theValue ) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);


/*! @abstract Compare and swap for <code>int</code> values.
    @discussion
	This function compares the value in <code>__oldValue</code> to the value
	in the memory location referenced by <code>__theValue</code>.  If the values
	match, this function stores the value from <code>__newValue</code> into
	that memory location atomically.

	This function is equivalent to {@link OSAtomicCompareAndSwapInt}
	except that it also introduces a barrier.

	This function is equivalent to {@link OSAtomicCompareAndSwap32Barrier}.
    @result Returns TRUE on a match, FALSE otherwise.
 */
bool	OSAtomicCompareAndSwapIntBarrier( int __oldValue, int __newValue, volatile int *__theValue ) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);


/*! @abstract Compare and swap for <code>long</code> values.
    @discussion
	This function compares the value in <code>__oldValue</code> to the value
	in the memory location referenced by <code>__theValue</code>.  If the values
	match, this function stores the value from <code>__newValue</code> into
	that memory location atomically.

	This function is equivalent to {@link OSAtomicCompareAndSwap32} on 32-bit architectures, 
	or {@link OSAtomicCompareAndSwap64} on 64-bit architectures.
    @result Returns TRUE on a match, FALSE otherwise.
 */
bool	OSAtomicCompareAndSwapLong( long __oldValue, long __newValue, volatile long *__theValue ) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);


/*! @abstract Compare and swap for <code>long</code> values.
    @discussion
	This function compares the value in <code>__oldValue</code> to the value
	in the memory location referenced by <code>__theValue</code>.  If the values
	match, this function stores the value from <code>__newValue</code> into
	that memory location atomically.

	This function is equivalent to {@link OSAtomicCompareAndSwapLong}
	except that it also introduces a barrier.

	This function is equivalent to {@link OSAtomicCompareAndSwap32} on 32-bit architectures, 
	or {@link OSAtomicCompareAndSwap64} on 64-bit architectures.
    @result Returns TRUE on a match, FALSE otherwise.
 */
bool	OSAtomicCompareAndSwapLongBarrier( long __oldValue, long __newValue, volatile long *__theValue ) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);


/*! @abstract Compare and swap for <code>uint64_t</code> values.
    @discussion
	This function compares the value in <code>__oldValue</code> to the value
	in the memory location referenced by <code>__theValue</code>.  If the values
	match, this function stores the value from <code>__newValue</code> into
	that memory location atomically.
    @result Returns TRUE on a match, FALSE otherwise.
 */
bool    OSAtomicCompareAndSwap64( int64_t __oldValue, int64_t __newValue, volatile int64_t *__theValue );


/*! @abstract Compare and swap for <code>uint64_t</code> values.
    @discussion
	This function compares the value in <code>__oldValue</code> to the value
	in the memory location referenced by <code>__theValue</code>.  If the values
	match, this function stores the value from <code>__newValue</code> into
	that memory location atomically.

	This function is equivalent to {@link OSAtomicCompareAndSwap64}
	except that it also introduces a barrier.
    @result Returns TRUE on a match, FALSE otherwise.
 */
bool    OSAtomicCompareAndSwap64Barrier( int64_t __oldValue, int64_t __newValue, volatile int64_t *__theValue ) __OSX_AVAILABLE_STARTING(__MAC_10_4, __IPHONE_3_2);


/* Test and set.  They return the original value of the bit, and operate on bit (0x80>>(n&7))
 * in byte ((char*)theAddress + (n>>3)).
 */
/*! @abstract Atomic test and set
    @discussion
	This function tests a bit in the value referenced by <code>__theAddress</code>
	and if it is not set, sets it.  The bit is chosen by the value of <code>__n</code>.
	The bits are numbered in order beginning with bit 1 as the lowest order bit.

	For example, if <code>__theAddress</code> points to a 64-bit value,
	to compare the value of the highest bit, you would specify <code>64</code> for
	<code>__n</code>.
    @result
	Returns the original value of the bit being tested.
 */
bool    OSAtomicTestAndSet( uint32_t __n, volatile void *__theAddress );


/*! @abstract Atomic test and set with barrier
    @discussion
	This function tests a bit in the value referenced by <code>__theAddress</code>
	and if it is not set, sets it.  The bit is chosen by the value of <code>__n</code>.
	The bits are numbered in order beginning with bit 1 as the lowest order bit.

	For example, if <code>__theAddress</code> points to a 64-bit value,
	to compare the value of the highest bit, you would specify <code>64</code> for
	<code>__n</code>.

	This function is equivalent to {@link OSAtomicTestAndSet}
	except that it also introduces a barrier.
    @result
	Returns the original value of the bit being tested.
 */

bool    OSAtomicTestAndSetBarrier( uint32_t __n, volatile void *__theAddress );



/*! @abstract Atomic test and clear
    @discussion
	This function tests a bit in the value referenced by <code>__theAddress</code>
	and if it is not cleared, clears it.  The bit is chosen by the value of <code>__n</code>.
	The bits are numbered in order beginning with bit 1 as the lowest order bit.

	For example, if <code>__theAddress</code> points to a 64-bit value,
	to compare the value of the highest bit, you would specify <code>64</code> for
	<code>__n</code>.
    @result
	Returns the original value of the bit being tested.
 */
bool    OSAtomicTestAndClear( uint32_t __n, volatile void *__theAddress );


/*! @abstract Atomic test and clear
    @discussion
	This function tests a bit in the value referenced by <code>__theAddress</code>
	and if it is not cleared, clears it.  The bit is chosen by the value of <code>__n</code>.
	The bits are numbered in order beginning with bit 1 as the lowest order bit.

	For example, if <code>__theAddress</code> points to a 64-bit value,
	to compare the value of the highest bit, you would specify <code>64</code> for
	<code>__n</code>.

	This function is equivalent to {@link OSAtomicTestAndSet}
	except that it also introduces a barrier.
    @result
	Returns the original value of the bit being tested.
 */
bool    OSAtomicTestAndClearBarrier( uint32_t __n, volatile void *__theAddress );
 

/*! @group Spinlocks
 * These spinlocks use memory barriers as required to synchronize access to shared
 * memory protected by the lock.
 */

/*! @abstract The default value for an <code>OSSpinLock</code>.
    @discussion
	The convention is that unlocked is zero, locked is nonzero.
 */
#define	OS_SPINLOCK_INIT    0


/*! @abstract Data type for a spinlock.
    @discussion
	You should always initialize a spinlock to {@link OS_SPINLOCK_INIT} before
	using it.
 */
typedef int32_t OSSpinLock;


/*! @abstract Locks a spinlock if it would not block
    @result
	Returns <code>false</code> if the lock was already held by another thread,
	<code>true</code> if it took the lock successfully. 
 */
bool    OSSpinLockTry( volatile OSSpinLock *__lock );


/*! @abstract Locks a spinlock
    @discussion
	Although the lock operation spins, it employs various strategies
        to back off if the lock is held, making it immune to most priority-inversion
	livelocks.
 */
void    OSSpinLockLock( volatile OSSpinLock *__lock );


/*! @abstract Unlocks a spinlock */
void    OSSpinLockUnlock( volatile OSSpinLock *__lock );


/*! @group Lockless atomic enqueue and dequeue
 * These routines manipulate singly-linked LIFO lists.
 */

/*! @abstract The data structure for a queue head.
    @discussion
	You should always initialize a queue head structure with the
	initialization vector {@link OS_ATOMIC_QUEUE_INIT} before use.
 */
#if defined(__x86_64__)

typedef volatile struct {
	void	*opaque1;
	long	 opaque2;
} __attribute__ ((aligned (16))) OSQueueHead;

#else

typedef volatile struct {
	void	*opaque1;
	long	 opaque2;
} OSQueueHead;

#endif

/*! @abstract The initialization vector for a queue head. */
#define	OS_ATOMIC_QUEUE_INIT	{ NULL, 0 }

/*! @abstract Enqueue an item onto a list.
    @discussion
	Memory barriers are incorporated as needed to permit thread-safe access
	to the queue element.
    @param __list
	The list on which you want to enqueue the item.
    @param __new
	The item to add.
    @param __offset
	The "offset" parameter is the offset (in bytes) of the link field
	from the beginning of the data structure being queued (<code>__new</code>).
	The link field should be a pointer type.
	The <code>__offset</code> value needs to be same for all enqueuing and
	dequeuing operations on the same queue, even if different structure types
	are enqueued on that queue.  The use of <code>offsetset()</code>, defined in
	<code>stddef.h</code> is the common way to specify the <code>__offset</code>
	value.
 */
void  OSAtomicEnqueue( OSQueueHead *__list, void *__new, size_t __offset) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_4_0);


/*! @abstract Dequeue an item from a list.
    @discussion
	Memory barriers are incorporated as needed to permit thread-safe access
	to the queue element.
    @param __list
	The list on which you want to enqueue the item.
    @param __offset
	The "offset" parameter is the offset (in bytes) of the link field
	from the beginning of the data structure being queued (<code>__new</code>).
	The link field should be a pointer type.
	The <code>__offset</code> value needs to be same for all enqueuing and
	dequeuing operations on the same queue, even if different structure types
	are enqueued on that queue.  The use of <code>offsetset()</code>, defined in
	<code>stddef.h</code> is the common way to specify the <code>__offset</code>
	value.
    @result
	Returns the most recently enqueued element, or <code>NULL</code> if the
	list is empty.  
 */
void* OSAtomicDequeue( OSQueueHead *__list, size_t __offset) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_4_0);

#if defined(__x86_64__) || defined(__i386__)

/*! @group Lockless atomic fifo enqueue and dequeue
 * These routines manipulate singly-linked FIFO lists.
 */

/*! @abstract The data structure for a fifo queue head.
    @discussion
	You should always initialize a fifo queue head structure with the
	initialization vector {@link OS_ATOMIC_FIFO_QUEUE_INIT} before use.
 */
#if defined(__x86_64__)

typedef	volatile struct {
	void	*opaque1;
	void	*opaque2;
	int	 opaque3;
} __attribute__ ((aligned (16))) OSFifoQueueHead;

#else

typedef	volatile struct {
	void	*opaque1;
	void	*opaque2;
	int	 opaque3;
} OSFifoQueueHead;

#endif

/*! @abstract The initialization vector for a fifo queue head. */
#define OS_ATOMIC_FIFO_QUEUE_INIT   { NULL, NULL, 0 }

/*! @abstract Enqueue an item onto a list.
    @discussion
	Memory barriers are incorporated as needed to permit thread-safe access
	to the queue element.
    @param __list
	The list on which you want to enqueue the item.
    @param __new
	The item to add.
    @param __offset
	The "offset" parameter is the offset (in bytes) of the link field
	from the beginning of the data structure being queued (<code>__new</code>).
	The link field should be a pointer type.
	The <code>__offset</code> value needs to be same for all enqueuing and
	dequeuing operations on the same queue, even if different structure types
	are enqueued on that queue.  The use of <code>offsetset()</code>, defined in
	<code>stddef.h</code> is the common way to specify the <code>__offset</code>
	value.
 */
void  OSAtomicFifoEnqueue( OSFifoQueueHead *__list, void *__new, size_t __offset) __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);

/*! @abstract Dequeue an item from a list.
    @discussion
	Memory barriers are incorporated as needed to permit thread-safe access
	to the queue element.
    @param __list
	The list on which you want to enqueue the item.
    @param __offset
	The "offset" parameter is the offset (in bytes) of the link field
	from the beginning of the data structure being queued (<code>__new</code>).
	The link field should be a pointer type.
	The <code>__offset</code> value needs to be same for all enqueuing and
	dequeuing operations on the same queue, even if different structure types
	are enqueued on that queue.  The use of <code>offsetset()</code>, defined in
	<code>stddef.h</code> is the common way to specify the <code>__offset</code>
	value.
    @result
	Returns the oldest enqueued element, or <code>NULL</code> if the
	list is empty.  
 */
void* OSAtomicFifoDequeue( OSFifoQueueHead *__list, size_t __offset) __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);

#endif /* __i386__ || __x86_64__ */

/*! @group Memory barriers */

/*! @abstract Memory barrier.
    @discussion
	This function serves as both a read and write barrier.
 */
void    OSMemoryBarrier( void );

__END_DECLS

#endif /* _OSATOMIC_H_ */
