// CFLAGS: -framework CoreMedia -framework CoreFoundation
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

// CMSimpleQueue: a lockless FIFO queue of pointer-sized elements.
// The documented contract (public CMSimpleQueue.h and the pages below): Create takes a positive capacity
// and fails with ParameterOutOfRange for a negative one; Enqueue fails with QueueIsFull on a full queue and
// takes no NULL element; Dequeue returns the elements in order and NULL when empty; GetHead returns the
// next element without removing it; Reset empties the queue and keeps the capacity; GetCapacity and
// GetCount return 0 on an error; one enqueueing and one dequeueing thread need no lock.
// https://developer.apple.com/documentation/coremedia/cmsimplequeuecreate(allocator:capacity:queueout:)
// https://developer.apple.com/documentation/coremedia/cmsimplequeueenqueue(_:element:)
// https://developer.apple.com/documentation/coremedia/cmsimplequeuedequeue(_:)
// https://developer.apple.com/documentation/coremedia/cmsimplequeuereset(_:)
// The error codes are those of the public header; a capacity of 0 giving RequiredParameterMissing is what macOS
// 26.7.3 and 10.14.4 do. Prints PASS when everything holds; the exit code is the number of failed checks.

#include <CoreMedia/CoreMedia.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { failures++; printf("FAIL line %d: %s\n", __LINE__, #cond); } } while (0)

#define kElements 200000

static CMSimpleQueueRef sharedQueue;
static int consumerFailed;

static void *producer(void *unused)
{
	for (uintptr_t next = 1; next <= kElements; next++)
		while (CMSimpleQueueEnqueue(sharedQueue, (const void *)next) != noErr)
			;
	return NULL;
}

static void *consumer(void *unused)
{
	for (uintptr_t expected = 1; expected <= kElements; expected++) {
		const void *element;
		while ((element = CMSimpleQueueDequeue(sharedQueue)) == NULL)
			;
		if ((uintptr_t)element != expected) {
			consumerFailed = 1;
			return NULL;
		}
	}
	return NULL;
}

