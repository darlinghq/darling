// Copyright (C) 2026 Lubos Dolezel
//
// Carbon event functions: CopyEvent, SetEventTime, GetCurrentEvent, GetCurrentEventQueue,
// AddEventTypesToHandler, RemoveEventTypesFromHandler, GetEventMonitorTarget,
// SendEventToEventTargetWithOptions and AEProcessEvent. Observed on macOS 26.7.3; the program
// passes unchanged there.
#include <Carbon/Carbon.h>
#include <stdio.h>

static int failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL: %s (line %d)\n", #c, __LINE__); failures++; } } while (0)

static int g_first, g_second;

static OSStatus firstHandler(EventHandlerCallRef call, EventRef event, void *userData)
{
	g_first++;
	return noErr;
}

static OSStatus secondHandler(EventHandlerCallRef call, EventRef event, void *userData)
{
	g_second++;
	return noErr;
}

// Not handled: the event goes on to the next handler
static OSStatus passHandler(EventHandlerCallRef call, EventRef event, void *userData)
{
	g_first++;
	return eventNotHandledErr;
}

enum { kClass = 'test', kKindA = 7, kKindB = 8 };

int main(void)
{
	// The event queues: the main thread's current queue is the main queue
	CHECK(GetMainEventQueue() != NULL);
	CHECK(GetCurrentEventQueue() == GetMainEventQueue());
	CHECK(GetCurrentEvent() == NULL);

	EventRef event = NULL;
	CHECK(CreateEvent(NULL, kClass, kKindA, 0, kEventAttributeNone, &event) == noErr);
	int value = 5;
	CHECK(SetEventParameter(event, 'abcd', 'long', sizeof(value), &value) == noErr);

	// CopyEvent: a separate event with the same class, kind, time and parameters
	EventRef copy = CopyEvent(event);
	CHECK(copy != NULL && copy != event);
	CHECK(GetEventClass(copy) == kClass);
	CHECK(GetEventKind(copy) == kKindA);
	CHECK(GetEventTime(copy) == GetEventTime(event));
	CHECK(GetEventRetainCount(copy) == 1);
	int copied = 0;
	CHECK(GetEventParameter(copy, 'abcd', 'long', NULL, sizeof(copied), NULL, &copied) == noErr);
	CHECK(copied == 5);
	CHECK(CopyEvent(NULL) == NULL);

	// SetEventTime changes only the event it is given
	CHECK(SetEventTime(event, 123.5) == noErr);
	CHECK(GetEventTime(event) == 123.5);
	CHECK(GetEventTime(copy) != 123.5);
	CHECK(SetEventTime(NULL, 1.0) == paramErr);
	ReleaseEvent(copy);

	// Event types of an installed handler can be added and removed
	EventTypeSpec kindA = { kClass, kKindA }, kindB = { kClass, kKindB };
	EventRef eventB = NULL;
	CHECK(CreateEvent(NULL, kClass, kKindB, 0, kEventAttributeNone, &eventB) == noErr);
	EventHandlerRef handler = NULL;
	CHECK(InstallEventHandler(GetApplicationEventTarget(), firstHandler, 1, &kindA, NULL, &handler) == noErr);
	CHECK(SendEventToEventTarget(eventB, GetApplicationEventTarget()) == eventNotHandledErr);
	CHECK(g_first == 0);
	CHECK(AddEventTypesToHandler(handler, 1, &kindB) == noErr);
	CHECK(SendEventToEventTarget(eventB, GetApplicationEventTarget()) == noErr);
	CHECK(g_first == 1);
	CHECK(RemoveEventTypesFromHandler(handler, 1, &kindB) == noErr);
	CHECK(SendEventToEventTarget(eventB, GetApplicationEventTarget()) == eventNotHandledErr);
	CHECK(g_first == 1);
	CHECK(SendEventToEventTarget(event, GetApplicationEventTarget()) == noErr);
	CHECK(g_first == 2);
	// Removing a type the handler has is accepted
	CHECK(RemoveEventTypesFromHandler(handler, 1, &kindB) == noErr);
	CHECK(AddEventTypesToHandler(NULL, 1, &kindA) == paramErr);
	CHECK(RemoveEventTypesFromHandler(NULL, 1, &kindA) == paramErr);

	// The options of SendEventToEventTargetWithOptions: none is a plain send, and the
	// event monitor target exists
	CHECK(GetEventMonitorTarget() != NULL);
	g_first = g_second = 0;
	EventHandlerRef second = NULL;
	CHECK(InstallEventHandler(GetApplicationEventTarget(), secondHandler, 1, &kindA, NULL, &second) == noErr);
	CHECK(SendEventToEventTargetWithOptions(event, GetApplicationEventTarget(), 0) == noErr);
	CHECK(g_second == 1 && g_first == 0);   // the newest handler handled it
	CHECK(SendEventToEventTargetWithOptions(event, GetApplicationEventTarget(), kEventTargetSendToAllHandlers) == noErr);
	CHECK(g_second == 2 && g_first == 1);
	CHECK(RemoveEventHandler(second) == noErr);
	CHECK(RemoveEventHandler(handler) == noErr);

	// AEProcessEvent: only Apple event class events are accepted, and they need their class and ID
	CHECK(AEProcessEvent(NULL) == paramErr);
	CHECK(AEProcessEvent(event) == paramErr);
	EventRef appleEvent = NULL;
	CHECK(CreateEvent(NULL, kEventClassAppleEvent, kEventAppleEvent, 0, kEventAttributeNone, &appleEvent) == noErr);
	CHECK(AEProcessEvent(appleEvent) == eventParameterNotFoundErr);

	ReleaseEvent(appleEvent);
	ReleaseEvent(eventB);
	ReleaseEvent(event);
	printf(failures ? "FAILED\n" : "PASS\n");
	return failures != 0;
}
