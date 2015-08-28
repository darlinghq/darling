#include "OSAtomic.h"
#include "libc-stub.h"
#include <cstdio>
#include <cassert>
#include <sched.h>

int32_t OSAtomicAdd32(int32_t theAmount, volatile int32_t *theValue)
{
	return __sync_add_and_fetch(theValue, theAmount);
}

int32_t OSAtomicAdd32Barrier(int32_t theAmount, volatile int32_t *theValue)
{
	return __sync_add_and_fetch(theValue, theAmount);
}

int32_t OSAtomicIncrement32(volatile int32_t *theValue)
{
	return OSAtomicAdd32(1, theValue);
}

int32_t OSAtomicIncrement32Barrier(volatile int32_t *theValue)
{
	return OSAtomicAdd32Barrier(1, theValue);
}

int32_t OSAtomicDecrement32(volatile int32_t *theValue)
{
	return OSAtomicAdd32(-1, theValue);
}

int32_t OSAtomicDecrement32Barrier(volatile int32_t *theValue)
{
	return OSAtomicAdd32Barrier(-1, theValue);
}

int32_t OSAtomicOr32(uint32_t theMask, volatile uint32_t *theValue)
{
	return __sync_or_and_fetch(theValue, theMask);
}

int32_t OSAtomicOr32Barrier(uint32_t theMask, volatile uint32_t *theValue)
{
	return __sync_or_and_fetch(theValue, theMask);
}

int32_t OSAtomicAnd32(uint32_t theMask, volatile uint32_t *theValue)
{
	return __sync_and_and_fetch(theValue, theMask);
}

int32_t OSAtomicAnd32Barrier(uint32_t theMask, volatile uint32_t *theValue)
{
	return __sync_and_and_fetch(theValue, theMask);
}

int32_t OSAtomicXor32(uint32_t theMask, volatile uint32_t *theValue)
{
	return __sync_xor_and_fetch(theValue, theMask);
}

int32_t OSAtomicXor32Barrier(uint32_t theMask, volatile uint32_t *theValue)
{
	return __sync_xor_and_fetch(theValue, theMask);
}

int32_t OSAtomicOr32Orig(uint32_t theMask, volatile uint32_t *theValue)
{
	return __sync_fetch_and_or(theValue, theMask);
}

int32_t OSAtomicOr32OrigBarrier(uint32_t theMask, volatile uint32_t *theValue)
{
	return __sync_fetch_and_or(theValue, theMask);
}

int32_t OSAtomicAnd32Orig(uint32_t theMask, volatile uint32_t *theValue)
{
	return __sync_fetch_and_and(theValue, theMask);
}

int32_t OSAtomicAnd32OrigBarrier(uint32_t theMask, volatile uint32_t *theValue)
{
	return __sync_fetch_and_and(theValue, theMask);
}

int32_t OSAtomicXor32Orig(uint32_t theMask, volatile uint32_t *theValue)
{
	return __sync_fetch_and_xor(theValue, theMask);
}

int32_t OSAtomicXor32OrigBarrier(uint32_t theMask, volatile uint32_t *theValue)
{
	return __sync_fetch_and_xor(theValue, theMask);
}

int64_t OSAtomicAdd64(int64_t theAmount, volatile int64_t *theValue)
{
	return __sync_add_and_fetch(theValue, theAmount);
}

int64_t OSAtomicAdd64Barrier(int64_t theAmount, volatile int64_t *theValue)
{
	return __sync_add_and_fetch(theValue, theAmount);
}

int64_t OSAtomicIncrement64(volatile int64_t *theValue)
{
	return OSAtomicAdd64(1, theValue);
}

int64_t OSAtomicIncrement64Barrier(volatile int64_t *theValue)
{
	return OSAtomicAdd64Barrier(1, theValue);
}

int64_t OSAtomicDecrement64(volatile int64_t *theValue)
{
	return OSAtomicAdd64(-1, theValue);
}

int64_t OSAtomicDecrement64Barrier(volatile int64_t *theValue)
{
	return OSAtomicAdd64Barrier(-1, theValue);
}

