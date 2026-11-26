/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * CMMemoryPool: a CFAllocator that keeps recently deallocated blocks, in whole pages, for the next allocation
 * of the same size. Every block carries a header with its size, because CFAllocator deallocation is given only
 * the pointer. A block that sits unused for longer than the age-out period is freed whenever the allocator is
 * used. The state of the pool is shared by the pool and by its allocator, so the allocator can outlive the pool.
 */

#include <CoreMedia/CoreMedia.h>
#include "CMClass.h"
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* The default period after which an unused block is freed, in seconds, as documented. */
#define kDefaultAgeOutPeriod 0.5

/* The room in front of the memory handed out for the size of the block; a multiple of the strictest alignment. */
#define kHeaderSize 16

/* A block held for reuse; it lives in the block itself, after the header, because the memory is dead. */
struct heldBlock {
	struct heldBlock *next;
	uint64_t heldSince;
};

struct poolState {
	pthread_mutex_t lock;
	int32_t references;
	Boolean invalidated;
	uint64_t agePeriodNanoseconds;
	size_t pageSize;
	struct heldBlock *held;
};

struct OpaqueCMMemoryPool {
	CFRuntimeBase base;
	struct poolState *state;
	CFAllocatorRef allocator;
};

static uint64_t nowNanoseconds(void)
{
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	return (uint64_t)now.tv_sec * 1000000000u + (uint64_t)now.tv_nsec;
}

static size_t sizeOfHeldBlock(const struct heldBlock *block)
{
	return *(const size_t *)((const char *)block - kHeaderSize);
}

static void freeHeldBlock(struct heldBlock *block)
{
	free((char *)block - kHeaderSize);
}

/* Frees the blocks held since before olderThan. The caller holds the lock. */
static void freeHeldBlocks(struct poolState *state, uint64_t olderThan)
{
	struct heldBlock **link = &state->held;
	while (*link != NULL) {
		struct heldBlock *block = *link;
		if (block->heldSince < olderThan) {
			*link = block->next;
			freeHeldBlock(block);
		} else {
			link = &block->next;
		}
	}
}

/* Frees the held blocks that have outlived the age-out period. The caller holds the lock. */
static void ageOut(struct poolState *state)
{
	uint64_t now = nowNanoseconds();
	if (now > state->agePeriodNanoseconds)
		freeHeldBlocks(state, now - state->agePeriodNanoseconds);
}

static const void *stateRetain(const void *info)
{
	struct poolState *state = (struct poolState *)info;
	__atomic_add_fetch(&state->references, 1, __ATOMIC_RELAXED);
	return info;
}

static void stateRelease(const void *info)
{
	struct poolState *state = (struct poolState *)info;
	if (__atomic_sub_fetch(&state->references, 1, __ATOMIC_ACQ_REL) == 0) {
		freeHeldBlocks(state, UINT64_MAX);
		pthread_mutex_destroy(&state->lock);
		free(state);
	}
}

static void *poolAllocate(CFIndex size, CFOptionFlags hint, void *info)
{
	struct poolState *state = (struct poolState *)info;
	if (size <= 0 || (uint64_t)size > SIZE_MAX - kHeaderSize - state->pageSize)
		return NULL;
	size_t needed = (size_t)size + kHeaderSize;
	pthread_mutex_lock(&state->lock);
	/* The block is a whole number of pages, as documented for the pool, and stays so after the pool is
	   invalidated. */
	size_t blockSize = (needed + state->pageSize - 1) / state->pageSize * state->pageSize;
	char *block = NULL;
	if (!state->invalidated) {
		ageOut(state);
		for (struct heldBlock **link = &state->held; *link != NULL; link = &(*link)->next) {
			if (sizeOfHeldBlock(*link) == blockSize) {
				block = (char *)*link - kHeaderSize;
				*link = (*link)->next;
				break;
			}
		}
	}
	pthread_mutex_unlock(&state->lock);
	if (block == NULL) {
		block = malloc(blockSize);
		if (block == NULL)
			return NULL;
		*(size_t *)block = blockSize;
	}
	return block + kHeaderSize;
}

static void poolDeallocate(void *pointer, void *info)
{
	struct poolState *state = (struct poolState *)info;
	char *block = (char *)pointer - kHeaderSize;
	pthread_mutex_lock(&state->lock);
	if (state->invalidated) {
		pthread_mutex_unlock(&state->lock);
		free(block);
		return;
	}
	struct heldBlock *held = (struct heldBlock *)pointer;
	held->heldSince = nowNanoseconds();
	held->next = state->held;
	state->held = held;
	ageOut(state);
	pthread_mutex_unlock(&state->lock);
}

