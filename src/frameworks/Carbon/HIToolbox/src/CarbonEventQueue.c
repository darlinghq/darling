/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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

// Event queues. A queue holds a reference to each event posted to it. Events leave it in
// the order of their priority (high, standard, low), in posting order within a priority; an
// event posted at the head of the queue (a private function of the framework) comes before
// all of them. Any thread can post to, and look into, any queue.

#include <HIToolbox/CarbonEventsCore.h>
#include "HIToolboxInternal.h"
#include "HIToolboxPrivate.h"
#include <CarbonCore/MacErrors.h>

#include <stdlib.h>
#include <string.h>

// The index of the list for events posted at the head of the queue; the others are the
// priorities
enum { kListHead = 3, kListCount = 4 };

static atomic_bool g_mouseCoalescing = true;

Boolean _HIMatchesEventTypes(EventRef event, void* typeList)
{
	const struct HIEventTypeList* list = typeList;

	if (!list || list->count == 0)
		return true;
	for (ItemCount i = 0; i < list->count; i++)
	{
		if (list->types[i].eventClass == event->eventClass && list->types[i].eventKind == event->kind)
			return true;
	}
	return false;
}

EventQueueRef GetMainEventQueue(void)
{
	return &_HIMainEventLoop()->queue;
}

EventQueueRef GetCurrentEventQueue(void)
{
	return &_HIEventLoopForCurrentThread()->queue;
}

// With the queue locked
static void unlinkEvent(EventQueueRef queue, int list, EventRef previous, EventRef event)
{
	if (previous)
		previous->queueNext = event->queueNext;
	else
		queue->head[list] = event->queueNext;
	if (queue->tail[list] == event)
		queue->tail[list] = previous;
	event->queueNext = NULL;
	atomic_store(&event->queue, NULL);
	queue->count--;
}

// Removes the event from the queue it is in, without releasing it. False if it is not in this queue.
static bool removeEvent(EventQueueRef queue, EventRef event)
{
	bool removed = false;

	pthread_mutex_lock(&queue->lock);
	for (int list = kListCount - 1; list >= 0 && !removed; list--)
	{
		EventRef previous = NULL;
		for (EventRef current = queue->head[list]; current; previous = current, current = current->queueNext)
		{
			if (current == event)
			{
				unlinkEvent(queue, list, previous, current);
				removed = true;
				break;
			}
		}
	}
	pthread_mutex_unlock(&queue->lock);
	return removed;
}

// Whether two mouse moved or dragged events are about the same window, and, for events made from
// CGEvents, the same buttons: a left drag does not replace a right drag.
static bool sameMotionTarget(EventRef a, EventRef b)
{
	if (a->cgEvent && b->cgEvent)
	{
		return CGEventGetType(a->cgEvent) == CGEventGetType(b->cgEvent)
			&& CGEventGetIntegerValueField(a->cgEvent, kCGMouseEventWindowUnderMousePointer)
				== CGEventGetIntegerValueField(b->cgEvent, kCGMouseEventWindowUnderMousePointer);
	}

	EventParamType typeA, typeB;
	ByteCount sizeA = 0, sizeB = 0;
	char windowA[16] = {0}, windowB[16] = {0};
	bool hasA = _HICopyEventParameter(a, 'wind', &typeA, &sizeA, windowA, sizeof(windowA));
	bool hasB = _HICopyEventParameter(b, 'wind', &typeB, &sizeB, windowB, sizeof(windowB));
	return hasA == hasB && (!hasA || (sizeA == sizeB && memcmp(windowA, windowB, sizeA) == 0));
}

