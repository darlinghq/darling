// Copyright (C) 2026 Lubos Dolezel
//
// RunApplicationEventLoop in a process without windows: the events of the main queue are sent to
// the event dispatcher target and then the application target, the loop ends on
// QuitApplicationEventLoop or a quit command, and GetCurrentEvent during dispatch. Observed on
// macOS 26.7.3; the program passes unchanged there.
#include <Carbon/Carbon.h>
#include <stdio.h>

void RunApplicationEventLoop(void);
void QuitApplicationEventLoop(void);

static int failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL: %s (line %d)\n", #c, __LINE__); failures++; } } while (0)

enum { kClass = 'test' };

static int g_step;
static int g_order[16];
static OSType g_class[16];
static int g_orderCount;
static int g_currentIsEvent[16];
static int g_monitorCalls;

static void note(int who, EventRef event)
{
	if (g_orderCount < 16)
	{
		g_order[g_orderCount] = who * 100 + (int) GetEventKind(event);
		g_class[g_orderCount] = GetEventClass(event);
		g_currentIsEvent[g_orderCount] = GetCurrentEvent() == event;
		g_orderCount++;
	}
}

static OSStatus dispatcherHandler(EventHandlerCallRef call, EventRef event, void* data)
{
	note(1, event);
	return eventNotHandledErr;
}

static OSStatus applicationHandler(EventHandlerCallRef call, EventRef event, void* data)
{
	note(2, event);
	return eventNotHandledErr;
}

static OSStatus monitorHandler(EventHandlerCallRef call, EventRef event, void* data)
{
	g_monitorCalls++;
	return eventNotHandledErr;
}

static void post(EventRef event)
{
	CHECK(PostEventToQueue(GetMainEventQueue(), event, kEventPriorityStandard) == noErr);
	ReleaseEvent(event);
}

static EventRef plainEvent(UInt32 kind)
{
	EventRef event = NULL;
	CreateEvent(NULL, kClass, kind, 0, kEventAttributeNone, &event);
	return event;
}

static EventRef mouseEvent(void)
{
	CGEventRef cg = CGEventCreateMouseEvent(NULL, kCGEventLeftMouseDown, CGPointMake(5, 5), kCGMouseButtonLeft);
	EventRef event = NULL;
	CreateEventWithCGEvent(NULL, cg, kEventAttributeNone, &event);
	CFRelease(cg);
	return event;
}

static EventRef keyEvent(void)
{
	CGEventRef cg = CGEventCreateKeyboardEvent(NULL, 0, true);
	EventRef event = NULL;
	CreateEventWithCGEvent(NULL, cg, kEventAttributeNone, &event);
	CFRelease(cg);
	return event;
}

// Posts one event after the other, then asks the loop to end
static void step(EventLoopTimerRef timer, void* data)
{
	CHECK(GetCurrentEvent() == NULL);   // a timer is not an event
	switch (g_step++)
	{
		case 0: post(plainEvent(1)); break;
		case 1: post(mouseEvent()); break;
		case 2: post(keyEvent()); break;
		case 3:
		{
			HICommand command = { 0, kHICommandQuit };
			EventRef event = NULL;
			CreateEvent(NULL, kEventClassCommand, kEventCommandProcess, 0, kEventAttributeNone, &event);
			SetEventParameter(event, kEventParamDirectObject, typeHICommand, sizeof(command), &command);
			post(event);
			// The quit command is not handled by the application: the loop quits by itself
			SetEventLoopTimerNextFireTime(timer, 3.0);
			return;
		}
		default:
			printf("the quit command did not end the loop\n");
			QuitApplicationEventLoop();
			return;
	}
	SetEventLoopTimerNextFireTime(timer, 0.1);
}

static void quitAfterDelay(EventLoopTimerRef timer, void* data)
{
	QuitApplicationEventLoop();
}

