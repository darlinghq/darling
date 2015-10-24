/*
 * Copyright (c) 2004, 2008 Apple Inc. All rights reserved.
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

#include <libkern/OSAtomic.h>
#include <arm/arch.h>


#if !defined(_ARM_ARCH_6)

/*
 * The only atomic operation ARMv4T provides (atomic swap) is not
 * sufficient for the general 32-bit arithmetic and compare-and-swap
 * operations OSAtomic is supposed to provide.  So we use a global
 * spin lock around all operations.
 *
 * Since we have only a single, in-order, CPU, we do not need
 * memory barriers for data.
 */

static OSSpinLock _atomic_lock = OS_SPINLOCK_INIT;

int32_t	OSAtomicAdd32( int32_t theAmount, volatile int32_t *theValue )
{
    int32_t result;

    OSSpinLockLock(&_atomic_lock);
    result = (*theValue += theAmount);
    OSSpinLockUnlock(&_atomic_lock);

    return result;
}

int32_t	OSAtomicAdd32Barrier( int32_t theAmount, volatile int32_t *theValue )
{
    return OSAtomicAdd32(theAmount, theValue);
}

int64_t	OSAtomicAdd64( int64_t theAmount, volatile int64_t *theValue )
{
    int64_t result;

    OSSpinLockLock(&_atomic_lock);
    result = (*theValue += theAmount);
    OSSpinLockUnlock(&_atomic_lock);

    return result;
}

int64_t	OSAtomicAdd64Barrier( int64_t theAmount, volatile int64_t *theValue )
{
    return OSAtomicAdd64(theAmount, theValue);
}

int32_t	OSAtomicOr32( uint32_t theMask, volatile uint32_t *theValue )
{
    int32_t result;

    OSSpinLockLock(&_atomic_lock);
    result = (*theValue |= theMask);
    OSSpinLockUnlock(&_atomic_lock);

    return result;
}

int32_t	OSAtomicOr32Barrier( uint32_t theMask, volatile uint32_t *theValue )
{
    return OSAtomicOr32(theMask, theValue);
}

int32_t	OSAtomicOr32Orig( uint32_t theMask, volatile uint32_t *theValue )
{
    int32_t result;

    OSSpinLockLock(&_atomic_lock);
    result = *theValue;
    *theValue |= theMask;
    OSSpinLockUnlock(&_atomic_lock);

    return result;
}

int32_t	OSAtomicOr32OrigBarrier( uint32_t theMask, volatile uint32_t *theValue )
{
    return OSAtomicOr32Orig(theMask, theValue);
}

int32_t	OSAtomicAnd32( uint32_t theMask, volatile uint32_t *theValue )
{
    int32_t result;

    OSSpinLockLock(&_atomic_lock);
    result = (*theValue &= theMask);
    OSSpinLockUnlock(&_atomic_lock);

    return result;
}

int32_t	OSAtomicAnd32Barrier( uint32_t theMask, volatile uint32_t *theValue )
{
    return OSAtomicAnd32(theMask, theValue);
}

int32_t	OSAtomicAnd32Orig( uint32_t theMask, volatile uint32_t *theValue )
{
    int32_t result;

    OSSpinLockLock(&_atomic_lock);
    result = *theValue;
    *theValue &= theMask;
    OSSpinLockUnlock(&_atomic_lock);

    return result;
}

int32_t	OSAtomicAnd32OrigBarrier( uint32_t theMask, volatile uint32_t *theValue )
{
    return OSAtomicAnd32Orig(theMask, theValue);
}

int32_t	OSAtomicXor32( uint32_t theMask, volatile uint32_t *theValue )
{
    int32_t result;

    OSSpinLockLock(&_atomic_lock);
    result = (*theValue ^= theMask);
    OSSpinLockUnlock(&_atomic_lock);

    return result;
}

int32_t	OSAtomicXor32Barrier( uint32_t theMask, volatile uint32_t *theValue )
{
    return OSAtomicXor32(theMask, theValue);
}

int32_t	OSAtomicXor32Orig( uint32_t theMask, volatile uint32_t *theValue )
{
    int32_t result;

    OSSpinLockLock(&_atomic_lock);
    result = *theValue;
    *theValue ^= theMask;
    OSSpinLockUnlock(&_atomic_lock);

    return result;
}

int32_t	OSAtomicXor32OrigBarrier( uint32_t theMask, volatile uint32_t *theValue )
{
    return OSAtomicXor32Orig(theMask, theValue);
}

