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
#include <AE/AEDataModel.h>
#include <CoreGraphics/CGEvent.h>
#include <CoreGraphics/CGEventSource.h>

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
	struct EventTarget* nextCreated;     // the targets made by _HICreateEventTarget(), newest first
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
// The standard handler of the events of the windows: the events for a window go to the window's own
// handlers (MacWindows.c). Behind the raw one, so every handler of the application sees an event before them.
extern OSStatus _HIStandardWindowEventHandler(EventHandlerCallRef call, EventRef event, void* userData);
extern const EventTypeSpec _HIStandardWindowEventTypes[];
static struct EventHandlerEntry g_standardWindowEventHandler = {
	NULL, NULL, _HIStandardWindowEventHandler, NULL, (EventTypeSpec*) _HIStandardWindowEventTypes, kHIStandardWindowEventTypeCount, false
};
static EventTypeSpec g_standardRawEventTypes[1] = { { kHIEventClassRaw, kHIEventRawSystemDefined } };
static struct EventHandlerEntry g_standardRawEventHandler = {
	&g_standardWindowEventHandler, NULL, _HIStandardRawEventHandler, NULL, g_standardRawEventTypes, 1, false
};

// The standard handler of the application target: the quit command ends the application event loop, and
// the menu bar's key equivalents and mouse clicks are handled
static EventTypeSpec g_standardApplicationTypes[5] = {
	{ kEventClassCommand, kEventCommandProcess },
	{ kEventClassCommand, kEventCommandUpdateStatus },
	{ kEventClassKeyboard, kEventRawKeyDown },
	{ kEventClassKeyboard, kEventRawKeyRepeat },
	{ kEventClassMouse, kEventMouseDown },
};
static struct EventHandlerEntry g_standardApplicationHandler = {
	NULL, NULL, _HIStandardApplicationHandler, NULL, g_standardApplicationTypes, 5, false
};

static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static struct EventTarget g_applicationTarget = { &g_standardApplicationHandler, NULL, NULL };
static struct EventTarget g_dispatcherTarget = { &g_standardRawEventHandler, &g_applicationTarget, NULL };
// The event monitor target: handlers installed on it only observe events sent to it.
static struct EventTarget g_monitorTarget = { NULL, NULL, NULL };
// The targets of the objects of the frameworks (menus, ...), see _HICreateEventTarget()
static struct EventTarget* g_createdTargets;
// Targets destroyed while a dispatch was running (it may stand on them), freed when it is over
static struct EventTarget* g_destroyedTargets;

// Handlers removed while an event is being dispatched stay allocated (a dispatch may still
// hold a pointer to them) and are freed when no dispatch is running.
static int g_dispatchDepth;
static struct EventHandlerEntry* g_removedHandlers;

// The event of the event loop dispatch that is running on the thread, see GetCurrentEvent()
static _Thread_local EventRef g_currentEvent;

EventRef _HISetCurrentEvent(EventRef event)
{
	EventRef previous = g_currentEvent;

	g_currentEvent = event;
	return previous;
}

EventRef GetCurrentEvent(void)
{
	return g_currentEvent;
}

EventTime _HICurrentTime(void);

static EventTime currentTime(void)
{
	return _HICurrentTime();
}

EventTime _HICurrentTime(void)
{
	static mach_timebase_info_data_t timebase;

	if (timebase.denom == 0)
		mach_timebase_info(&timebase);
	return (double) mach_absolute_time() * timebase.numer / timebase.denom / 1e9;
}

EventTargetRef _HIApplicationEventTarget(void)
{
	return &g_applicationTarget;
}

EventTargetRef GetApplicationEventTarget(void)
{
	// A process that uses the application target gets the events about the front process.
	_HIObserveFrontProcess();
	return &g_applicationTarget;
}

EventTargetRef GetEventDispatcherTarget(void)
{
	return &g_dispatcherTarget;
}

