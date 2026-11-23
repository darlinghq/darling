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
 * CMSimpleQueue: a ring of capacity + 1 slots between one producer and one consumer. The producer owns the
 * tail index and the consumer the head index; each publishes its index with a release store and reads the
 * other's with an acquire load, so neither needs a lock. The queue is empty when the indices are equal and
 * full when advancing the tail would reach the head.
 */

#include <CoreMedia/CoreMedia.h>
#include "CMClass.h"
#include <limits.h>
#include <stdint.h>

struct opaqueCMSimpleQueue {
	CFRuntimeBase base;
	int32_t capacity;
	uint32_t slotCount;
	uint32_t head;
	uint32_t tail;
	const void **slots;
};

static void simpleQueueFinalize(CFTypeRef cf)
{
	struct opaqueCMSimpleQueue *queue = (struct opaqueCMSimpleQueue *)cf;
	if (queue->slots != NULL)
		CFAllocatorDeallocate(CFGetAllocator(cf), queue->slots);
}

CM_CLASS(CMSimpleQueue_, "CMSimpleQueue", .finalize = simpleQueueFinalize)

CFTypeID CMSimpleQueueGetTypeID(void) { return CMSimpleQueue_GetTypeID(); }

static struct opaqueCMSimpleQueue *asSimpleQueue(CMSimpleQueueRef queue)
{
	return queue != NULL && CFGetTypeID(queue) == CMSimpleQueue_GetTypeID() ? (struct opaqueCMSimpleQueue *)queue : NULL;
}

OSStatus CMSimpleQueueCreate(CFAllocatorRef allocator, int32_t capacity, CMSimpleQueueRef *queueOut)
{
	if (queueOut == NULL)
		return kCMSimpleQueueError_RequiredParameterMissing;
	/* The capacity is a required parameter, so 0 counts as missing; a negative one is documented as out of range. */
	if (capacity == 0)
		return kCMSimpleQueueError_RequiredParameterMissing;
	if (capacity < 0)
		return kCMSimpleQueueError_ParameterOutOfRange;
	uint32_t slotCount = (uint32_t)capacity + 1;
	size_t bytes;
	if (__builtin_mul_overflow((size_t)slotCount, sizeof(const void *), &bytes) || bytes > (size_t)LONG_MAX)
		return kCMSimpleQueueError_AllocationFailed;
	struct opaqueCMSimpleQueue *queue = CM_NEW(struct opaqueCMSimpleQueue *, CMSimpleQueue_, allocator);
	if (queue == NULL)
		return kCMSimpleQueueError_AllocationFailed;
	queue->slots = CFAllocatorAllocate(allocator, (CFIndex)bytes, 0);
	if (queue->slots == NULL) {
		CFRelease(queue);
		return kCMSimpleQueueError_AllocationFailed;
	}
	queue->capacity = capacity;
	queue->slotCount = slotCount;
	*queueOut = queue;
	return noErr;
}

OSStatus CMSimpleQueueEnqueue(CMSimpleQueueRef queueRef, const void *element)
{
	/* The documentation lists no code for an object that is not a queue (macOS crashes on it); the argument is
	   then out of range here. */
	if (queueRef == NULL || element == NULL)
		return kCMSimpleQueueError_RequiredParameterMissing;
	struct opaqueCMSimpleQueue *queue = asSimpleQueue(queueRef);
	if (queue == NULL)
		return kCMSimpleQueueError_ParameterOutOfRange;
	uint32_t tail = __atomic_load_n(&queue->tail, __ATOMIC_RELAXED);
	uint32_t next = tail + 1 == queue->slotCount ? 0 : tail + 1;
	if (next == __atomic_load_n(&queue->head, __ATOMIC_ACQUIRE))
		return kCMSimpleQueueError_QueueIsFull;
	queue->slots[tail] = element;
	__atomic_store_n(&queue->tail, next, __ATOMIC_RELEASE);
	return noErr;
}

const void *CMSimpleQueueGetHead(CMSimpleQueueRef queueRef)
{
	struct opaqueCMSimpleQueue *queue = asSimpleQueue(queueRef);
	if (queue == NULL)
		return NULL;
	uint32_t head = __atomic_load_n(&queue->head, __ATOMIC_RELAXED);
	if (head == __atomic_load_n(&queue->tail, __ATOMIC_ACQUIRE))
		return NULL;
	return queue->slots[head];
}

const void *CMSimpleQueueDequeue(CMSimpleQueueRef queueRef)
{
	struct opaqueCMSimpleQueue *queue = asSimpleQueue(queueRef);
	if (queue == NULL)
		return NULL;
	uint32_t head = __atomic_load_n(&queue->head, __ATOMIC_RELAXED);
	if (head == __atomic_load_n(&queue->tail, __ATOMIC_ACQUIRE))
		return NULL;
	const void *element = queue->slots[head];
	__atomic_store_n(&queue->head, head + 1 == queue->slotCount ? 0 : head + 1, __ATOMIC_RELEASE);
	return element;
}

OSStatus CMSimpleQueueReset(CMSimpleQueueRef queueRef)
{
	if (queueRef == NULL)
		return kCMSimpleQueueError_RequiredParameterMissing;
	struct opaqueCMSimpleQueue *queue = asSimpleQueue(queueRef);
	if (queue == NULL)
		return kCMSimpleQueueError_ParameterOutOfRange;
	/* Not synchronised, as documented: the reader and the writer are held off by the caller. */
	__atomic_store_n(&queue->head, 0, __ATOMIC_SEQ_CST);
	__atomic_store_n(&queue->tail, 0, __ATOMIC_SEQ_CST);
	return noErr;
}

int32_t CMSimpleQueueGetCapacity(CMSimpleQueueRef queueRef)
{
	struct opaqueCMSimpleQueue *queue = asSimpleQueue(queueRef);
	return queue != NULL ? queue->capacity : 0;
}

int32_t CMSimpleQueueGetCount(CMSimpleQueueRef queueRef)
{
	struct opaqueCMSimpleQueue *queue = asSimpleQueue(queueRef);
	if (queue == NULL)
		return 0;
	uint32_t head = __atomic_load_n(&queue->head, __ATOMIC_ACQUIRE);
	uint32_t tail = __atomic_load_n(&queue->tail, __ATOMIC_ACQUIRE);
	return (int32_t)(tail >= head ? tail - head : tail + queue->slotCount - head);
}
