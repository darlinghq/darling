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

// Carbon events and event handlers: the event objects (a class, a kind, a time and typed
// parameters), the event targets of the process, handler installation and dispatch.
//
// Two targets exist, as in the documented model: the event dispatcher target, whose
// handlers see an event first, and the application target behind it. Events sent to a
// target go to the handlers installed on it, newest first, until one of them handles the
// event; the dispatcher target passes an event on to the application target when its
// handlers did not handle it, or asked to move on. Windows, controls and menus have no
// targets of their own here.

#include <HIToolbox/CarbonEventsCore.h>
#include "HIToolboxInternal.h"
#include <HIToolbox/CarbonEvents.h>
#include <CarbonCore/MacErrors.h>
#include <CoreGraphics/CGEvent.h>

#include <mach/mach_time.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

struct EventParameter
{
	struct EventParameter* next;
	OSType name;
	OSType type;
	ByteCount size;
	char data[];
};

struct OpaqueEventRef
{
	atomic_uint retainCount;
	OSType eventClass;
	UInt32 kind;
	EventTime time;
	EventAttributes attributes;
	struct EventParameter* parameters;
	CGEventRef cgEvent;   // for the events made from a CGEvent
};

struct EventHandlerEntry
{
	struct EventHandlerEntry* next;
	struct EventTarget* target;
	EventHandlerProcPtr proc;
	void* userData;
	EventTypeSpec* types;
	ItemCount typeCount;
	bool removed;
};

struct EventTarget
{
	struct EventHandlerEntry* handlers;  // newest first
	struct EventTarget* next;            // the target behind this one
};

// What EventHandlerCallRef points to while a handler runs
struct HandlerCall
{
	struct EventTarget* target;
	struct EventHandlerEntry* handler;
	EventRef event;
	int nextCalled;	// the handler passed the event on itself with CallNextEventHandler
};

// The standard handlers of the event dispatcher target. They are the oldest handlers, so every
// handler an application installs sees an event before them. They cannot be removed.
extern OSStatus _HIStandardRawEventHandler(EventHandlerCallRef call, EventRef event, void* userData);
static EventTypeSpec g_standardRawEventTypes[1] = { { kHIEventClassRaw, kHIEventRawSystemDefined } };
static struct EventHandlerEntry g_standardRawEventHandler = {
	NULL, NULL, _HIStandardRawEventHandler, NULL, g_standardRawEventTypes, 1, false
};

static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static struct EventTarget g_applicationTarget = { NULL, NULL };
static struct EventTarget g_dispatcherTarget = { &g_standardRawEventHandler, &g_applicationTarget };

// Handlers removed while an event is being dispatched stay allocated (a dispatch may still
// hold a pointer to them) and are freed when no dispatch is running.
static int g_dispatchDepth;
static struct EventHandlerEntry* g_removedHandlers;

static EventTime currentTime(void)
{
	static mach_timebase_info_data_t timebase;

	if (timebase.denom == 0)
		mach_timebase_info(&timebase);
	return (double) mach_absolute_time() * timebase.numer / timebase.denom / 1e9;
}

EventTargetRef GetApplicationEventTarget(void)
{
	return &g_applicationTarget;
}

EventTargetRef GetEventDispatcherTarget(void)
{
	return &g_dispatcherTarget;
}

OSStatus CreateEvent(CFAllocatorRef inAllocator, OSType inClassID, UInt32 inKind, EventTime inEventTime, EventAttributes inAttributes, EventRef* outEvent)
{
	if (!outEvent)
		return paramErr;

	struct OpaqueEventRef* event = calloc(1, sizeof(*event));
	if (!event)
		return memFullErr;

	atomic_init(&event->retainCount, 1);
	event->eventClass = inClassID;
	event->kind = inKind;
	// A time of zero means now
	event->time = inEventTime != 0 ? inEventTime : currentTime();
	event->attributes = inAttributes;
	*outEvent = event;
	return noErr;
}

