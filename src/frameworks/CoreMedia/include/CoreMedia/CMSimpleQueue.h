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

#ifndef _CMSIMPLEQUEUE_H_
#define _CMSIMPLEQUEUE_H_

#include <CoreMedia/CMBase.h>

// A lockless FIFO queue of pointer-sized elements, safe for one enqueueing and one dequeueing thread. NULL is
// not a valid element: it is what Dequeue returns for an empty queue.

CF_IMPLICIT_BRIDGING_ENABLED

// CMSimpleQueue error codes
enum {
	kCMSimpleQueueError_AllocationFailed = -12770,
	kCMSimpleQueueError_RequiredParameterMissing = -12771,
	kCMSimpleQueueError_ParameterOutOfRange = -12772,
	kCMSimpleQueueError_QueueIsFull = -12773,
};

typedef struct CF_BRIDGED_TYPE(id) opaqueCMSimpleQueue *CMSimpleQueueRef;

CM_EXPORT CFTypeID CMSimpleQueueGetTypeID(void);

// The capacity is the number of elements the queue holds at most; it must be positive.
CM_EXPORT OSStatus CMSimpleQueueCreate(CFAllocatorRef allocator, int32_t capacity, CMSimpleQueueRef *queueOut);

// Fails with kCMSimpleQueueError_QueueIsFull when the queue is full.
CM_EXPORT OSStatus CMSimpleQueueEnqueue(CMSimpleQueueRef queue, const void *element);

// NULL when the queue is empty, or on an error.
CM_EXPORT const void *CMSimpleQueueDequeue(CMSimpleQueueRef queue);
CM_EXPORT const void *CMSimpleQueueGetHead(CMSimpleQueueRef queue);

// Empties the queue. Not synchronised: the reader and the writer must be held off meanwhile.
CM_EXPORT OSStatus CMSimpleQueueReset(CMSimpleQueueRef queue);

// 0 on an error.
CM_EXPORT int32_t CMSimpleQueueGetCapacity(CMSimpleQueueRef queue);
CM_EXPORT int32_t CMSimpleQueueGetCount(CMSimpleQueueRef queue);

// How full the queue is, from 0.0 (empty) to 1.0 (full); 0.0 also stands for an error.
static inline Float32 CMSimpleQueueGetFullness(CMSimpleQueueRef queue)
{
	int32_t capacity = CMSimpleQueueGetCapacity(queue);
	return capacity == 0 ? 0.0f : (Float32)CMSimpleQueueGetCount(queue) / (Float32)capacity;
}

CF_IMPLICIT_BRIDGING_DISABLED

#endif
