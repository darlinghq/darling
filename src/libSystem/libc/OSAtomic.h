#ifndef OSATOMIC_H
#define OSATOMIC_H
#include <stdint.h>
#include <pthread.h>

typedef pthread_spinlock_t OSSpinLock;
struct OSQueueHead
{
    void* p;
    long l;
};

#ifdef __cplusplus
extern "C" {
#endif

int32_t OSAtomicAdd32(int32_t theAmount, volatile int32_t *theValue);

int32_t OSAtomicAdd32Barrier(int32_t theAmount, volatile int32_t *theValue);

int32_t OSAtomicIncrement32(volatile int32_t *theValue);

int32_t OSAtomicIncrement32Barrier(volatile int32_t *theValue);

int32_t OSAtomicDecrement32(volatile int32_t *theValue);

int32_t OSAtomicDecrement32Barrier(volatile int32_t *theValue);

int32_t OSAtomicOr32(uint32_t theMask, volatile uint32_t *theValue);

int32_t OSAtomicOr32Barrier(uint32_t theMask, volatile uint32_t *theValue);

int32_t OSAtomicAnd32(uint32_t theMask, volatile uint32_t *theValue);

int32_t OSAtomicAnd32Barrier(uint32_t theMask, volatile uint32_t *theValue);

int32_t OSAtomicXor32(uint32_t theMask, volatile uint32_t *theValue);

int32_t OSAtomicXor32Barrier(uint32_t theMask, volatile uint32_t *theValue);

int32_t OSAtomicOr32Orig(uint32_t theMask, volatile uint32_t *theValue);

int32_t OSAtomicOr32OrigBarrier(uint32_t theMask, volatile uint32_t *theValue);

int32_t OSAtomicAnd32Orig(uint32_t theMask, volatile uint32_t *theValue);

int32_t OSAtomicAnd32OrigBarrier(uint32_t theMask, volatile uint32_t *theValue);

int32_t OSAtomicXor32Orig(uint32_t theMask, volatile uint32_t *theValue);

int32_t OSAtomicXor32OrigBarrier(uint32_t theMask, volatile uint32_t *theValue);

int64_t OSAtomicAdd64(int64_t theAmount, volatile int64_t *theValue);

int64_t OSAtomicAdd64Barrier(int64_t theAmount, volatile int64_t *theValue);

int64_t OSAtomicIncrement64(volatile int64_t *theValue);

int64_t OSAtomicIncrement64Barrier(volatile int64_t *theValue);

int64_t OSAtomicDecrement64(volatile int64_t *theValue);

int64_t OSAtomicDecrement64Barrier(volatile int64_t *theValue);

bool OSAtomicCompareAndSwapInt(int oldValue, int newValue, volatile int *theValue);

bool OSAtomicCompareAndSwapIntBarrier(int oldValue, int newValue, volatile int *theValue);

bool OSAtomicCompareAndSwapLong(long oldValue, long newValue, volatile long *theValue);

bool OSAtomicCompareAndSwapLongBarrier(long oldValue, long newValue, volatile long *theValue);

bool OSAtomicCompareAndSwapPtr(void* oldValue, void* newValue, void* volatile *theValue);

bool OSAtomicCompareAndSwapPtrBarrier(void* oldValue, void* newValue, void* volatile *theValue);

bool OSAtomicCompareAndSwap32(int32_t oldValue, int32_t newValue, volatile int32_t *theValue);

bool OSAtomicCompareAndSwap32Barrier(int32_t oldValue, int32_t newValue, volatile int32_t *theValue);

bool OSAtomicCompareAndSwap64(int64_t oldValue, int64_t newValue, volatile int64_t *theValue);

bool OSAtomicCompareAndSwap64Barrier(int64_t oldValue, int64_t newValue, volatile int64_t *theValue);

bool OSAtomicTestAndSet(uint32_t n, volatile void *theAddress);

bool OSAtomicTestAndSetBarrier(uint32_t n, volatile void *theAddress);

bool OSAtomicTestAndClear(uint32_t n, volatile void *theAddress);

bool OSAtomicTestAndClearBarrier(uint32_t n, volatile void *theAddress);

bool OSSpinLockTry(OSSpinLock *lock);

void OSSpinLockLock(OSSpinLock *lock);

void OSSpinLockUnlock(OSSpinLock *lock);

void OSAtomicEnqueue(OSQueueHead *list, void *_new, size_t offset);

void* OSAtomicDequeue(OSQueueHead *list, size_t offset);

#ifdef __cplusplus
}
#endif

#endif