OSStatus CreateEventWithCGEvent(CFAllocatorRef inAllocator, CGEventRef inEvent, EventAttributes inAttributes, EventRef* outEvent)
{
	if (!inEvent || !outEvent)
		return paramErr;

	// Events made from a CGEvent are of the raw event class; the kind is the CGEvent's type (as
	// observed on macOS 26.7.3: a system-defined event gives class 'cgs ' and kind 14, tier 2,
	// mac-probes/v443-systemdefined-carbon-event.m).
	EventTime time = (double) CGEventGetTimestamp(inEvent) / 1e9;
	OSStatus status = CreateEvent(inAllocator, kHIEventClassRaw, (UInt32) CGEventGetType(inEvent), time, inAttributes, outEvent);

	if (status == noErr)
		(*outEvent)->cgEvent = (CGEventRef) CFRetain(inEvent);
	return status;
}

CGEventRef CopyEventCGEvent(EventRef inEvent)
{
	if (!inEvent || !inEvent->cgEvent)
		return NULL;
	return (CGEventRef) CFRetain(inEvent->cgEvent);
}

EventRef RetainEvent(EventRef inEvent)
{
	if (inEvent)
		atomic_fetch_add(&inEvent->retainCount, 1);
	return inEvent;
}

UInt32 GetEventRetainCount(EventRef inEvent)
{
	return inEvent ? atomic_load(&inEvent->retainCount) : 0;
}

void ReleaseEvent(EventRef inEvent)
{
	if (!inEvent || atomic_fetch_sub(&inEvent->retainCount, 1) != 1)
		return;

	struct EventParameter* parameter = inEvent->parameters;
	while (parameter)
	{
		struct EventParameter* next = parameter->next;
		free(parameter);
		parameter = next;
	}
	if (inEvent->cgEvent)
		CFRelease(inEvent->cgEvent);
	free(inEvent);
}

OSType GetEventClass(EventRef inEvent)
{
	return inEvent ? inEvent->eventClass : 0;
}

UInt32 GetEventKind(EventRef inEvent)
{
	return inEvent ? inEvent->kind : 0;
}

EventTime GetEventTime(EventRef inEvent)
{
	return inEvent ? inEvent->time : 0;
}

OSStatus SetEventParameter(EventRef inEvent, EventParamName inName, EventParamType inType, ByteCount inSize, const void* inDataPtr)
{
	if (!inEvent || (inSize != 0 && !inDataPtr))
		return paramErr;

	struct EventParameter* parameter = calloc(1, sizeof(*parameter) + inSize);
	if (!parameter)
		return memFullErr;
	parameter->name = inName;
	parameter->type = inType;
	parameter->size = inSize;
	if (inSize)
		memcpy(parameter->data, inDataPtr, inSize);

	// A parameter set again replaces the old value
	struct EventParameter** link = &inEvent->parameters;
	while (*link)
	{
		if ((*link)->name == inName)
		{
			struct EventParameter* old = *link;
			*link = old->next;
			free(old);
			break;
		}
		link = &(*link)->next;
	}
	parameter->next = inEvent->parameters;
	inEvent->parameters = parameter;
	return noErr;
}

OSStatus GetEventParameter(EventRef inEvent, EventParamName inName, EventParamType inDesiredType, EventParamType* outActualType, ByteCount inBufferSize, ByteCount* outActualSize, void* outData)
{
	if (!inEvent)
		return paramErr;

	struct EventParameter* parameter = inEvent->parameters;
	while (parameter && parameter->name != inName)
		parameter = parameter->next;
	if (!parameter)
		return eventParameterNotFoundErr;

	if (outActualType)
		*outActualType = parameter->type;
	if (outActualSize)
		*outActualSize = parameter->size;

	// The parameter is only given out as the type it was stored as. A wildcard desired
	// type is not accepted.
	if (inDesiredType != parameter->type)
		return inDesiredType == '****' ? paramErr : errAECoercionFail;

	// A buffer that is too small gets the beginning of the value; the actual size says
	// how much there is. Without a buffer only the size is returned.
	if (outData)
		memcpy(outData, parameter->data, inBufferSize < parameter->size ? inBufferSize : parameter->size);
	return noErr;
}