EventTargetRef GetEventMonitorTarget(void)
{
	return &g_monitorTarget;
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

// The Carbon class and kind of a CGEvent type. Events of the mouse and keyboard classes are what
// Carbon handlers of the mouse and keyboard classes expect; the others keep the raw class.
void _HIClassAndKindForCGEventType(CGEventType type, OSType* outClass, UInt32* outKind)
{
	// QUIRK: mouse, scroll wheel and keyboard CGEvents give events of the 'mous' and 'keyb'
	// classes (with the kinds of the documented Carbon mouse and raw keyboard events), and not
	// the raw class that every other type gives. Not documented for this function. Evidence:
	// black-box observation on macOS 26.7.3 (tier 2, mac-probes/937-p-events.m; the same
	// observation is pinned in tests/src/carbon_event_additions.c).
	// Test: test_quirk_cgevent_classes in tests/src/carbon_event_additions.c
	*outClass = kHIEventClassRaw;
	*outKind = (UInt32) type;

	switch (type)
	{
		case kCGEventLeftMouseDown:
		case kCGEventRightMouseDown:
		case kCGEventOtherMouseDown:
			*outClass = kEventClassMouse;
			*outKind = kEventMouseDown;
			break;
		case kCGEventLeftMouseUp:
		case kCGEventRightMouseUp:
		case kCGEventOtherMouseUp:
			*outClass = kEventClassMouse;
			*outKind = kEventMouseUp;
			break;
		case kCGEventMouseMoved:
			*outClass = kEventClassMouse;
			*outKind = kEventMouseMoved;
			break;
		case kCGEventLeftMouseDragged:
		case kCGEventRightMouseDragged:
		case kCGEventOtherMouseDragged:
			*outClass = kEventClassMouse;
			*outKind = kEventMouseDragged;
			break;
		case kCGEventScrollWheel:
			*outClass = kEventClassMouse;
			*outKind = kEventMouseWheelMoved;
			break;
		case kCGEventKeyDown:
			*outClass = kEventClassKeyboard;
			*outKind = kEventRawKeyDown;
			break;
		case kCGEventKeyUp:
			*outClass = kEventClassKeyboard;
			*outKind = kEventRawKeyUp;
			break;
		case kCGEventFlagsChanged:
			*outClass = kEventClassKeyboard;
			*outKind = kEventRawKeyModifiersChanged;
			break;
		default:
			break;
	}
}

OSStatus CreateEventWithCGEvent(CFAllocatorRef inAllocator, CGEventRef inEvent, EventAttributes inAttributes, EventRef* outEvent)
{
	if (!inEvent || !outEvent)
		return paramErr;

	OSType eventClass;
	UInt32 kind;
	_HIClassAndKindForCGEventType(CGEventGetType(inEvent), &eventClass, &kind);

	EventTime time = (double) CGEventGetTimestamp(inEvent) / 1e9;
	OSStatus status = CreateEvent(inAllocator, eventClass, kind, time, inAttributes, outEvent);

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

EventRef CopyEvent(EventRef inOther)
{
	if (!inOther)
		return NULL;

	EventRef copy = NULL;
	if (CreateEvent(NULL, inOther->eventClass, inOther->kind, inOther->time, inOther->attributes, &copy) != noErr)
		return NULL;

	// The parameters are copied in the order they were set, which keeps the same lookup order
	struct EventParameter* source[64];
	size_t count = 0;
	for (struct EventParameter* parameter = inOther->parameters; parameter; parameter = parameter->next)
	{
		if (count == sizeof(source) / sizeof(source[0]))
			break;
		source[count++] = parameter;
	}
	while (count > 0)
	{
		struct EventParameter* parameter = source[--count];
		if (SetEventParameter(copy, parameter->name, parameter->type, parameter->size, parameter->data) != noErr)
		{
			ReleaseEvent(copy);
			return NULL;
		}
	}
	if (inOther->cgEvent)
		copy->cgEvent = (CGEventRef) CFRetain(inOther->cgEvent);
	return copy;
}

OSStatus SetEventTime(EventRef inEvent, EventTime inTime)
{
	if (!inEvent)
		return paramErr;
	inEvent->time = inTime;
	return noErr;
}

OSStatus AEProcessEvent(EventRef inEvent)
{
	if (!inEvent)
		return paramErr;
	if (inEvent->eventClass != kEventClassAppleEvent)
		return paramErr;

	// The Apple event is identified by its class and ID
	OSType eventClass, eventID;
	OSStatus status = GetEventParameter(inEvent, kEventParamAEEventClass, typeType, NULL, sizeof(eventClass), NULL, &eventClass);
	if (status == noErr)
		status = GetEventParameter(inEvent, kEventParamAEEventID, typeType, NULL, sizeof(eventID), NULL, &eventID);
	if (status != noErr)
		return status;

	// A Carbon event carries no Apple event data here, so there is nothing to hand to the
	// Apple event handlers
	return errAEEventNotHandled;
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

static struct EventParameter* findParameter(EventRef inEvent, EventParamName inName)
{
	struct EventParameter* parameter = inEvent->parameters;

	while (parameter && parameter->name != inName)
		parameter = parameter->next;
	return parameter;
}

bool _HICopyEventParameter(EventRef inEvent, EventParamName inName, EventParamType* outType, ByteCount* outSize, void* outData, ByteCount dataCapacity)
{
	struct EventParameter* parameter = findParameter(inEvent, inName);

	if (!parameter)
		return false;
	*outType = parameter->type;
	*outSize = parameter->size;
	if (outData)
		memcpy(outData, parameter->data, dataCapacity < parameter->size ? dataCapacity : parameter->size);
	return true;
}

// The left and right button events name their button by their type; the others, and the events
// that no button caused, by the button number field
static EventMouseButton mouseButtonOfCGEvent(CGEventRef cgEvent)
{
	switch (CGEventGetType(cgEvent))
	{
		case kCGEventLeftMouseDown:
		case kCGEventLeftMouseUp:
		case kCGEventLeftMouseDragged:
			return kEventMouseButtonPrimary;
		case kCGEventRightMouseDown:
		case kCGEventRightMouseUp:
		case kCGEventRightMouseDragged:
			return kEventMouseButtonSecondary;
		default:
			return (EventMouseButton) (CGEventGetIntegerValueField(cgEvent, kCGMouseEventButtonNumber) + 1);
	}
}

// The parameters a mouse event gets from its CGEvent: where it happened, the click count, the
// button and the modifiers; a raw keyboard event: the key code and the modifiers. They are worked out when asked for, so that an event costs nothing
// for the parameters nobody reads.
bool _HICopyCGEventParameter(EventRef inEvent, EventParamName inName, EventParamType* outType, ByteCount* outSize, void* outData, ByteCount dataCapacity)
{
	union { HIPoint point; UInt32 count; EventMouseButton button; UInt32 modifiers; } value;

	if (!inEvent->cgEvent || (inEvent->eventClass != kEventClassMouse && inEvent->eventClass != kEventClassKeyboard))
		return false;

	// The raw keyboard events have the key code and the modifiers only
	if (inEvent->eventClass == kEventClassKeyboard && inName != kEventParamKeyCode && inName != kEventParamKeyModifiers)
		return false;

	switch (inName)
	{
		case kEventParamKeyCode:
			if (inEvent->eventClass != kEventClassKeyboard)
				return false;
			value.count = (UInt32) CGEventGetIntegerValueField(inEvent->cgEvent, kCGKeyboardEventKeycode);
			*outType = typeUInt32;
			*outSize = sizeof(value.count);
			break;
		case kEventParamMouseLocation:
		{
			if (inEvent->eventClass != kEventClassMouse)
				return false;
			CGPoint location = CGEventGetLocation(inEvent->cgEvent);
			value.point.x = location.x;
			value.point.y = location.y;
			*outType = typeHIPoint;
			*outSize = sizeof(value.point);
			break;
		}
		case kEventParamClickCount:
			if (inEvent->eventClass != kEventClassMouse)
				return false;
			value.count = (UInt32) CGEventGetIntegerValueField(inEvent->cgEvent, kCGMouseEventClickState);
			*outType = typeUInt32;
			*outSize = sizeof(value.count);
			break;
		case kEventParamMouseButton:
			if (inEvent->eventClass != kEventClassMouse)
				return false;
			value.button = mouseButtonOfCGEvent(inEvent->cgEvent);
			*outType = typeMouseButton;
			*outSize = sizeof(value.button);
			break;
		case kEventParamKeyModifiers:
			value.modifiers = _HIModifiersFromCGEventFlags(CGEventGetFlags(inEvent->cgEvent));
			*outType = typeUInt32;
			*outSize = sizeof(value.modifiers);
			break;
		default:
			return false;
	}
	if (outData)
		memcpy(outData, &value, dataCapacity < *outSize ? dataCapacity : *outSize);
	return true;
}

OSStatus GetEventParameter(EventRef inEvent, EventParamName inName, EventParamType inDesiredType, EventParamType* outActualType, ByteCount inBufferSize, ByteCount* outActualSize, void* outData)
{
	if (!inEvent)
		return paramErr;

	EventParamType type;
	ByteCount size;
	char derived[sizeof(HIPoint)];
	struct EventParameter* parameter = findParameter(inEvent, inName);
	const void* data;

	if (parameter)
	{
		type = parameter->type;
		size = parameter->size;
		data = parameter->data;
	}
	else if (_HICopyCGEventParameter(inEvent, inName, &type, &size, derived, sizeof(derived)))
		data = derived;
	else
		return eventParameterNotFoundErr;

	if (outActualType)
		*outActualType = type;
	if (outActualSize)
		*outActualSize = size;

	// The parameter is only given out as the type it was stored as. A wildcard desired
	// type is not accepted.
	if (inDesiredType != type)
		return inDesiredType == '****' ? paramErr : errAECoercionFail;

	// A buffer that is too small gets the beginning of the value; the actual size says
	// how much there is. Without a buffer only the size is returned.
	if (outData)
		memcpy(outData, data, inBufferSize < size ? inBufferSize : size);
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

static void freeHandler(struct EventHandlerEntry* handler);

static bool isTarget(EventTargetRef target)
{
	if (target == &g_applicationTarget || target == &g_dispatcherTarget || target == &g_monitorTarget)
		return true;

	pthread_mutex_lock(&g_lock);
	struct EventTarget* other = g_createdTargets;
	while (other && other != target)
		other = other->nextCreated;
	pthread_mutex_unlock(&g_lock);
	return other != NULL;
}

EventTargetRef _HICreateEventTarget(EventTargetRef next)
{
	struct EventTarget* target = calloc(1, sizeof(*target));

	if (!target)
		return NULL;
	target->next = next;
	pthread_mutex_lock(&g_lock);
	target->nextCreated = g_createdTargets;
	g_createdTargets = target;
	pthread_mutex_unlock(&g_lock);
	return target;
}

void _HISetEventTargetNext(EventTargetRef inTarget, EventTargetRef next)
{
	struct EventTarget* target = inTarget;

	pthread_mutex_lock(&g_lock);
	target->next = next ? next : (struct EventTarget*) GetApplicationEventTarget();
	pthread_mutex_unlock(&g_lock);
}

void _HIDestroyEventTarget(EventTargetRef inTarget)
{
	struct EventTarget* target = inTarget;

	pthread_mutex_lock(&g_lock);
	for (struct EventTarget** link = &g_createdTargets; *link; link = &(*link)->nextCreated)
	{
		if (*link != target)
			continue;
		*link = target->nextCreated;
		break;
	}
	// The handlers go with the target. One that is running now stays allocated until the
	// dispatch is over, as for RemoveEventHandler().
	struct EventHandlerEntry* handler = target->handlers;
	target->handlers = NULL;
	while (handler)
	{
		struct EventHandlerEntry* following = handler->next;

		handler->removed = true;
		if (g_dispatchDepth > 0)
		{
			handler->target = (struct EventTarget*) g_removedHandlers;
			g_removedHandlers = handler;
		}
		else
			freeHandler(handler);
		handler = following;
	}
	// A dispatch may still be standing on the target (a window disposes of itself from its own handler)
	if (g_dispatchDepth == 0)
		free(target);
	else
	{
		target->nextCreated = g_destroyedTargets;
		g_destroyedTargets = target;
	}
	pthread_mutex_unlock(&g_lock);
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
	struct EventTarget* targets[3] = { &g_applicationTarget, &g_dispatcherTarget, &g_monitorTarget };
	struct EventTarget* created = g_createdTargets;
	for (int i = 0; result != noErr; i++)
	{
		struct EventTarget* current;

		if (i < 3)
			current = targets[i];
		else if (created)
		{
			current = created;
			created = created->nextCreated;
		}
		else
			break;
		for (struct EventHandlerEntry** link = &current->handlers; *link; link = &(*link)->next)
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

// The handler an EventHandlerRef refers to, or NULL if the reference is not an installed handler.
// Called with g_lock held.
static struct EventHandlerEntry* findInstalledHandler(EventHandlerRef ref)
{
	struct EventTarget* targets[3] = { &g_applicationTarget, &g_dispatcherTarget, &g_monitorTarget };
	struct EventTarget* created = g_createdTargets;

	for (int i = 0;; i++)
	{
		struct EventTarget* current;

		if (i < 3)
			current = targets[i];
		else if (created)
		{
			current = created;
			created = created->nextCreated;
		}
		else
			break;
		for (struct EventHandlerEntry* handler = current->handlers; handler; handler = handler->next)
		{
			if (handler == ref && !handler->removed)
				return handler;
		}
	}
	return NULL;
}

OSStatus AddEventTypesToHandler(EventHandlerRef inHandlerRef, ItemCount inNumTypes, const EventTypeSpec* inList)
{
	if (!inHandlerRef || (inNumTypes != 0 && !inList))
		return paramErr;

	pthread_mutex_lock(&g_lock);
	struct EventHandlerEntry* handler = findInstalledHandler(inHandlerRef);
	if (!handler)
	{
		pthread_mutex_unlock(&g_lock);
		return paramErr;
	}

	EventTypeSpec* types = realloc(handler->types, (handler->typeCount + inNumTypes + 1) * sizeof(EventTypeSpec));
	if (!types)
	{
		pthread_mutex_unlock(&g_lock);
		return memFullErr;
	}
	handler->types = types;
	for (ItemCount i = 0; i < inNumTypes; i++)
	{
		bool present = false;
		for (ItemCount j = 0; j < handler->typeCount && !present; j++)
			present = types[j].eventClass == inList[i].eventClass && types[j].eventKind == inList[i].eventKind;
		if (!present)
			types[handler->typeCount++] = inList[i];
	}
	pthread_mutex_unlock(&g_lock);
	return noErr;
}

OSStatus RemoveEventTypesFromHandler(EventHandlerRef inHandlerRef, ItemCount inNumTypes, const EventTypeSpec* inList)
{
	if (!inHandlerRef || (inNumTypes != 0 && !inList))
		return paramErr;

	pthread_mutex_lock(&g_lock);
	struct EventHandlerEntry* handler = findInstalledHandler(inHandlerRef);
	if (!handler)
	{
		pthread_mutex_unlock(&g_lock);
		return paramErr;
	}

	// Types the handler does not have are ignored
	for (ItemCount i = 0; i < inNumTypes; i++)
	{
		for (ItemCount j = 0; j < handler->typeCount; j++)
		{
			if (handler->types[j].eventClass == inList[i].eventClass && handler->types[j].eventKind == inList[i].eventKind)
			{
				handler->types[j] = handler->types[--handler->typeCount];
				break;
			}
		}
	}
	pthread_mutex_unlock(&g_lock);
	return noErr;
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
// the event; eventPassToNextTargetErr if one asked to go on with the next target. With
// allHandlers, every handler for the event runs, handled or not, and the answer is that of the
// first handler that did not answer eventNotHandledErr.
static OSStatus dispatchFrom(struct EventTarget* target, struct EventHandlerEntry* start, EventRef event, bool allHandlers)
{
	struct EventHandlerEntry* handler = start;
	OSStatus answer = eventNotHandledErr;

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
			return answer;

		struct HandlerCall call = { target, handler, event, 0 };
		OSStatus result = proc(&call, event, userData);

		if (allHandlers)
		{
			if (answer == eventNotHandledErr)
				answer = result;
			pthread_mutex_lock(&g_lock);
			handler = handler->next;
			pthread_mutex_unlock(&g_lock);
			continue;
		}

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
		while (g_destroyedTargets)
		{
			struct EventTarget* target = g_destroyedTargets;
			g_destroyedTargets = target->nextCreated;
			free(target);
		}
	}
	pthread_mutex_unlock(&g_lock);
}

static OSStatus sendEvent(EventRef inEvent, EventTargetRef inTarget, OptionBits inOptions)
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

		// QUIRK: the handlers of the application target see an event made from a CGEvent as the
		// current event (GetCurrentEvent()); those of the dispatcher target, and the handlers of
		// any event without a CGEvent, see none. Not documented. Evidence: black-box observation on
		// macOS 26.7.3 (tier 2, mac-probes/937-q-events.m for the dispatch of a Cocoa
		// application; the same observation for SendEventToEventTarget() is pinned in
		// tests/src/carbon_application_event_loop.c).
		// Test: test_quirk_current_event_at_application_target in tests/src/carbon_application_event_loop.c
		EventRef previousCurrent = NULL;
		bool setCurrent = target == &g_applicationTarget && inEvent->cgEvent != NULL;
		if (setCurrent)
			previousCurrent = _HISetCurrentEvent(inEvent);
		result = dispatchFrom(target, first, inEvent, (inOptions & kEventTargetSendToAllHandlers) != 0);
		if (setCurrent)
			_HISetCurrentEvent(previousCurrent);
		if (result != eventNotHandledErr && result != eventPassToNextTargetErr)
			break;
		if (inOptions & kEventTargetDontPropagate)
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
	OSStatus result = dispatchFrom(call->target, next, inEvent, false);

	// Leaving the target is up to the handler that called this: it returns the status
	return result;
}

OSStatus SendEventToEventTarget(EventRef inEvent, EventTargetRef inTarget)
{
	return sendEvent(inEvent, inTarget, 0);
}

OSStatus SendEventToEventTargetWithOptions(EventRef inEvent, EventTargetRef inTarget, OptionBits inOptions)
{
	return sendEvent(inEvent, inTarget, inOptions);
}