int main(void)
{
	CMSimpleQueueRef queue = NULL;

	CHECK(kCMSimpleQueueError_AllocationFailed == -12770);
	CHECK(kCMSimpleQueueError_RequiredParameterMissing == -12771);
	CHECK(kCMSimpleQueueError_ParameterOutOfRange == -12772);
	CHECK(kCMSimpleQueueError_QueueIsFull == -12773);

	// creation: a CF object of its own type, empty, with the requested capacity
	CHECK(CMSimpleQueueCreate(kCFAllocatorDefault, 3, &queue) == noErr && queue != NULL);
	CHECK(CFGetTypeID(queue) == CMSimpleQueueGetTypeID());
	CHECK(CMSimpleQueueGetCapacity(queue) == 3 && CMSimpleQueueGetCount(queue) == 0);
	CHECK(CMSimpleQueueGetFullness(queue) == 0.0f);
	CHECK(CMSimpleQueueDequeue(queue) == NULL && CMSimpleQueueGetHead(queue) == NULL);

	// arguments of Create
	CMSimpleQueueRef rejected = NULL;
	CHECK(CMSimpleQueueCreate(kCFAllocatorDefault, -1, &rejected) == kCMSimpleQueueError_ParameterOutOfRange);
	CHECK(CMSimpleQueueCreate(kCFAllocatorDefault, INT32_MIN, &rejected) == kCMSimpleQueueError_ParameterOutOfRange);
	CHECK(CMSimpleQueueCreate(kCFAllocatorDefault, 0, &rejected) == kCMSimpleQueueError_RequiredParameterMissing);
	CHECK(CMSimpleQueueCreate(kCFAllocatorDefault, 3, NULL) == kCMSimpleQueueError_RequiredParameterMissing);
	CHECK(rejected == NULL);

	// first in, first out; the head is not removed
	CHECK(CMSimpleQueueEnqueue(queue, (const void *)10) == noErr);
	CHECK(CMSimpleQueueEnqueue(queue, (const void *)20) == noErr);
	CHECK(CMSimpleQueueGetCount(queue) == 2 && CMSimpleQueueGetFullness(queue) > 0.66f && CMSimpleQueueGetFullness(queue) < 0.67f);
	CHECK(CMSimpleQueueGetHead(queue) == (const void *)10 && CMSimpleQueueGetCount(queue) == 2);
	CHECK(CMSimpleQueueEnqueue(queue, (const void *)30) == noErr);
	// full: Enqueue fails and changes nothing
	CHECK(CMSimpleQueueGetCount(queue) == 3 && CMSimpleQueueGetFullness(queue) == 1.0f);
	CHECK(CMSimpleQueueEnqueue(queue, (const void *)40) == kCMSimpleQueueError_QueueIsFull);
	CHECK(CMSimpleQueueGetCount(queue) == 3 && CMSimpleQueueGetHead(queue) == (const void *)10);
	CHECK(CMSimpleQueueDequeue(queue) == (const void *)10);
	CHECK(CMSimpleQueueDequeue(queue) == (const void *)20);
	CHECK(CMSimpleQueueGetCount(queue) == 1);
	// the ring wraps: a slot freed by Dequeue is used again, and the order is kept
	CHECK(CMSimpleQueueEnqueue(queue, (const void *)40) == noErr);
	CHECK(CMSimpleQueueEnqueue(queue, (const void *)50) == noErr);
	CHECK(CMSimpleQueueEnqueue(queue, (const void *)60) == kCMSimpleQueueError_QueueIsFull);
	CHECK(CMSimpleQueueGetCount(queue) == 3);
	CHECK(CMSimpleQueueDequeue(queue) == (const void *)30);
	CHECK(CMSimpleQueueDequeue(queue) == (const void *)40);
	CHECK(CMSimpleQueueDequeue(queue) == (const void *)50);
	CHECK(CMSimpleQueueDequeue(queue) == NULL && CMSimpleQueueGetCount(queue) == 0);
	// elements are not dereferenced: any non-NULL value goes through, the largest included
	CHECK(CMSimpleQueueEnqueue(queue, (const void *)UINTPTR_MAX) == noErr);
	CHECK(CMSimpleQueueDequeue(queue) == (const void *)UINTPTR_MAX);

	// NULL is not an element
	CHECK(CMSimpleQueueEnqueue(queue, NULL) == kCMSimpleQueueError_RequiredParameterMissing);
	CHECK(CMSimpleQueueGetCount(queue) == 0);

	// Reset empties the queue and keeps the capacity, also from a wrapped state
	CHECK(CMSimpleQueueEnqueue(queue, (const void *)1) == noErr && CMSimpleQueueEnqueue(queue, (const void *)2) == noErr);
	CHECK(CMSimpleQueueDequeue(queue) == (const void *)1);
	CHECK(CMSimpleQueueEnqueue(queue, (const void *)3) == noErr && CMSimpleQueueEnqueue(queue, (const void *)4) == noErr);
	CHECK(CMSimpleQueueReset(queue) == noErr);
	CHECK(CMSimpleQueueGetCount(queue) == 0 && CMSimpleQueueGetCapacity(queue) == 3);
	CHECK(CMSimpleQueueDequeue(queue) == NULL && CMSimpleQueueGetHead(queue) == NULL);
	for (uintptr_t i = 1; i <= 3; i++)
		CHECK(CMSimpleQueueEnqueue(queue, (const void *)i) == noErr);
	CHECK(CMSimpleQueueEnqueue(queue, (const void *)4) == kCMSimpleQueueError_QueueIsFull);

	// a queue of capacity 1
	CMSimpleQueueRef one = NULL;
	CHECK(CMSimpleQueueCreate(NULL, 1, &one) == noErr);
	CHECK(CMSimpleQueueEnqueue(one, (const void *)7) == noErr);
	CHECK(CMSimpleQueueEnqueue(one, (const void *)8) == kCMSimpleQueueError_QueueIsFull);
	CHECK(CMSimpleQueueDequeue(one) == (const void *)7 && CMSimpleQueueDequeue(one) == NULL);
	CHECK(CMSimpleQueueEnqueue(one, (const void *)8) == noErr && CMSimpleQueueGetHead(one) == (const void *)8);
	CFRelease(one);

	// NULL: the documented results are 0, NULL and the status codes of the header (as observed on macOS 26.7.3 and
	// 10.14.4; an object that is not a queue crashes there and is not tested)
	CHECK(CMSimpleQueueEnqueue(NULL, (const void *)1) == kCMSimpleQueueError_RequiredParameterMissing);
	CHECK(CMSimpleQueueDequeue(NULL) == NULL && CMSimpleQueueGetHead(NULL) == NULL);
	CHECK(CMSimpleQueueReset(NULL) == kCMSimpleQueueError_RequiredParameterMissing);
	CHECK(CMSimpleQueueGetCapacity(NULL) == 0 && CMSimpleQueueGetCount(NULL) == 0 && CMSimpleQueueGetFullness(NULL) == 0.0f);
	CFRelease(queue);

	// one thread enqueues and one dequeues, on a queue much smaller than the number of elements: every element
	// arrives, in order
	CHECK(CMSimpleQueueCreate(kCFAllocatorDefault, 16, &sharedQueue) == noErr);
	pthread_t writer, reader;
	CHECK(pthread_create(&reader, NULL, consumer, NULL) == 0);
	CHECK(pthread_create(&writer, NULL, producer, NULL) == 0);
	pthread_join(writer, NULL);
	pthread_join(reader, NULL);
	CHECK(!consumerFailed);
	CHECK(CMSimpleQueueGetCount(sharedQueue) == 0);
	CFRelease(sharedQueue);

	if (failures == 0)
		printf("PASS\n");
	return failures;
}
