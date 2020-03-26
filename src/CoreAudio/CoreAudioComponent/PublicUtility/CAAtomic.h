/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
Part of Core Audio Public Utility Classes
*/

/*
	This file implements all Atomic operations using Interlocked functions specified in
	Winbase.h
NOTE: According to Microsoft documentation, all Interlocked functions generates a
full barrier. 
	On Windows:
	As the Interlocked functions returns the Old value, Extra checks and operations 
	are made after the atomic operation to return value consistent with OSX counterparts.
*/

#ifndef __CAAtomic_h__
#define __CAAtomic_h__

#if TARGET_OS_WIN32
	#include <windows.h>
	#include <intrin.h>
	#pragma intrinsic(_InterlockedOr)
	#pragma intrinsic(_InterlockedAnd)
#else
	#include <CoreFoundation/CFBase.h>
	#include <libkern/OSAtomic.h>
#endif

inline void CAMemoryBarrier() 
{
#if TARGET_OS_WIN32
	MemoryBarrier();
#else
	OSMemoryBarrier();
#endif
}

inline SInt32 CAAtomicAdd32Barrier(SInt32 theAmt, volatile SInt32* theValue)
{
#if TARGET_OS_WIN32
	long lRetVal = InterlockedExchangeAdd((volatile long*)theValue, theAmt);
	// InterlockedExchangeAdd returns the original value which differs from OSX version. 
	// At this point the addition would have occured and hence returning the new value
	// to keep it sync with OSX.
	return lRetVal + theAmt;
#else
	return OSAtomicAdd32Barrier(theAmt, (volatile int32_t *)theValue);
#endif
}

inline SInt32 CAAtomicOr32Barrier(UInt32 theMask, volatile UInt32* theValue)
{
#if TARGET_OS_WIN32
	// InterlockedAnd macro is not defined in x86 platform, and hence using the intrinsic
	// function instead.
	long j = _InterlockedOr((volatile long*)theValue, theMask);
	// _InterlockedOr returns the original value which differs from OSX version.
	// Returning the new value similar to OSX
	return (SInt32)(j | theMask);
#else
	return OSAtomicOr32Barrier(theMask, (volatile uint32_t *)theValue);
#endif
}

inline SInt32 CAAtomicAnd32Barrier(UInt32 theMask, volatile UInt32* theValue)
{
#if TARGET_OS_WIN32
// InterlockedAnd macro is not defined in x86 platform, and hence using the intrinsic
// function instead.
	long j = _InterlockedAnd((volatile long*)theValue, theMask);
	// _InterlockedAnd returns the original value which differs from OSX version.
	// Returning the new value similar to OSX
	return (SInt32)(j & theMask);
#else
	return OSAtomicAnd32Barrier(theMask, (volatile uint32_t *)theValue);
#endif
}

inline bool CAAtomicCompareAndSwap32Barrier(SInt32 oldValue, SInt32 newValue, volatile SInt32 *theValue)
{
#if TARGET_OS_WIN32
	// InterlockedCompareExchange returns the old value. But we need to return bool value.
	long lRetVal = InterlockedCompareExchange((volatile long*)theValue, newValue, oldValue);
// Hence we check if the new value is set and if it is we return true else false.
// If theValue is equal to oldValue then the swap happens. Otherwise swap doesn't happen.
	return (oldValue == lRetVal);
#else
	return OSAtomicCompareAndSwap32Barrier(oldValue, newValue, (volatile int32_t *)theValue);
#endif
}


inline SInt32 CAAtomicIncrement32(volatile SInt32* theValue)
{
#if TARGET_OS_WIN32
	return (SInt32)InterlockedIncrement((volatile long*)theValue);
#else
	return OSAtomicIncrement32((volatile int32_t *)theValue);
#endif
}

inline SInt32 CAAtomicDecrement32(volatile SInt32* theValue)
{
#if TARGET_OS_WIN32
	return (SInt32)InterlockedDecrement((volatile long*)theValue);
#else
	return OSAtomicDecrement32((volatile int32_t *)theValue);
#endif
}

inline SInt32 CAAtomicIncrement32Barrier(volatile SInt32* theValue)
{
#if TARGET_OS_WIN32
	return CAAtomicIncrement32(theValue);
#else
	return OSAtomicIncrement32Barrier((volatile int32_t *)theValue);
#endif
}

inline SInt32 CAAtomicDecrement32Barrier(volatile SInt32* theValue)
{
#if TARGET_OS_WIN32
	return CAAtomicDecrement32(theValue);
#else
	return OSAtomicDecrement32Barrier((volatile int32_t *)theValue);
#endif
}

inline bool CAAtomicTestAndClearBarrier(int bitToClear, void* theAddress)
{
#if TARGET_OS_WIN32
	BOOL bOldVal = InterlockedBitTestAndReset((long*)theAddress, bitToClear);
	return (bOldVal ? true : false);
#else
	return OSAtomicTestAndClearBarrier(bitToClear, (volatile void *)theAddress);
#endif
}

