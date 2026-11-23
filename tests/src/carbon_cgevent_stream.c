// Copyright (C) 2026 Lubos Dolezel
//
// A pure Carbon process (no AppKit): the events of the process's CoreGraphics event stream reach
// the Carbon event loop. A thread that waits in ReceiveNextEvent, RunCurrentEventLoop or
// RunApplicationEventLoop reads what CGEventPost() posted, from any thread, after the event taps
// have seen it, and finds it in the main queue as a Carbon event of the right class, kind and
// parameters; mouse moves coalesce; the handlers of the application target run in
// RunApplicationEventLoop. Also, HIToolbox sits on CoreGraphics and does not depend on AppKit.
//
// Darling's behaviour: on macOS the posting of events needs the accessibility permission, and an
// event that is only posted goes nowhere. The program is for Darling.
#include <Carbon/Carbon.h>
#include <ApplicationServices/ApplicationServices.h>
#include <dlfcn.h>
#include <mach-o/dyld.h>
#include <mach-o/loader.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void RunApplicationEventLoop(void);
void QuitApplicationEventLoop(void);
OSStatus SetMouseCoalescingEnabled(Boolean inNewState, Boolean* outOldState);

static int failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL: %s (line %d)\n", #c, __LINE__); failures++; } } while (0)

static void post(CGEventRef event)
{
	CGEventPost(kCGSessionEventTap, event);
	CFRelease(event);
}

static CGEventRef mouse(CGEventType type, double x, double y, CGMouseButton button)
{
	return CGEventCreateMouseEvent(NULL, type, CGPointMake(x, y), button);
}

static EventRef next(double timeout)
{
	EventRef event = NULL;
	OSStatus status = ReceiveNextEvent(0, NULL, timeout, true, &event);

	return status == noErr ? event : NULL;
}

static void drain(void)
{
	EventRef event;

	while ((event = next(0.0)) != NULL)
		ReleaseEvent(event);
}

// The events one after the other: class, kind and parameters
static void testReceive(void)
{
	// The first wait registers the thread as the reader of the stream
	drain();

	CGEventRef down = mouse(kCGEventLeftMouseDown, 10, 20, kCGMouseButtonLeft);
	CGEventSetIntegerValueField(down, kCGMouseEventClickState, 2);
	CGEventSetFlags(down, kCGEventFlagMaskShift);
	post(down);

	EventRef event = next(2.0);
	CHECK(event != NULL);
	if (event)
	{
		HIPoint point = { 0, 0 };
		UInt32 clicks = 0, modifiers = 0;
		EventMouseButton button = 0;

		CHECK(GetEventClass(event) == kEventClassMouse && GetEventKind(event) == kEventMouseDown);
		CHECK(GetEventParameter(event, kEventParamMouseLocation, typeHIPoint, NULL, sizeof(point), NULL, &point) == noErr);
		CHECK(point.x == 10 && point.y == 20);
		CHECK(GetEventParameter(event, kEventParamClickCount, typeUInt32, NULL, sizeof(clicks), NULL, &clicks) == noErr && clicks == 2);
		CHECK(GetEventParameter(event, kEventParamMouseButton, typeMouseButton, NULL, sizeof(button), NULL, &button) == noErr && button == kEventMouseButtonPrimary);
		CHECK(GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(modifiers), NULL, &modifiers) == noErr && modifiers == shiftKey);
		CGEventRef cg = CopyEventCGEvent(event);
		CHECK(cg != NULL && CGEventGetType(cg) == kCGEventLeftMouseDown);
		if (cg)
			CFRelease(cg);
		ReleaseEvent(event);
	}

	CGEventRef up = mouse(kCGEventRightMouseUp, 11, 21, kCGMouseButtonRight);
	post(up);
	event = next(2.0);
	CHECK(event != NULL && GetEventClass(event) == kEventClassMouse && GetEventKind(event) == kEventMouseUp);
	if (event)
	{
		EventMouseButton button = 0;

		CHECK(GetEventParameter(event, kEventParamMouseButton, typeMouseButton, NULL, sizeof(button), NULL, &button) == noErr && button == kEventMouseButtonSecondary);
		ReleaseEvent(event);
	}

	CGEventRef key = CGEventCreateKeyboardEvent(NULL, 0x0b, true);
	CGEventSetFlags(key, kCGEventFlagMaskCommand);
	post(key);
	event = next(2.0);
	CHECK(event != NULL && GetEventClass(event) == kEventClassKeyboard && GetEventKind(event) == kEventRawKeyDown);
	if (event)
	{
		UInt32 code = 0, modifiers = 0;

		CHECK(GetEventParameter(event, kEventParamKeyCode, typeUInt32, NULL, sizeof(code), NULL, &code) == noErr && code == 0x0b);
		CHECK(GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(modifiers), NULL, &modifiers) == noErr && modifiers == cmdKey);
		ReleaseEvent(event);
	}

	post(CGEventCreateKeyboardEvent(NULL, 0x0b, false));
	event = next(2.0);
	CHECK(event != NULL && GetEventClass(event) == kEventClassKeyboard && GetEventKind(event) == kEventRawKeyUp);
	if (event)
		ReleaseEvent(event);

	// The kinds of the other event types
	post(mouse(kCGEventMouseMoved, 1, 2, kCGMouseButtonLeft));
	event = next(2.0);
	CHECK(event != NULL && GetEventClass(event) == kEventClassMouse && GetEventKind(event) == kEventMouseMoved);
	if (event)
		ReleaseEvent(event);
	post(mouse(kCGEventLeftMouseDragged, 1, 2, kCGMouseButtonLeft));
	event = next(2.0);
	CHECK(event != NULL && GetEventClass(event) == kEventClassMouse && GetEventKind(event) == kEventMouseDragged);
	if (event)
		ReleaseEvent(event);
	post(CGEventCreateScrollWheelEvent(NULL, kCGScrollEventUnitLine, 1, 3));
	event = next(2.0);
	CHECK(event != NULL && GetEventClass(event) == kEventClassMouse && GetEventKind(event) == kEventMouseWheelMoved);
	if (event)
		ReleaseEvent(event);

	CHECK(next(0.0) == NULL);
}