EventHandlerUPP NewEventHandlerUPP(EventHandlerProcPtr userRoutine)
{
	return userRoutine;
}

void DisposeEventHandlerUPP(EventHandlerUPP userUPP)
{
}

OSStatus InvokeEventHandlerUPP(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void* inUserData, EventHandlerUPP userUPP)
{
	return userUPP(inHandlerCallRef, inEvent, inUserData);
}

static bool isTarget(EventTargetRef target)
{
	return target == &g_applicationTarget || target == &g_dispatcherTarget;
}

static void freeHandler(struct EventHandlerEntry* handler)
{
	free(handler->types);
	free(handler);
}

OSStatus InstallEventHandler(EventTargetRef inTarget, EventHandlerUPP inHandler, ItemCount inNumTypes, const EventTypeSpec* inList, void* inUserData, EventHandlerRef* outRef)
{
	if (!isTarget(inTarget) || !inHandler)
		return paramErr;

	// QUIRK: a handler installed for no event types (a count of zero, or no list) is
	// accepted; it never sees an event. Not documented. Evidence: black-box observation on
	// macOS 26.7.3 (tier 2, mac-probes/v443-handler-bad-arguments.c).
	// Test: quirk_handler_for_no_event_types in tests/src/carbon_hotkeys.c
	if (!inList)
		inNumTypes = 0;

	struct EventTarget* target = inTarget;
	struct EventHandlerEntry* handler = calloc(1, sizeof(*handler));
	if (!handler)
		return memFullErr;
	handler->types = malloc((inNumTypes ? inNumTypes : 1) * sizeof(EventTypeSpec));
	if (!handler->types)
	{
		free(handler);
		return memFullErr;
	}
	if (inNumTypes)
		memcpy(handler->types, inList, inNumTypes * sizeof(EventTypeSpec));
	handler->typeCount = inNumTypes;
	handler->target = target;
	handler->proc = inHandler;
	handler->userData = inUserData;

	pthread_mutex_lock(&g_lock);
	// The same routine with the same user data cannot be installed twice on a target,
	// whatever the event types
	for (struct EventHandlerEntry* other = target->handlers; other; other = other->next)
	{
		if (other->proc == inHandler && other->userData == inUserData)
		{
			pthread_mutex_unlock(&g_lock);
			freeHandler(handler);
			return eventHandlerAlreadyInstalledErr;
		}
	}
	handler->next = target->handlers;
	target->handlers = handler;
	pthread_mutex_unlock(&g_lock);

	if (outRef)
		*outRef = handler;
	return noErr;
}

OSStatus RemoveEventHandler(EventHandlerRef inHandlerRef)
{
	struct EventHandlerEntry* handler = inHandlerRef;
	OSStatus result = paramErr;

	if (!handler)
		return paramErr;

	pthread_mutex_lock(&g_lock);
	struct EventTarget* targets[2] = { &g_applicationTarget, &g_dispatcherTarget };
	for (int i = 0; i < 2 && result != noErr; i++)
	{
		for (struct EventHandlerEntry** link = &targets[i]->handlers; *link; link = &(*link)->next)
		{
			if (*link != handler)
				continue;
			// The entry keeps its next pointer so that a dispatch standing on it can go on
			*link = handler->next;
			handler->removed = true;
			if (g_dispatchDepth > 0)
			{
				// Chained through the target pointer, which is no longer needed
				handler->target = (struct EventTarget*) g_removedHandlers;
				g_removedHandlers = handler;
			}
			else
				freeHandler(handler);
			result = noErr;
			break;
		}
	}
	pthread_mutex_unlock(&g_lock);
	return result;
}

