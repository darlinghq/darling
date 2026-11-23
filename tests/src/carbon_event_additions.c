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

static EventRef fromCGEvent(CGEventRef cg)
{
	EventRef event = NULL;

	CHECK(CreateEventWithCGEvent(NULL, cg, kEventAttributeNone, &event) == noErr);
	CHECK(event != NULL);
	// The event keeps the CGEvent it was made from
	CGEventRef back = event ? CopyEventCGEvent(event) : NULL;
	CHECK(back == cg);
	if (back)
		CFRelease(back);
	return event;
}

static void expectClassAndKind(const char* what, CGEventRef cg, OSType eventClass, UInt32 kind)
{
	EventRef event = fromCGEvent(cg);

	if (event && (GetEventClass(event) != eventClass || GetEventKind(event) != kind))
		printf("FAIL: %s is %08x/%u\n", what, (unsigned) GetEventClass(event), (unsigned) GetEventKind(event));
	CHECK(event && GetEventClass(event) == eventClass && GetEventKind(event) == kind);
	if (event)
		ReleaseEvent(event);
}

// QUIRK (test_quirk_cgevent_classes): CreateEventWithCGEvent gives the mouse and keyboard classes
// for input events, and the raw class 'cgs ' with the CGEvent type as the kind for the others.
// Observed on macOS 26.7.3.
static void test_quirk_cgevent_classes(void)
{
	CGPoint where = CGPointMake(10, 20);
	CGEventRef cg;

	cg = CGEventCreateMouseEvent(NULL, kCGEventLeftMouseDown, where, kCGMouseButtonLeft);
	expectClassAndKind("left mouse down", cg, 'mous', 1);
	CGEventSetIntegerValueField(cg, kCGMouseEventClickState, 2);
	CGEventSetFlags(cg, kCGEventFlagMaskShift);

	// The mouse events have their location, click count, button and modifiers
	EventRef mouse = fromCGEvent(cg);
	HIPoint point = { 0, 0 };
	UInt32 clicks = 0, modifiers = 99;
	EventMouseButton button = 0;
	EventParamType type = 0;
	CHECK(GetEventParameter(mouse, 'mloc', 'hipt', &type, sizeof(point), NULL, &point) == noErr);
	CHECK(point.x == 10 && point.y == 20);
	CHECK(GetEventParameter(mouse, 'ccnt', 'magn', NULL, sizeof(clicks), NULL, &clicks) == noErr);
	CHECK(clicks == 2);
	CHECK(GetEventParameter(mouse, 'mbtn', 'mbtn', NULL, sizeof(button), NULL, &button) == noErr);
	CHECK(button == 1);
	CHECK(GetEventParameter(mouse, 'kmod', 'magn', NULL, sizeof(modifiers), NULL, &modifiers) == noErr);
	CHECK(modifiers == 0x200);   // shiftKey
	// A parameter the application sets is the one it gets
	UInt32 two = 77;
	CHECK(SetEventParameter(mouse, 'ccnt', 'magn', sizeof(two), &two) == noErr);
	CHECK(GetEventParameter(mouse, 'ccnt', 'magn', NULL, sizeof(clicks), NULL, &clicks) == noErr);
	CHECK(clicks == 77);
	ReleaseEvent(mouse);
	CFRelease(cg);

	cg = CGEventCreateMouseEvent(NULL, kCGEventMouseMoved, where, kCGMouseButtonLeft);
	expectClassAndKind("mouse moved", cg, 'mous', 5);
	CFRelease(cg);
	cg = CGEventCreateMouseEvent(NULL, kCGEventLeftMouseUp, where, kCGMouseButtonLeft);
	expectClassAndKind("left mouse up", cg, 'mous', 2);
	CFRelease(cg);
	cg = CGEventCreateMouseEvent(NULL, kCGEventLeftMouseDragged, where, kCGMouseButtonLeft);
	expectClassAndKind("left mouse dragged", cg, 'mous', 6);
	CFRelease(cg);
	cg = CGEventCreateMouseEvent(NULL, kCGEventRightMouseDown, where, kCGMouseButtonRight);
	expectClassAndKind("right mouse down", cg, 'mous', 1);
	mouse = fromCGEvent(cg);
	CHECK(GetEventParameter(mouse, 'mbtn', 'mbtn', NULL, sizeof(button), NULL, &button) == noErr);
	CHECK(button == 2);
	ReleaseEvent(mouse);
	CFRelease(cg);
	cg = CGEventCreateMouseEvent(NULL, kCGEventOtherMouseDown, where, kCGMouseButtonCenter);
	CGEventSetIntegerValueField(cg, kCGMouseEventButtonNumber, 2);
	expectClassAndKind("other mouse down", cg, 'mous', 1);
	mouse = fromCGEvent(cg);
	CHECK(GetEventParameter(mouse, 'mbtn', 'mbtn', NULL, sizeof(button), NULL, &button) == noErr);
	CHECK(button == 3);
	ReleaseEvent(mouse);
	CFRelease(cg);
	cg = CGEventCreateScrollWheelEvent(NULL, kCGScrollEventUnitLine, 1, 1);
	expectClassAndKind("scroll wheel", cg, 'mous', 10);
	CFRelease(cg);

	// The keyboard events have the key code and the modifiers, and no Unicode text
	cg = CGEventCreateKeyboardEvent(NULL, 0, true);
	CGEventSetIntegerValueField(cg, kCGKeyboardEventKeycode, 5);
	CGEventSetFlags(cg, kCGEventFlagMaskCommand);
	expectClassAndKind("key down", cg, 'keyb', 1);
	EventRef key = fromCGEvent(cg);
	UInt32 code = 0;
	UniChar text[4];
	CHECK(GetEventParameter(key, 'kcod', 'magn', NULL, sizeof(code), NULL, &code) == noErr);
	CHECK(code == 5);
	CHECK(GetEventParameter(key, 'kmod', 'magn', NULL, sizeof(modifiers), NULL, &modifiers) == noErr);
	CHECK(modifiers == 0x100);   // cmdKey
	CHECK(GetEventParameter(key, 'kuni', 'utxt', NULL, sizeof(text), NULL, text) == eventParameterNotFoundErr);
	// The mouse parameters are not on a key event
	CHECK(GetEventParameter(key, 'mloc', 'hipt', NULL, sizeof(point), NULL, &point) == eventParameterNotFoundErr);
	ReleaseEvent(key);
	CGEventSetType(cg, kCGEventFlagsChanged);
	expectClassAndKind("flags changed", cg, 'keyb', 4);
	CFRelease(cg);

	// Types with no Carbon class of their own: the raw class, the CGEvent type as the kind
	cg = CGEventCreate(NULL);
	CGEventSetType(cg, (CGEventType) 14);
	expectClassAndKind("system defined", cg, 'cgs ', 14);
	CGEventSetType(cg, (CGEventType) 15);
	expectClassAndKind("application defined", cg, 'cgs ', 15);
	CFRelease(cg);
}

int main(void)
{
	test_quirk_cgevent_classes();

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