// Events leave in the order they were posted
static void testOrder(void)
{
	post(CGEventCreateKeyboardEvent(NULL, 1, true));
	post(mouse(kCGEventLeftMouseDown, 5, 5, kCGMouseButtonLeft));
	post(CGEventCreateKeyboardEvent(NULL, 1, false));
	post(mouse(kCGEventLeftMouseUp, 5, 5, kCGMouseButtonLeft));

	UInt32 expected[4][2] = {
		{ kEventClassKeyboard, kEventRawKeyDown }, { kEventClassMouse, kEventMouseDown },
		{ kEventClassKeyboard, kEventRawKeyUp }, { kEventClassMouse, kEventMouseUp },
	};
	for (int i = 0; i < 4; i++)
	{
		EventRef event = next(2.0);

		CHECK(event != NULL);
		if (!event)
			break;
		CHECK(GetEventClass(event) == expected[i][0] && GetEventKind(event) == expected[i][1]);
		ReleaseEvent(event);
	}
	CHECK(next(0.0) == NULL);
}

// Mouse moves coalesce unless that is switched off
static void testCoalescing(void)
{
	Boolean old = false;

	for (int i = 1; i <= 5; i++)
		post(mouse(kCGEventMouseMoved, i, 0, kCGMouseButtonLeft));
	post(CGEventCreateKeyboardEvent(NULL, 2, true));
	EventRef event = next(2.0);
	CHECK(event != NULL && GetEventKind(event) == kEventMouseMoved);
	if (event)
	{
		HIPoint point = { 0, 0 };

		CHECK(GetEventParameter(event, kEventParamMouseLocation, typeHIPoint, NULL, sizeof(point), NULL, &point) == noErr && point.x == 5);
		ReleaseEvent(event);
	}
	event = next(2.0);
	CHECK(event != NULL && GetEventClass(event) == kEventClassKeyboard);
	if (event)
		ReleaseEvent(event);
	CHECK(next(0.0) == NULL);

	CHECK(SetMouseCoalescingEnabled(false, &old) == noErr && old);
	for (int i = 1; i <= 5; i++)
		post(mouse(kCGEventMouseMoved, i, 0, kCGMouseButtonLeft));
	int count = 0;
	while ((event = next(count == 0 ? 2.0 : 0.0)) != NULL)
	{
		count++;
		ReleaseEvent(event);
	}
	CHECK(count == 5);
	SetMouseCoalescingEnabled(old, NULL);
}