bool	OSAtomicCompareAndSwap32( int32_t oldValue, int32_t newValue, volatile int32_t *theValue )
{
    bool result;
    
    OSSpinLockLock(&_atomic_lock);
    result = (*theValue == oldValue);
    if (result) *theValue = newValue;
    OSSpinLockUnlock(&_atomic_lock);

    return result;
}

bool    OSAtomicCompareAndSwap32Barrier( int32_t oldValue, int32_t newValue, volatile int32_t *theValue )
{
    return OSAtomicCompareAndSwap32(oldValue, newValue, theValue);
}

bool
OSAtomicCompareAndSwapInt(int oldValue, int newValue, volatile int *theValue)
{
	return OSAtomicCompareAndSwap32(oldValue, newValue, theValue);
}

bool
OSAtomicCompareAndSwapIntBarrier(int oldValue, int newValue, volatile int *theValue)
{
	return OSAtomicCompareAndSwap32(oldValue, newValue, theValue);
}

bool
OSAtomicCompareAndSwapLong(long oldValue, long newValue, volatile long *theValue)
{
	return OSAtomicCompareAndSwap32(oldValue, newValue, (volatile int32_t *)theValue);
}

bool
OSAtomicCompareAndSwapLongBarrier(long oldValue, long newValue, volatile long *theValue)
{
	return OSAtomicCompareAndSwap32(oldValue, newValue, (volatile int32_t *)theValue);
}

bool	OSAtomicCompareAndSwap64( int64_t oldValue, int64_t newValue, volatile int64_t *theValue )
{
    bool result;
    
    OSSpinLockLock(&_atomic_lock);
    result = (*theValue == oldValue);
    if (result) *theValue = newValue;
    OSSpinLockUnlock(&_atomic_lock);

    return result;
}

bool    OSAtomicCompareAndSwap64Barrier( int64_t oldValue, int64_t newValue, volatile int64_t *theValue )
{
    return OSAtomicCompareAndSwap64(oldValue, newValue, theValue);
}

bool    OSAtomicTestAndSet( uint32_t n, volatile void *theAddress )
{
    char *byteAddress = ((char*)theAddress + (n>>3));
    uint32_t byteBit = (0x80>>(n&7));
    bool result;

    OSSpinLockLock(&_atomic_lock);
    result = *byteAddress & byteBit;
    *byteAddress |= byteBit;
    OSSpinLockUnlock(&_atomic_lock);

    return result;
}

bool    OSAtomicTestAndSetBarrier( uint32_t n, volatile void *theAddress )
{
    return OSAtomicTestAndSet(n, theAddress);
}

bool    OSAtomicTestAndClear( uint32_t n, volatile void *theAddress )
{
    char *byteAddress = ((char*)theAddress + (n>>3));
    uint32_t byteBit = (0x80>>(n&7));
    bool result;

    OSSpinLockLock(&_atomic_lock);
    result = *byteAddress & byteBit;
    *byteAddress &= (~byteBit);
    OSSpinLockUnlock(&_atomic_lock);

    return result;
}

bool    OSAtomicTestAndClearBarrier( uint32_t n, volatile void *theAddress )
{
    return OSAtomicTestAndClear(n, theAddress);
}

void    OSMemoryBarrier( void )
{
    return;
}


bool    OSAtomicCompareAndSwapPtrBarrier( void *__oldValue, void *__newValue, void * volatile *__theValue )
{
    return OSAtomicCompareAndSwapPtr(__oldValue, __newValue, __theValue);
}

bool    OSAtomicCompareAndSwapPtr( void *__oldValue, void *__newValue, void * volatile *__theValue )
{
    bool result;
    
    OSSpinLockLock(&_atomic_lock);
    result = (*__theValue == __oldValue);
    if (result) *__theValue = __newValue;
    OSSpinLockUnlock(&_atomic_lock);

    return result;
}

void    OSAtomicEnqueue( OSQueueHead *__list, void *__new, size_t __offset )
{
    OSSpinLockLock(&_atomic_lock);
    *((void **)((char *)__new + __offset)) = __list->opaque1;
    __list->opaque1 = __new;
    OSSpinLockUnlock(&_atomic_lock);
}

void*   OSAtomicDequeue( OSQueueHead *__list, size_t __offset )
{
    void *head;
    
    OSSpinLockLock(&_atomic_lock);
    head = __list->opaque1;
    if (head != NULL) {
      void **next = (void **)((char *)head + __offset);
        __list->opaque1 = *next;
    }
    OSSpinLockUnlock(&_atomic_lock);

    return head;
}

#endif /* !defined(_ARM_ARCH_6) */