bool OSAtomicCompareAndSwapInt(int oldValue, int newValue, volatile int *theValue)
{
	return __sync_bool_compare_and_swap(theValue, oldValue, newValue);
}

bool OSAtomicCompareAndSwapIntBarrier(int oldValue, int newValue, volatile int *theValue)
{
	return __sync_bool_compare_and_swap(theValue, oldValue, newValue);
}

bool OSAtomicCompareAndSwapLong(long oldValue, long newValue, volatile long *theValue)
{
	return __sync_bool_compare_and_swap(theValue, oldValue, newValue);
}

bool OSAtomicCompareAndSwapLongBarrier(long oldValue, long newValue, volatile long *theValue)
{
	return __sync_bool_compare_and_swap(theValue, oldValue, newValue);
}

bool OSAtomicCompareAndSwapPtr(void* oldValue, void* newValue, void* volatile *theValue)
{
	return __sync_bool_compare_and_swap(theValue, oldValue, newValue);
}

bool OSAtomicCompareAndSwapPtrBarrier(void* oldValue, void* newValue, void* volatile *theValue)
{
	return __sync_bool_compare_and_swap(theValue, oldValue, newValue);
}

bool OSAtomicCompareAndSwap32(int32_t oldValue, int32_t newValue, volatile int32_t *theValue)
{
	return __sync_bool_compare_and_swap(theValue, oldValue, newValue);
}

bool OSAtomicCompareAndSwap32Barrier(int32_t oldValue, int32_t newValue, volatile int32_t *theValue)
{
	return __sync_bool_compare_and_swap(theValue, oldValue, newValue);
}

bool OSAtomicCompareAndSwap64(int64_t oldValue, int64_t newValue, volatile int64_t *theValue)
{
	return __sync_bool_compare_and_swap(theValue, oldValue, newValue);
}

bool OSAtomicCompareAndSwap64Barrier(int64_t oldValue, int64_t newValue, volatile int64_t *theValue)
{
	return __sync_bool_compare_and_swap(theValue, oldValue, newValue);
}

bool OSAtomicTestAndSet(uint32_t n, volatile void *theAddress)
{
	LIBC_STUB();
	return false;
}

bool OSAtomicTestAndSetBarrier(uint32_t n, volatile void *theAddress)
{
	LIBC_STUB();
	return false;
}

bool OSAtomicTestAndClear(uint32_t n, volatile void *theAddress)
{
	LIBC_STUB();
	return false;
}

bool OSAtomicTestAndClearBarrier(uint32_t n, volatile void *theAddress)
{
	LIBC_STUB();
	return false;
}

bool OSSpinLockTry(OSSpinLock *lock)
{
	return __sync_bool_compare_and_swap(lock, 0, 1);
}

void OSSpinLockLock(OSSpinLock *lock)
{
	// We cannot use pthread spinlocks, these need init and destruction
	while (!OSSpinLockTry(lock))
		sched_yield();
}

void OSSpinLockUnlock(OSSpinLock *lock)
{
	bool success = __sync_bool_compare_and_swap(lock, 1, 0);
	assert(success); // Will fail if spinlock wasn't locked
}

// http://i.stack.imgur.com/FSBA3.png
void OSAtomicEnqueue(OSQueueHead *list, void *_new, size_t offset)
{
	/*
	long* newNext = reinterpret_cast<long*>(reinterpret_cast<char*>(_new) + offset);
	*newNext = 0;

	// If the queue is not empty
	if (!__sync_bool_compare_and_swap(list->p, 0, _new))
	{
		void* prevTop = __sync_lock_test_and_set(list->p, _new);
		*newNext = prevTop; // FIXME: this place may cause races with OSAtomicDequeue
	}
	*/
	LIBC_STUB();
}

void* OSAtomicDequeue(OSQueueHead *list, size_t offset)
{
	LIBC_STUB();
	return nullptr;
}

void OSMemoryBarrier()
{
	__atomic_thread_fence(__ATOMIC_SEQ_CST);
}