static void *poolReallocate(void *pointer, CFIndex newSize, CFOptionFlags hint, void *info)
{
	if (pointer == NULL)
		return poolAllocate(newSize, hint, info);
	if (newSize <= 0) {
		poolDeallocate(pointer, info);
		return NULL;
	}
	size_t oldCapacity = *(size_t *)((char *)pointer - kHeaderSize) - kHeaderSize;
	void *grown = poolAllocate(newSize, hint, info);
	if (grown == NULL)
		return NULL;
	memcpy(grown, pointer, oldCapacity < (size_t)newSize ? oldCapacity : (size_t)newSize);
	poolDeallocate(pointer, info);
	return grown;
}

static void poolFlush(struct poolState *state, Boolean invalidate)
{
	pthread_mutex_lock(&state->lock);
	if (invalidate)
		state->invalidated = true;
	freeHeldBlocks(state, UINT64_MAX);
	pthread_mutex_unlock(&state->lock);
}

static void memoryPoolFinalize(CFTypeRef cf)
{
	struct OpaqueCMMemoryPool *pool = (struct OpaqueCMMemoryPool *)cf;
	if (pool->state != NULL)
		poolFlush(pool->state, true);
	if (pool->allocator != NULL)
		CFRelease(pool->allocator);
}

CM_CLASS(CMMemoryPool_, "CMMemoryPool", .finalize = memoryPoolFinalize)

CFTypeID CMMemoryPoolGetTypeID(void) { return CMMemoryPool_GetTypeID(); }

static struct OpaqueCMMemoryPool *asMemoryPool(CMMemoryPoolRef pool)
{
	return pool != NULL && CFGetTypeID(pool) == CMMemoryPool_GetTypeID() ? (struct OpaqueCMMemoryPool *)pool : NULL;
}

/* The string value of the option key is the one macOS 26.7.3 and 10.14.4 give the public symbol (read from the
   symbol in a probe program; tests/src/coremedia_memorypool.c checks it). */
const CFStringRef kCMMemoryPoolOption_AgeOutPeriod = CFSTR("FigMemoryPool_AgeOutPeriod");

/* The age-out period in nanoseconds from the options, or the documented default. GUESS: an option that is not
   a number, is negative or is not finite is ignored, and a very long period is capped; the documentation says
   nothing about these. */
static uint64_t agePeriodFromOptions(CFDictionaryRef options)
{
	double seconds = kDefaultAgeOutPeriod;
	CFTypeRef value = options != NULL ? CFDictionaryGetValue(options, kCMMemoryPoolOption_AgeOutPeriod) : NULL;
	double requested;
	if (value != NULL && CFGetTypeID(value) == CFNumberGetTypeID()
			&& CFNumberGetValue(value, kCFNumberFloat64Type, &requested) && requested >= 0.0 && requested < 1e18)
		seconds = requested;
	return seconds >= 1e9 ? (uint64_t)1e18 : (uint64_t)(seconds * 1e9);
}

CMMemoryPoolRef CMMemoryPoolCreate(CFDictionaryRef options)
{
	/* A failed allocation gives NULL, although the declaration says non-null. */
	struct poolState *state = calloc(1, sizeof(*state));
	if (state == NULL)
		return NULL;
	pthread_mutex_init(&state->lock, NULL);
	state->agePeriodNanoseconds = agePeriodFromOptions(options);
	state->pageSize = (size_t)sysconf(_SC_PAGESIZE);
	CFAllocatorContext context = { 0, state, stateRetain, stateRelease, NULL, poolAllocate, poolReallocate, poolDeallocate, NULL };
	/* The allocator takes the first reference to the state, and every reference to the allocator keeps it. */
	CFAllocatorRef allocator = CFAllocatorCreate(kCFAllocatorSystemDefault, &context);
	if (allocator == NULL) {
		pthread_mutex_destroy(&state->lock);
		free(state);
		return NULL;
	}
	struct OpaqueCMMemoryPool *pool = CM_NEW(struct OpaqueCMMemoryPool *, CMMemoryPool_, kCFAllocatorSystemDefault);
	if (pool == NULL) {
		CFRelease(allocator);
		return NULL;
	}
	pool->state = state;
	pool->allocator = allocator;
	return pool;
}

CFAllocatorRef CMMemoryPoolGetAllocator(CMMemoryPoolRef poolRef)
{
	struct OpaqueCMMemoryPool *pool = asMemoryPool(poolRef);
	return pool != NULL ? pool->allocator : NULL;
}

void CMMemoryPoolFlush(CMMemoryPoolRef poolRef)
{
	struct OpaqueCMMemoryPool *pool = asMemoryPool(poolRef);
	if (pool != NULL)
		poolFlush(pool->state, false);
}

void CMMemoryPoolInvalidate(CMMemoryPoolRef poolRef)
{
	struct OpaqueCMMemoryPool *pool = asMemoryPool(poolRef);
	if (pool != NULL)
		poolFlush(pool->state, true);
}