static void testDispatch(void)
{
	EventTypeSpec types[] = { { kClass, 1 }, { kEventClassMouse, kEventMouseDown }, { kEventClassKeyboard, kEventRawKeyDown } };
	EventHandlerRef dispatcher, application, monitor;
	EventLoopTimerRef timer;

	CHECK(InstallEventHandler(GetEventDispatcherTarget(), dispatcherHandler, 3, types, NULL, &dispatcher) == noErr);
	CHECK(InstallEventHandler(GetApplicationEventTarget(), applicationHandler, 3, types, NULL, &application) == noErr);
	CHECK(InstallEventHandler(GetEventMonitorTarget(), monitorHandler, 3, types, NULL, &monitor) == noErr);
	CHECK(InstallEventLoopTimer(GetCurrentEventLoop(), 0.1, 0, step, NULL, &timer) == noErr);

	RunApplicationEventLoop();

	// Each event went to the dispatcher target, then the application target. The quit command
	// ended the loop.
	CHECK(g_step == 4);
	CHECK(g_orderCount == 6);
	CHECK(g_order[0] == 101 && g_order[1] == 201);
	CHECK(g_order[2] == 101 && g_order[3] == 201);
	CHECK(g_order[4] == 101 && g_order[5] == 201);
	CHECK(g_class[0] == kClass && g_class[1] == kClass);
	CHECK(g_class[2] == kEventClassMouse && g_class[3] == kEventClassMouse);
	CHECK(g_class[4] == kEventClassKeyboard && g_class[5] == kEventClassKeyboard);
	// The event monitor target sees nothing of it
	CHECK(g_monitorCalls == 0);

	// QUIRK: an event made from a CGEvent is the current event while the loop dispatches it
	// (to both targets); an event that has none is not. Not documented. Evidence: black-box
	// observation on macOS 26.7.3 (tier 2). Test: test_quirk_current_event_during_dispatch
	CHECK(!g_currentIsEvent[0] && !g_currentIsEvent[1]);
	CHECK(g_currentIsEvent[2] && g_currentIsEvent[3]);
	CHECK(g_currentIsEvent[4] && g_currentIsEvent[5]);
	CHECK(GetCurrentEvent() == NULL);

	CHECK(RemoveEventLoopTimer(timer) == noErr);
	CHECK(RemoveEventHandler(dispatcher) == noErr);
	CHECK(RemoveEventHandler(application) == noErr);
	CHECK(RemoveEventHandler(monitor) == noErr);
	FlushEventQueue(GetMainEventQueue());
}

static int g_sentCurrent[2];

static OSStatus sentDispatcher(EventHandlerCallRef call, EventRef event, void* data)
{
	g_sentCurrent[0] = GetCurrentEvent() == event;
	return eventNotHandledErr;
}

static OSStatus sentApplication(EventHandlerCallRef call, EventRef event, void* data)
{
	g_sentCurrent[1] = GetCurrentEvent() == event;
	return eventNotHandledErr;
}

// test_quirk_current_event_at_application_target
static void testSentEvent(void)
{
	EventTypeSpec mouseDown = { kEventClassMouse, kEventMouseDown };
	EventHandlerRef dispatcher, application;
	EventRef event = mouseEvent();

	CHECK(InstallEventHandler(GetEventDispatcherTarget(), sentDispatcher, 1, &mouseDown, NULL, &dispatcher) == noErr);
	CHECK(InstallEventHandler(GetApplicationEventTarget(), sentApplication, 1, &mouseDown, NULL, &application) == noErr);

	// QUIRK: SendEventToEventTarget() of an event made from a CGEvent: the application target's
	// handlers see it as the current event, the dispatcher target's do not. Not documented.
	// Evidence: black-box observation on macOS 26.7.3 (tier 2).
	CHECK(SendEventToEventTarget(event, GetEventDispatcherTarget()) == eventNotHandledErr);
	CHECK(g_sentCurrent[0] == 0);
	CHECK(g_sentCurrent[1] == 1);
	CHECK(GetCurrentEvent() == NULL);

	CHECK(RemoveEventHandler(dispatcher) == noErr);
	CHECK(RemoveEventHandler(application) == noErr);
	ReleaseEvent(event);
}

static void testQuit(void)
{
	EventLoopTimerRef timer;

	// QuitApplicationEventLoop ends the loop
	CHECK(InstallEventLoopTimer(GetCurrentEventLoop(), 0.1, 0, quitAfterDelay, NULL, &timer) == noErr);
	RunApplicationEventLoop();
	CHECK(RemoveEventLoopTimer(timer) == noErr);

	// With no loop running, it is a quit of the main event loop, which is not forgotten
	QuitApplicationEventLoop();
	CHECK(RunCurrentEventLoop(0.05) == eventLoopQuitErr);
	CHECK(RunCurrentEventLoop(0.05) == eventLoopTimedOutErr);

	// A quit command that the application handles first is the application's own business
	EventTypeSpec commands = { kEventClassCommand, kEventCommandProcess };
	EventHandlerRef handler;
	g_monitorCalls = 0;
	CHECK(InstallEventHandler(GetApplicationEventTarget(), monitorHandler, 1, &commands, NULL, &handler) == noErr);
	g_step = 3;
	CHECK(InstallEventLoopTimer(GetCurrentEventLoop(), 0.1, 0, step, NULL, &timer) == noErr);
	RunApplicationEventLoop();
	CHECK(g_monitorCalls == 1);   // the application handler saw the command, did not handle it, and the loop quit
	CHECK(RemoveEventLoopTimer(timer) == noErr);
	CHECK(RemoveEventHandler(handler) == noErr);
}

int main(void)
{
	testSentEvent();
	testDispatch();
	testQuit();
	printf(failures ? "FAILED\n" : "PASS\n");
	return failures != 0;
}