static bool handlerWants(const struct EventHandlerEntry* handler, EventRef event)
{
	for (ItemCount i = 0; i < handler->typeCount; i++)
	{
		if (handler->types[i].eventClass == event->eventClass && handler->types[i].eventKind == event->kind)
			return true;
	}
	return false;
}

// The result of the handlers from start on, newest first. eventNotHandledErr if none handled
// the event; eventPassToNextTargetErr if one asked to go on with the next target.
static OSStatus dispatchFrom(struct EventTarget* target, struct EventHandlerEntry* start, EventRef event)
{
	struct EventHandlerEntry* handler = start;

	for (;;)
	{
		EventHandlerProcPtr proc = NULL;
		void* userData = NULL;

		pthread_mutex_lock(&g_lock);
		while (handler && (handler->removed || !handlerWants(handler, event)))
			handler = handler->next;
		if (handler)
		{
			proc = handler->proc;
			userData = handler->userData;
		}
		pthread_mutex_unlock(&g_lock);

		if (!handler)
			return eventNotHandledErr;

		struct HandlerCall call = { target, handler, event, 0 };
		OSStatus result = proc(&call, event, userData);

		// A handler that has already passed the event on has had the rest of the handlers
		// try it; its own "not handled" is the answer of the whole list.
		if (result != eventNotHandledErr || call.nextCalled)
			return result;

		pthread_mutex_lock(&g_lock);
		handler = handler->next;
		pthread_mutex_unlock(&g_lock);
	}
}

static void beginDispatch(void)
{
	pthread_mutex_lock(&g_lock);
	g_dispatchDepth++;
	pthread_mutex_unlock(&g_lock);
}

static void endDispatch(void)
{
	pthread_mutex_lock(&g_lock);
	if (--g_dispatchDepth == 0)
	{
		while (g_removedHandlers)
		{
			struct EventHandlerEntry* handler = g_removedHandlers;
			g_removedHandlers = (struct EventHandlerEntry*) handler->target;
			freeHandler(handler);
		}
	}
	pthread_mutex_unlock(&g_lock);
}

OSStatus SendEventToEventTarget(EventRef inEvent, EventTargetRef inTarget)
{
	if (!isTarget(inTarget))
		return paramErr;

	// QUIRK: no event is reported as not handled, not as a bad argument. Not documented.
	// Evidence: black-box observation on macOS 26.7.3 (tier 2,
	// mac-probes/v443-handler-bad-arguments.c).
	// Test: quirk_send_no_event_is_not_handled in tests/src/carbon_hotkeys.c
	if (!inEvent)
		return eventNotHandledErr;

	OSStatus result = eventNotHandledErr;

	beginDispatch();
	for (struct EventTarget* target = inTarget; target; target = target->next)
	{
		struct EventHandlerEntry* first;

		pthread_mutex_lock(&g_lock);
		first = target->handlers;
		pthread_mutex_unlock(&g_lock);

		result = dispatchFrom(target, first, inEvent);
		if (result != eventNotHandledErr && result != eventPassToNextTargetErr)
			break;
	}
	endDispatch();

	// Nothing left to pass the event to
	return result == eventPassToNextTargetErr ? eventNotHandledErr : result;
}

OSStatus CallNextEventHandler(EventHandlerCallRef inCallRef, EventRef inEvent)
{
	struct HandlerCall* call = inCallRef;

	if (!call || !inEvent)
		return paramErr;

	struct EventHandlerEntry* next;
	pthread_mutex_lock(&g_lock);
	next = call->handler->next;
	pthread_mutex_unlock(&g_lock);

	call->nextCalled = 1;
	OSStatus result = dispatchFrom(call->target, next, inEvent);

	// Leaving the target is up to the handler that called this: it returns the status
	return result;
}