static OSStatus postEvent(EventQueueRef queue, EventRef event, int list, bool coalesce, bool wake)
{
	if (!queue || !event)
		return paramErr;

	EventQueueRef none = NULL;
	if (!atomic_compare_exchange_strong(&event->queue, &none, queue))
		return eventAlreadyPostedErr;

	RetainEvent(event);

	pthread_mutex_lock(&queue->lock);
	if (coalesce && atomic_load(&g_mouseCoalescing) && event->eventClass == kEventClassMouse
		&& (event->kind == kEventMouseMoved || event->kind == kEventMouseDragged))
	{
		// The event at the end of the list is out of date if it is of the same kind for the same
		// window. Only the last event can be replaced: a click that was queued after an older
		// move must not end up before the move, which comes later.
		EventRef current = queue->tail[list];
		if (current && current->eventClass == event->eventClass && current->kind == event->kind
			&& sameMotionTarget(current, event))
		{
			// The deltas of a mouse event count from the previous one the application gets
			if (current->cgEvent && event->cgEvent)
			{
				CGEventSetIntegerValueField(event->cgEvent, kCGMouseEventDeltaX,
					CGEventGetIntegerValueField(event->cgEvent, kCGMouseEventDeltaX) + CGEventGetIntegerValueField(current->cgEvent, kCGMouseEventDeltaX));
				CGEventSetIntegerValueField(event->cgEvent, kCGMouseEventDeltaY,
					CGEventGetIntegerValueField(event->cgEvent, kCGMouseEventDeltaY) + CGEventGetIntegerValueField(current->cgEvent, kCGMouseEventDeltaY));
			}

			// The list is singly linked: find the event before the last one
			EventRef previous = NULL;
			for (EventRef walk = queue->head[list]; walk != current; walk = walk->queueNext)
				previous = walk;
			unlinkEvent(queue, list, previous, current);
			ReleaseEvent(current);
		}
	}
	event->queuePriority = list;
	event->queueNext = NULL;
	if (list == kListHead)
	{
		event->queueNext = queue->head[list];
		queue->head[list] = event;
		if (!queue->tail[list])
			queue->tail[list] = event;
	}
	else
	{
		if (queue->tail[list])
			queue->tail[list]->queueNext = event;
		else
			queue->head[list] = event;
		queue->tail[list] = event;
	}
	queue->count++;
	pthread_mutex_unlock(&queue->lock);

	if (wake)
		_HIWakeEventLoop(queue->loop);
	return noErr;
}

static int listForPriority(EventPriority priority)
{
	switch (priority)
	{
		case kEventPriorityHigh: return 2;
		case kEventPriorityStandard: return 1;
		case kEventPriorityLow: return 0;
		default: return -1;
	}
}

OSStatus PostEventToQueue(EventQueueRef inQueue, EventRef inEvent, EventPriority inPriority)
{
	int list = listForPriority(inPriority);

	if (list < 0)
		return paramErr;
	return postEvent(inQueue, inEvent, list, false, true);
}

OSStatus _HIPostEventToQueueCoalescing(EventQueueRef queue, EventRef event, EventPriority priority)
{
	int list = listForPriority(priority);

	if (list < 0)
		return paramErr;
	return postEvent(queue, event, list, true, true);
}

OSStatus _HIPostEventFromWaitingThread(EventQueueRef queue, EventRef event)
{
	return postEvent(queue, event, listForPriority(kEventPriorityStandard), true, false);
}

OSStatus _HIPostEventToQueueAtHead(EventQueueRef queue, EventRef event)
{
	return postEvent(queue, event, kListHead, false, true);
}

OSStatus RemoveEventFromQueue(EventQueueRef inQueue, EventRef inEvent)
{
	if (!inQueue || !inEvent)
		return paramErr;
	if (!removeEvent(inQueue, inEvent))
		return eventNotInQueueErr;
	ReleaseEvent(inEvent);
	return noErr;
}

Boolean IsEventInQueue(EventQueueRef inQueue, EventRef inEvent)
{
	return inQueue && inEvent && atomic_load(&inEvent->queue) == inQueue;
}

ItemCount GetNumEventsInQueue(EventQueueRef inQueue)
{
	ItemCount count;

	if (!inQueue)
		return 0;
	pthread_mutex_lock(&inQueue->lock);
	count = inQueue->count;
	pthread_mutex_unlock(&inQueue->lock);
	return count;
}

// The queued events in the order they are taken, each retained. Free the array with
// releaseSnapshot().
static ItemCount snapshotQueue(EventQueueRef queue, EventRef** outEvents)
{
	pthread_mutex_lock(&queue->lock);
	EventRef* events = malloc((queue->count ? queue->count : 1) * sizeof(EventRef));
	ItemCount count = 0;

	if (events)
	{
		for (int list = kListCount - 1; list >= 0; list--)
		{
			for (EventRef event = queue->head[list]; event; event = event->queueNext)
				events[count++] = RetainEvent(event);
		}
	}
	pthread_mutex_unlock(&queue->lock);
	*outEvents = events;
	return count;
}

static void releaseSnapshot(EventRef* events, ItemCount count)
{
	for (ItemCount i = 0; i < count; i++)
		ReleaseEvent(events[i]);
	free(events);
}

