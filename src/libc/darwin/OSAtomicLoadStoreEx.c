/*
 * Copyright (c) 2011 Apple Inc. All rights reserved.
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

// OSAtomic.h is included by files that include this C file.
/* #include "OSAtomic.h" */

int32_t OSAtomicAdd32(int32_t v, volatile int32_t *p) _OSATOMIC_VARIANT_B(OSAtomicAdd32);
int32_t OSAtomicAdd32(int32_t v, volatile int32_t *p)
{
	_OSATOMIC_ALIAS_NB(OSAtomicAdd32);
	int32_t r;
	uint32_t t;

	_osatomic_store_barrier();
	do {
		_osatomic_load_exclusive(p, r);
		r += v;
		_osatomic_store_exclusive(p, r, t);
	} while (slowpath(t));

	_osatomic_barrier();
	return r;
}

int64_t OSAtomicAdd64(int64_t v, volatile int64_t *p) _OSATOMIC_VARIANT_B(OSAtomicAdd64);
int64_t OSAtomicAdd64(int64_t v, volatile int64_t *p)
{
	_OSATOMIC_ALIAS_NB(OSAtomicAdd64);

	int64_t r;
	uint32_t t;

	_osatomic_store_barrier();
	do {
		_osatomic_load_exclusive64(p, r);
		r += v;
		_osatomic_store_exclusive64(p, r, t);
	} while (slowpath(t));

	_osatomic_barrier();
	return r;
}

int32_t OSAtomicOr32(int32_t v, volatile int32_t *p) _OSATOMIC_VARIANT_B(OSAtomicOr32);
int32_t OSAtomicOr32(int32_t v, volatile int32_t *p)
{
	_OSATOMIC_ALIAS_NB(OSAtomicOr32);
	int32_t r;
	uint32_t t;

	_osatomic_store_barrier();
	do {
		_osatomic_load_exclusive(p, r);
		r |= v;
		_osatomic_store_exclusive(p, r, t);
	} while (slowpath(t));

	_osatomic_barrier();
	return r;
}

int32_t OSAtomicOr32Orig(int32_t v, volatile int32_t *p) _OSATOMIC_VARIANT_B(OSAtomicOr32Orig);
int32_t OSAtomicOr32Orig(int32_t v, volatile int32_t *p)
{
	_OSATOMIC_ALIAS_NB(OSAtomicOr32Orig);
	int32_t r, n;
	uint32_t t;

	_osatomic_store_barrier();
	do {
		_osatomic_load_exclusive(p, r);
		n = r | v;
		_osatomic_store_exclusive(p, n, t);
	} while (slowpath(t));

	_osatomic_barrier();
	return r;
}

int32_t OSAtomicAnd32(int32_t v, volatile int32_t *p) _OSATOMIC_VARIANT_B(OSAtomicAnd32);
int32_t OSAtomicAnd32(int32_t v, volatile int32_t *p)
{
	_OSATOMIC_ALIAS_NB(OSAtomicAnd32);
	int32_t r;
	uint32_t t;

	_osatomic_store_barrier();
	do {
		_osatomic_load_exclusive(p, r);
		r &= v;
		_osatomic_store_exclusive(p, r, t);
	} while (slowpath(t));

	_osatomic_barrier();
	return r;
}

int32_t OSAtomicAnd32Orig(int32_t v, volatile int32_t *p) _OSATOMIC_VARIANT_B(OSAtomicAnd32Orig);
int32_t OSAtomicAnd32Orig(int32_t v, volatile int32_t *p)
{
	_OSATOMIC_ALIAS_NB(OSAtomicAnd32Orig);
	int32_t r, n;
	uint32_t t;

	_osatomic_store_barrier();
	do {
		_osatomic_load_exclusive(p, r);
		n = r & v;
		_osatomic_store_exclusive(p, n, t);
	} while (slowpath(t));

	_osatomic_barrier();
	return r;
}

int32_t OSAtomicXor32(int32_t v, volatile int32_t *p) _OSATOMIC_VARIANT_B(OSAtomicXor32);
int32_t OSAtomicXor32(int32_t v, volatile int32_t *p)
{
	_OSATOMIC_ALIAS_NB(OSAtomicXor32);
	int32_t r;
	uint32_t t;

	_osatomic_store_barrier();
	do {
		_osatomic_load_exclusive(p, r);
		r ^= v;
		_osatomic_store_exclusive(p, r, t);
	} while (slowpath(t));

	_osatomic_barrier();
	return r;
}

int32_t OSAtomicXor32Orig(int32_t v, volatile int32_t *p) _OSATOMIC_VARIANT_B(OSAtomicXor32Orig);
int32_t OSAtomicXor32Orig(int32_t v, volatile int32_t *p)
{
	_OSATOMIC_ALIAS_NB(OSAtomicXor32Orig);
	int32_t r, n;
	uint32_t t;

	_osatomic_store_barrier();
	do {
		_osatomic_load_exclusive(p, r);
		n = r ^ v;
		_osatomic_store_exclusive(p, n, t);
	} while (slowpath(t));

	_osatomic_barrier();
	return r;
}