inline bool CAAtomicTestAndClear(int bitToClear, void* theAddress)
{
#if TARGET_OS_WIN32
	BOOL bOldVal = CAAtomicTestAndClearBarrier(bitToClear, (long*)theAddress);
	return (bOldVal ? true : false);
#else
	return OSAtomicTestAndClear(bitToClear, (volatile void *)theAddress);
#endif
}

inline bool CAAtomicTestAndSetBarrier(int bitToSet, void* theAddress)
{
#if TARGET_OS_WIN32
	BOOL bOldVal = InterlockedBitTestAndSet((long*)theAddress, bitToSet);
	return (bOldVal ? true : false);
#else
	return OSAtomicTestAndSetBarrier(bitToSet, (volatile void *)theAddress);
#endif
}

// int32_t flavors -- for C++ only since we can't overload in C
// CFBase.h defines SInt32 as signed int which is similar to int32_t. If CFBase.h is included, then
// this will generate redefinition error. But on Mac, CFBase.h, still includes MacTypes.h where
// SInt32 is defined as signed long so this would work there.
// So in order to fix the redefinition errors, we define these functions only if MacTypes.h is included.
#if defined(__cplusplus) && defined(__MACTYPES__) && !__LP64__
inline int32_t CAAtomicAdd32Barrier(int32_t theAmt, volatile int32_t* theValue)
{
	return CAAtomicAdd32Barrier(theAmt, (volatile SInt32 *)theValue);
}

inline int32_t CAAtomicOr32Barrier(uint32_t theMask, volatile uint32_t* theValue)
{
	return CAAtomicOr32Barrier(theMask, (volatile UInt32 *)theValue);
}

inline int32_t CAAtomicAnd32Barrier(uint32_t theMask, volatile uint32_t* theValue)
{
	return CAAtomicAnd32Barrier(theMask, (volatile UInt32 *)theValue);
}

inline bool CAAtomicCompareAndSwap32Barrier(int32_t oldValue, int32_t newValue, volatile int32_t *theValue)
{
	return CAAtomicCompareAndSwap32Barrier(oldValue, newValue, (volatile SInt32 *)theValue);
}

inline int32_t CAAtomicIncrement32(volatile int32_t* theValue)
{
	return CAAtomicIncrement32((volatile SInt32 *)theValue);
}

inline int32_t CAAtomicDecrement32(volatile int32_t* theValue)
{
	return CAAtomicDecrement32((volatile SInt32 *)theValue);
}

inline int32_t CAAtomicIncrement32Barrier(volatile int32_t* theValue)
{
	return CAAtomicIncrement32Barrier((volatile SInt32 *)theValue);
}

inline int32_t CAAtomicDecrement32Barrier(volatile int32_t* theValue)
{
	return CAAtomicDecrement32Barrier((volatile SInt32 *)theValue);
}
#endif // __cplusplus && !__LP64__

#if __LP64__
inline bool CAAtomicCompareAndSwap64Barrier( int64_t __oldValue, int64_t __newValue, volatile int64_t *__theValue )
{
	return OSAtomicCompareAndSwap64Barrier(__oldValue, __newValue, __theValue);
}
#endif

inline bool CAAtomicCompareAndSwapPtrBarrier(void *__oldValue, void *__newValue, volatile void ** __theValue)
{
#if __LP64__
	return CAAtomicCompareAndSwap64Barrier((int64_t)__oldValue, (int64_t)__newValue, (int64_t *)__theValue);
#else
	return CAAtomicCompareAndSwap32Barrier((int32_t)__oldValue, (int32_t)__newValue, (int32_t *)__theValue);
#endif
}

/* Spinlocks.  These use memory barriers as required to synchronize access to shared
 * memory protected by the lock.  The lock operation spins, but employs various strategies
 * to back off if the lock is held, making it immune to most priority-inversion livelocks.
 * The try operation immediately returns false if the lock was held, true if it took the
 * lock.  The convention is that unlocked is zero, locked is nonzero.
 */
#define	CA_SPINLOCK_INIT    0

typedef int32_t CASpinLock;

bool    CASpinLockTry( volatile CASpinLock *__lock );
void    CASpinLockLock( volatile CASpinLock *__lock );
void    CASpinLockUnlock( volatile CASpinLock *__lock );

inline void    CASpinLockLock( volatile CASpinLock *__lock )
{
#if TARGET_OS_MAC
	OSSpinLockLock(__lock);
#else
	while (CAAtomicTestAndSetBarrier(0, (void*)__lock))
		usleep(1000); // ???
#endif
}

inline void    CASpinLockUnlock( volatile CASpinLock *__lock )
{
#if TARGET_OS_MAC
	OSSpinLockUnlock(__lock);
#else
	CAAtomicTestAndClearBarrier(0, (void*)__lock);
#endif
}

inline bool    CASpinLockTry( volatile CASpinLock *__lock )
{
#if TARGET_OS_MAC
	return OSSpinLockTry(__lock);
#else
	return (CAAtomicTestAndSetBarrier(0, (void*)__lock) == 0);
#endif
}


#endif // __CAAtomic_h__