// A post from another thread ends the wait of the main thread
static void* poster(void* arg)
{
	usleep(300000);
	post(CGEventCreateKeyboardEvent(NULL, 7, true));
	return NULL;
}

static double now(void)
{
	return CFAbsoluteTimeGetCurrent();
}

static void testOtherThread(void)
{
	pthread_t thread;
	double start = now();

	CHECK(pthread_create(&thread, NULL, poster, NULL) == 0);
	EventRef event = next(10.0);
	double waited = now() - start;
	pthread_join(thread, NULL);
	CHECK(event != NULL && GetEventClass(event) == kEventClassKeyboard);
	CHECK(waited > 0.2 && waited < 3.0);
	if (event)
		ReleaseEvent(event);

	// RunCurrentEventLoop does not take the events, but it reads the stream: the event is in the queue
	pthread_create(&thread, NULL, poster, NULL);
	OSStatus status = RunCurrentEventLoop(1.0);
	pthread_join(thread, NULL);
	CHECK(status == eventLoopTimedOutErr);
	CHECK(GetNumEventsInQueue(GetMainEventQueue()) == 1);
	drain();
}

// The event taps see the event before Carbon does, and decide what Carbon gets
static CGEventRef tapCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void* info)
{
	if (type == kCGEventLeftMouseDown || type == kCGEventLeftMouseUp)
	{
		CGPoint location = CGEventGetLocation(event);

		if (location.x == 99)
			return NULL;
		CGEventSetLocation(event, CGPointMake(location.x + 1000, location.y));
	}
	return event;
}

static void testTap(void)
{
	CFMachPortRef port = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, kCGEventTapOptionDefault,
		CGEventMaskBit(kCGEventLeftMouseDown) | CGEventMaskBit(kCGEventLeftMouseUp), tapCallback, NULL);

	CHECK(port != NULL);
	if (!port)
		return;
	CFRunLoopSourceRef source = CFMachPortCreateRunLoopSource(NULL, port, 0);
	CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopCommonModes);
	CGEventTapEnable(port, true);

	post(mouse(kCGEventLeftMouseDown, 99, 0, kCGMouseButtonLeft));   // dropped by the tap
	post(mouse(kCGEventLeftMouseDown, 5, 6, kCGMouseButtonLeft));    // changed by the tap
	EventRef event = next(2.0);
	CHECK(event != NULL);
	if (event)
	{
		HIPoint point = { 0, 0 };

		CHECK(GetEventParameter(event, kEventParamMouseLocation, typeHIPoint, NULL, sizeof(point), NULL, &point) == noErr);
		CHECK(point.x == 1005 && point.y == 6);
		ReleaseEvent(event);
	}
	CHECK(next(0.2) == NULL);

	CGEventTapEnable(port, false);
	post(mouse(kCGEventLeftMouseDown, 5, 6, kCGMouseButtonLeft));
	event = next(2.0);
	CHECK(event != NULL);
	if (event)
	{
		HIPoint point = { 0, 0 };

		GetEventParameter(event, kEventParamMouseLocation, typeHIPoint, NULL, sizeof(point), NULL, &point);
		CHECK(point.x == 5);
		ReleaseEvent(event);
	}

	CFRunLoopRemoveSource(CFRunLoopGetCurrent(), source, kCFRunLoopCommonModes);
	CFRelease(source);
	CFMachPortInvalidate(port);
	CFRelease(port);
}

// RunApplicationEventLoop dispatches them to the handlers of the application target
static int g_keys, g_mice, g_fromThread;
static pthread_t g_thread;

static OSStatus appHandler(EventHandlerCallRef call, EventRef event, void* data)
{
	if (GetEventClass(event) == kEventClassKeyboard)
	{
		UInt32 code = 0;

		GetEventParameter(event, kEventParamKeyCode, typeUInt32, NULL, sizeof(code), NULL, &code);
		g_keys++;
		if (code == 9)
			g_fromThread++;
	}
	else
		g_mice++;
	if (g_keys == 2 && g_mice == 1 && g_fromThread == 1)
		QuitApplicationEventLoop();
	return noErr;
}