EventRef _HIQueueTakeMatching(EventQueueRef queue, Boolean (*match)(EventRef, void*), void* info, bool pure, bool pull)
{
	EventRef found = NULL;

	if (!queue)
		return NULL;

	if (pure || !match)
	{
		pthread_mutex_lock(&queue->lock);
		for (int list = kListCount - 1; list >= 0 && !found; list--)
		{
			EventRef previous = NULL;
			for (EventRef event = queue->head[list]; event; previous = event, event = event->queueNext)
			{
				if (match && !match(event, info))
					continue;
				found = event;
				if (pull)
					unlinkEvent(queue, list, previous, event);
				break;
			}
		}
		pthread_mutex_unlock(&queue->lock);
		return found;
	}

	// The filter may call the queue functions: it works on a snapshot, and the event it picks
	// is taken if no other thread took it meanwhile
	EventRef* events;
	ItemCount count = snapshotQueue(queue, &events);
	for (ItemCount i = 0; i < count && !found; i++)
	{
		if (!match(events[i], info))
			continue;
		if (!pull)
		{
			if (IsEventInQueue(queue, events[i]))
				found = events[i];
		}
		else if (removeEvent(queue, events[i]))
			found = events[i];
	}
	releaseSnapshot(events, count);
	// A pulled event's reference is the one the queue had; the snapshot's own was released
	return found;
}


OSStatus _HIFlushEventsMatching(EventQueueRef queue, HIEventMatchProc match, void* info, EventRef beforeEvent)
{
	if (!queue)
		return paramErr;

	EventRef* events;
	ItemCount count = snapshotQueue(queue, &events);
	if (!events)
		return memFullErr;

	for (ItemCount i = 0; i < count; i++)
	{
		if (events[i] == beforeEvent)
			break;
		if (match && !match(events[i], info))
			continue;
		if (removeEvent(queue, events[i]))
			ReleaseEvent(events[i]);
	}
	releaseSnapshot(events, count);
	return noErr;
}

OSStatus FlushEventQueue(EventQueueRef inQueue)
{
	return _HIFlushEventsMatching(inQueue, NULL, NULL, NULL);
}

OSStatus FlushEventsMatchingListFromQueue(EventQueueRef inQueue, ItemCount inNumTypes, const EventTypeSpec* inList)
{
	if (!inQueue || !inList || inNumTypes == 0)
		return paramErr;

	struct HIEventTypeList list = { inNumTypes, inList };
	return _HIFlushEventsMatching(inQueue, _HIMatchesEventTypes, &list, NULL);
}

OSStatus FlushSpecificEventsFromQueue(EventQueueRef inQueue, EventComparatorUPP inComparator, void* inCompareData)
{
	if (!inQueue || !inComparator)
		return paramErr;
	return _HIFlushEventsMatching(inQueue, inComparator, inCompareData, NULL);
}

EventRef FindSpecificEventInQueue(EventQueueRef inQueue, EventComparatorUPP inComparator, void* inCompareData)
{
	if (!inQueue || !inComparator)
		return NULL;
	return _HIQueueTakeMatching(inQueue, inComparator, inCompareData, false, false);
}

EventRef AcquireFirstMatchingEventInQueue(EventQueueRef inQueue, ItemCount inNumTypes, const EventTypeSpec* inList, OptionBits inOptions)
{
	if (!inQueue || (inNumTypes != 0 && !inList))
		return NULL;

	struct HIEventTypeList list = { inNumTypes, inList };
	EventRef event = _HIQueueTakeMatching(inQueue, _HIMatchesEventTypes, &list, true, false);
	// The caller owns a reference of the event, which stays in the queue
	return RetainEvent(event);
}

OSStatus SetMouseCoalescingEnabled(Boolean inNewState, Boolean* outOldState)
{
	bool old = atomic_exchange(&g_mouseCoalescing, inNewState != 0);

	if (outOldState)
		*outOldState = old;
	return noErr;
}

Boolean IsMouseCoalescingEnabled(void)
{
	return atomic_load(&g_mouseCoalescing);
}

EventComparatorUPP NewEventComparatorUPP(EventComparatorProcPtr userRoutine)
{
	return userRoutine;
}

void DisposeEventComparatorUPP(EventComparatorUPP userUPP)
{
}

// Drops the events of the queue of a thread that ends
void _HIDiscardQueuedEvents(EventQueueRef queue)
{
	_HIFlushEventsMatching(queue, NULL, NULL, NULL);
}
