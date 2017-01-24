/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

#include "os/internal.h"
#include "libkern/OSAtomic.h"
#include "resolver.h"

#if TARGET_OS_EMBEDDED

OS_ATOMIC_EXPORT
int32_t OSAtomicAdd32Barrier(int32_t v, volatile int32_t *p);
OS_ATOMIC_EXPORT
int32_t OSAtomicIncrement32Barrier(volatile int32_t *p);
OS_ATOMIC_EXPORT
int32_t OSAtomicDecrement32Barrier(volatile int32_t *p);
OS_ATOMIC_EXPORT
int64_t OSAtomicAdd64Barrier(int64_t v, volatile int64_t *p);
OS_ATOMIC_EXPORT
int64_t OSAtomicIncrement64Barrier(volatile int64_t *p);
OS_ATOMIC_EXPORT
int64_t OSAtomicDecrement64Barrier(volatile int64_t *p);
OS_ATOMIC_EXPORT
int32_t OSAtomicAnd32Barrier(uint32_t v, volatile uint32_t *p);
OS_ATOMIC_EXPORT
int32_t OSAtomicAnd32OrigBarrier(uint32_t v, volatile uint32_t *p);
OS_ATOMIC_EXPORT
int32_t OSAtomicOr32Barrier(uint32_t v, volatile uint32_t *p);
OS_ATOMIC_EXPORT
int32_t OSAtomicOr32OrigBarrier(uint32_t v, volatile uint32_t *p);
OS_ATOMIC_EXPORT
int32_t OSAtomicXor32Barrier(uint32_t v, volatile uint32_t *p);
OS_ATOMIC_EXPORT
int32_t OSAtomicXor32OrigBarrier(uint32_t v, volatile uint32_t *p);
OS_ATOMIC_EXPORT
bool OSAtomicCompareAndSwap32Barrier(int32_t o, int32_t n, volatile int32_t *p);
OS_ATOMIC_EXPORT
bool OSAtomicCompareAndSwap64Barrier(int64_t o, int64_t n, volatile int64_t *p);
OS_ATOMIC_EXPORT
bool OSAtomicTestAndSetBarrier(uint32_t n, volatile void * p);
OS_ATOMIC_EXPORT
bool OSAtomicTestAndClearBarrier(uint32_t n, volatile void * p);
OS_ATOMIC_EXPORT
void OSAtomicEnqueue(OSQueueHead *list, void *new, size_t offset);
OS_ATOMIC_EXPORT
void* OSAtomicDequeue(OSQueueHead *list, size_t offset);
OS_ATOMIC_EXPORT
void OSMemoryBarrier(void);

#if OS_ATOMIC_UP
#define OS_ATOMIC_ALIAS_NO_BARRIER(n) OS_ATOMIC_EXPORT_ALIAS(n, n##Barrier)
#else
#define OS_ATOMIC_ALIAS_NO_BARRIER(n)
#endif

int32_t
OSAtomicAdd32Barrier(int32_t v, volatile int32_t *p)
{
	OS_ATOMIC_ALIAS_NO_BARRIER(OSAtomicAdd32);
	int32_t r = os_atomic_add(p, v, acq_rel);
	return r;
}

int32_t
OSAtomicIncrement32Barrier(volatile int32_t *p)
{
	OS_ATOMIC_ALIAS_NO_BARRIER(OSAtomicIncrement32);
	int32_t r = os_atomic_add(p, 1, acq_rel);
	return r;
}

int32_t
OSAtomicDecrement32Barrier(volatile int32_t *p)
{
	OS_ATOMIC_ALIAS_NO_BARRIER(OSAtomicDecrement32);
	int32_t r = os_atomic_add(p, -1, acq_rel);
	return r;
}

int64_t
OSAtomicAdd64Barrier(int64_t v, volatile int64_t *p)
{
	OS_ATOMIC_ALIAS_NO_BARRIER(OSAtomicAdd64);
	int64_t r = os_atomic_add(p, v, acq_rel);
	return r;
}

int64_t
OSAtomicIncrement64Barrier(volatile int64_t *p)
{
	OS_ATOMIC_ALIAS_NO_BARRIER(OSAtomicIncrement64);
	int64_t r = os_atomic_add(p, 1, acq_rel);
	return r;
}

int64_t
OSAtomicDecrement64Barrier(volatile int64_t *p)
{
	OS_ATOMIC_ALIAS_NO_BARRIER(OSAtomicDecrement64);
	int64_t r = os_atomic_add(p, -1, acq_rel);
	return r;
}

int32_t
OSAtomicAnd32Barrier(uint32_t v, volatile uint32_t *p)
{
	OS_ATOMIC_ALIAS_NO_BARRIER(OSAtomicAnd32);
	uint32_t r = os_atomic_and(p, v, acq_rel);
	return (int32_t)r;
}

int32_t
OSAtomicAnd32OrigBarrier(uint32_t v, volatile uint32_t *p)
{
	OS_ATOMIC_ALIAS_NO_BARRIER(OSAtomicAnd32Orig);
	uint32_t r = os_atomic_and_orig(p, v, acq_rel);
	return (int32_t)r;
}

int32_t
OSAtomicOr32Barrier(uint32_t v, volatile uint32_t *p)
{
	OS_ATOMIC_ALIAS_NO_BARRIER(OSAtomicOr32);
	uint32_t r = os_atomic_or(p, v, acq_rel);
	return (int32_t)r;
}