static void* appPoster(void* arg)
{
	usleep(200000);
	post(CGEventCreateKeyboardEvent(NULL, 9, true));
	return NULL;
}

static void startPosting(EventLoopTimerRef timer, void* data)
{
	post(CGEventCreateKeyboardEvent(NULL, 3, true));
	post(mouse(kCGEventLeftMouseDown, 1, 1, kCGMouseButtonLeft));
	pthread_create(&g_thread, NULL, appPoster, NULL);
}

static void giveUp(EventLoopTimerRef timer, void* data)
{
	QuitApplicationEventLoop();
}

static void testApplicationLoop(void)
{
	EventTypeSpec specs[3] = {
		{ kEventClassKeyboard, kEventRawKeyDown }, { kEventClassMouse, kEventMouseDown }, { kEventClassMouse, kEventMouseUp },
	};
	EventHandlerRef handler = NULL;
	EventLoopTimerRef timer = NULL, guard = NULL;

	CHECK(InstallEventHandler(GetApplicationEventTarget(), appHandler, 3, specs, NULL, &handler) == noErr);
	InstallEventLoopTimer(GetMainEventLoop(), 0.1, 0, startPosting, NULL, &timer);
	InstallEventLoopTimer(GetMainEventLoop(), 8.0, 0, giveUp, NULL, &guard);
	RunApplicationEventLoop();
	pthread_join(g_thread, NULL);
	CHECK(g_keys == 2 && g_mice == 1 && g_fromThread == 1);
	RemoveEventLoopTimer(timer);
	RemoveEventLoopTimer(guard);
	RemoveEventHandler(handler);
	drain();
}

// Which libraries a loaded image names
static int imageDependsOn(const char* imageSubstring, const char* dependencySubstring)
{
	for (uint32_t i = 0; i < _dyld_image_count(); i++)
	{
		const char* name = _dyld_get_image_name(i);

		if (!name || !strstr(name, imageSubstring))
			continue;

		const struct mach_header* header = _dyld_get_image_header(i);
		const struct load_command* command = (const struct load_command*) ((const char*) header +
			(header->magic == MH_MAGIC_64 ? sizeof(struct mach_header_64) : sizeof(struct mach_header)));
		for (uint32_t c = 0; c < header->ncmds; c++)
		{
			if (command->cmd == LC_LOAD_DYLIB || command->cmd == LC_LOAD_WEAK_DYLIB || command->cmd == LC_REEXPORT_DYLIB)
			{
				const struct dylib_command* dylib = (const struct dylib_command*) command;
				const char* path = (const char*) dylib + dylib->dylib.name.offset;

				if (strstr(path, dependencySubstring))
					return 1;
			}
			command = (const struct load_command*) ((const char*) command + command->cmdsize);
		}
		return 0;
	}
	return -1;
}

static void testLayering(void)
{
	int loaded = 0;

	for (uint32_t i = 0; i < _dyld_image_count(); i++)
	{
		const char* name = _dyld_get_image_name(i);

		if (name && strstr(name, "AppKit.framework"))
			loaded = 1;
	}
	CHECK(!loaded);

	// HIToolbox is built on CoreGraphics, which does not link it back, and neither links AppKit
	CHECK(imageDependsOn("HIToolbox.framework", "CoreGraphics.framework") == 1);
	CHECK(imageDependsOn("HIToolbox.framework", "AppKit.framework") == 0);
	CHECK(imageDependsOn("CoreGraphics.framework", "HIToolbox.framework") == 0);
	CHECK(imageDependsOn("CoreGraphics.framework", "Carbon.framework") == 0);
	CHECK(imageDependsOn("CoreGraphics.framework", "AppKit.framework") == 0);
}

int main(void)
{
	testReceive();
	testOrder();
	testCoalescing();
	testOtherThread();
	testTap();
	testApplicationLoop();
	testLayering();
	printf(failures ? "FAIL: %d checks failed\n" : "PASS\n", failures);
	return failures != 0;
}