bool OSAtomicCompareAndSwap32(int32_t o, int32_t n, volatile int32_t *p) _OSATOMIC_VARIANT_B(OSAtomicCompareAndSwap32);
bool OSAtomicCompareAndSwap32(int32_t o, int32_t n, volatile int32_t *p)
{
	_OSATOMIC_ALIAS_B(OSAtomicCompareAndSwapInt, OSAtomicCompareAndSwap32);

	_OSATOMIC_ALIAS_NB(OSAtomicCompareAndSwap32);
	_OSATOMIC_ALIAS_NB(OSAtomicCompareAndSwapInt);
#ifndef __LP64__
	_OSATOMIC_ALIAS_B(OSAtomicCompareAndSwapLong, OSAtomicCompareAndSwap32);
	_OSATOMIC_ALIAS_B(OSAtomicCompareAndSwapPtr, OSAtomicCompareAndSwap32);
	_OSATOMIC_ALIAS_NB(OSAtomicCompareAndSwapLong);
	_OSATOMIC_ALIAS_NB(OSAtomicCompareAndSwapPtr);
#endif

	int32_t r;
	uint32_t t;

	do {
		_osatomic_load_exclusive(p, r);
		if (r != o) return false;
		_osatomic_store_barrier();
		_osatomic_store_exclusive(p, n, t);
	} while (slowpath(t));

	_osatomic_barrier();
	return true;
}

bool OSAtomicCompareAndSwap64(int64_t o, int64_t n, volatile int64_t *p) _OSATOMIC_VARIANT_B(OSAtomicCompareAndSwap64);
bool OSAtomicCompareAndSwap64(int64_t o, int64_t n, volatile int64_t *p)
{
	_OSATOMIC_ALIAS_NB(OSAtomicCompareAndSwap64);
#ifdef __LP64__
	_OSATOMIC_ALIAS_B(OSAtomicCompareAndSwapLong, OSAtomicCompareAndSwap64);
	_OSATOMIC_ALIAS_B(OSAtomicCompareAndSwapPtr, OSAtomicCompareAndSwap64);
	_OSATOMIC_ALIAS_NB(OSAtomicCompareAndSwapLong);
	_OSATOMIC_ALIAS_NB(OSAtomicCompareAndSwapPtr);
#endif

	int64_t r;
	uint32_t t;

	do {
		_osatomic_load_exclusive64(p, r);
		if (r != o) return false;
		_osatomic_store_barrier();
		_osatomic_store_exclusive64(p, n, t);
	} while (slowpath(t));

	_osatomic_barrier();
	return true;
}

#if defined(_OSATOMIC_EXTRAS)

void OSMemoryBarrier(void) _OSATOMIC_VARIANT(OSMemoryBarrier);
void OSMemoryBarrier(void)
{
	_osatomic_barrier();
}

typedef volatile struct {
	void	*item;
	long	 unused;
} OSQueueHead;

void OSAtomicEnqueue(OSQueueHead *l, void *n, size_t o) _OSATOMIC_VARIANT(OSAtomicEnqueue);
void OSAtomicEnqueue(OSQueueHead *l, void *n, size_t o)
{
	void ** r = (void **)((char *)n + o);
	void * volatile * i = (void **)&(l->item);
	void * q;
	uint32_t t;

	_osatomic_store_barrier();
	do {
		_osatomic_load_exclusive(i, q);
		*r = q;
		_osatomic_store_exclusive(i, n, t);
	} while (slowpath(t));
	_osatomic_barrier();
}

void* OSAtomicDequeue(OSQueueHead *l, size_t o) _OSATOMIC_VARIANT(OSAtomicDequeue);
void* OSAtomicDequeue(OSQueueHead *l, size_t o)
{
	void * h;
	void ** r;
	void * volatile * i = (void **)&(l->item);
	uint32_t t;

	_osatomic_store_barrier();
	do {
		_osatomic_load_exclusive(i, r);
		if (!r) return NULL;
		h = *(void **)((char *)r + o);
		_osatomic_store_exclusive(i, h, t);
	} while (slowpath(t));

	_osatomic_barrier();
	return r;
}

#endif

bool OSAtomicTestAndSet(uint32_t n, volatile void * p) _OSATOMIC_VARIANT_B(OSAtomicTestAndSet);
bool OSAtomicTestAndSet(uint32_t n, volatile void * p)
{
	_OSATOMIC_ALIAS_NB(OSAtomicTestAndSet);
	uint32_t * ptr = (uint32_t *)((char *)p + (4 * (n / 32)));
	uint32_t i, r, t;

	n = (0x80 >> (n & 7)) << (n & ~7 & 31);

	_osatomic_store_barrier();
	do {
		_osatomic_load_exclusive(ptr, r);
		i = r | n;
		if (i == r) break;
		_osatomic_store_exclusive(ptr, i, t);
	} while (slowpath(t));

	_osatomic_barrier();
	return ((r & n) != 0);
}

bool OSAtomicTestAndClear(uint32_t n, volatile void * p) _OSATOMIC_VARIANT_B(OSAtomicTestAndClear);
bool OSAtomicTestAndClear(uint32_t n, volatile void * p)
{
	_OSATOMIC_ALIAS_NB(OSAtomicTestAndClear);
	uint32_t * ptr = (uint32_t *)((char *)p + (4 * (n / 32)));
	uint32_t i, r, t;

	n = (0x80 >> (n & 7)) << (n & ~7 & 31);

	_osatomic_store_barrier();
	do {
		_osatomic_load_exclusive(ptr, r);
		i = r & ~n;
		if (i == r) break;
		_osatomic_store_exclusive(ptr, i, t);
	} while (slowpath(t));

	_osatomic_barrier();
	return ((r & n) != 0);
}