int32_t
OSAtomicOr32OrigBarrier(uint32_t v, volatile uint32_t *p)
{
	OS_ATOMIC_ALIAS_NO_BARRIER(OSAtomicOr32Orig);
	uint32_t r = os_atomic_or_orig(p, v, acq_rel);
	return (int32_t)r;
}

int32_t
OSAtomicXor32Barrier(uint32_t v, volatile uint32_t *p)
{
	OS_ATOMIC_ALIAS_NO_BARRIER(OSAtomicXor32);
	uint32_t r = os_atomic_xor(p, v, acq_rel);
	return (int32_t)r;
}

int32_t
OSAtomicXor32OrigBarrier(uint32_t v, volatile uint32_t *p)
{
	OS_ATOMIC_ALIAS_NO_BARRIER(OSAtomicXor32Orig);
	uint32_t r = os_atomic_xor_orig(p, v, acq_rel);
	return (int32_t)r;
}

bool
OSAtomicCompareAndSwap32Barrier(int32_t o, int32_t n, volatile int32_t *p)
{
	OS_ATOMIC_ALIAS_NO_BARRIER(OSAtomicCompareAndSwap32);
	OS_ATOMIC_ALIAS(OSAtomicCompareAndSwapIntBarrier,
			OSAtomicCompareAndSwap32Barrier);
	OS_ATOMIC_ALIAS_NO_BARRIER(OSAtomicCompareAndSwapInt);
#ifndef __LP64__
	OS_ATOMIC_ALIAS(OSAtomicCompareAndSwapLongBarrier,
			OSAtomicCompareAndSwap32Barrier);
	OS_ATOMIC_ALIAS_NO_BARRIER(OSAtomicCompareAndSwapLong);
	OS_ATOMIC_ALIAS(OSAtomicCompareAndSwapPtrBarrier,
			OSAtomicCompareAndSwap32Barrier);
	OS_ATOMIC_ALIAS_NO_BARRIER(OSAtomicCompareAndSwapPtr);
#endif
	return os_atomic_cmpxchg(p, o, n, acq_rel);
}

bool
OSAtomicCompareAndSwap64Barrier(int64_t o, int64_t n, volatile int64_t *p)
{
	OS_ATOMIC_ALIAS_NO_BARRIER(OSAtomicCompareAndSwap64);
#ifdef __LP64__
	OS_ATOMIC_ALIAS(OSAtomicCompareAndSwapLongBarrier,
			OSAtomicCompareAndSwap64Barrier);
	OS_ATOMIC_ALIAS_NO_BARRIER(OSAtomicCompareAndSwapLong);
	OS_ATOMIC_ALIAS(OSAtomicCompareAndSwapPtrBarrier,
			OSAtomicCompareAndSwap64Barrier);
	OS_ATOMIC_ALIAS_NO_BARRIER(OSAtomicCompareAndSwapPtr);
#endif
	return os_atomic_cmpxchg(p, o, n, acq_rel);
}

static inline uint32_t*
_OSAtomicTestPtrVal(uint32_t bit, volatile void *addr, uint32_t *vp)
{
	uintptr_t a = (uintptr_t)addr;
	if (a & 3) {
		// 32-bit align addr and adjust bit to compensate <rdar://12927920>
		bit += (a & 3) * 8;
		a &= ~3ull;
	}
	*vp = (0x80u >> (bit & 7)) << (bit & ~7u & 31);
	return (uint32_t*)((char*)a + 4 * (bit / 32));
}

bool
OSAtomicTestAndSetBarrier(uint32_t bit, volatile void *addr)
{
	OS_ATOMIC_ALIAS_NO_BARRIER(OSAtomicTestAndSet);
	uint32_t v;
	volatile uint32_t *p = _OSAtomicTestPtrVal(bit, addr, &v);
	uint32_t r = os_atomic_or_orig(p, v, acq_rel);
	return (r & v);
}

bool
OSAtomicTestAndClearBarrier(uint32_t bit, volatile void *addr)
{
	OS_ATOMIC_ALIAS_NO_BARRIER(OSAtomicTestAndClear);
	uint32_t v;
	volatile uint32_t *p = _OSAtomicTestPtrVal(bit, addr, &v);
	uint32_t r = os_atomic_and_orig(p, ~v, acq_rel);
	return (r & v);
}

#if !OS_ATOMIC_NO_BARRIER_ONLY

typedef volatile struct {
	void * volatile item;
	long unused;
} _OSQueueHead;

void
OSAtomicEnqueue(OSQueueHead *list, void *new, size_t offset)
{
	void * volatile *headptr = &(((_OSQueueHead*)list)->item);
	void * volatile *nextptr = (void*)((char*)new + offset);
	void *head = *headptr;
	do {
		*nextptr = head;
	} while (!os_atomic_cmpxchgvw(headptr, head, new, &head, release));
}

void*
OSAtomicDequeue(OSQueueHead *list, size_t offset)
{
	void * volatile *headptr = &(((_OSQueueHead*)list)->item);
	void * volatile *nextptr;
	void *head, *next;
	(void)os_atomic_rmw_loop(headptr, head, next, acquire, {
		if (!head) {
			os_atomic_rmw_loop_give_up(break);
		}
		nextptr = (void*)((char*)head + offset);
		next = *nextptr;
	});
	return head;
}

void
OSMemoryBarrier(void)
{
	os_atomic_thread_fence(seq_cst);
}

#endif // !OS_ATOMIC_NO_BARRIER_ONLY
#endif // TARGET_OS_EMBEDDED

struct _os_empty_files_are_not_c_files;
