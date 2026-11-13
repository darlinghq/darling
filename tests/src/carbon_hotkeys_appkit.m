// Copyright (C) 2026 Lubos Dolezel
//
// A global hot key reaches Carbon through the application's event queue: the window system posts a
// system-defined event (subtype 6 pressed, 9 released, data1 naming the hot key, window number 0),
// -[NSApplication sendEvent:] sends it to the event dispatcher target as a Carbon raw event made with
// CreateEventWithCGEvent, whose standard handler (HIToolbox) sends the hot key event, so the Carbon
// handler of the registered target runs inside -sendEvent:. An application that overrides -sendEvent: and does not call super gets no
// Carbon callback. Needs a display (X on Darling).
//
// The CGEvent conversion part also passes on macOS 26.7.3; the rest is Darling's (data1 is not
// verified to be the uid there).
//
// Synthetic part: the events are posted by the program itself. The uid that data1 carries is
// found by trying small values; exactly one of them reaches the handler.
//
// With HOTKEY_E2E=<seconds> the program then waits for a real press of Control+Shift+A (xdotool key
// ctrl+shift+a on the display of the session) after it prints READY: the application sees a
// system-defined event of subtype 6 then 9 with the same data1 as the synthetic part found, window
// number 0, and the Carbon handler runs for both. With HOTKEY_CGS=1 (Darling only) it then switches hot
// keys off with CGSSetGlobalHotKeyOperatingMode(1): after READY_OFF the key must not arrive; after READY_ON
// (mode 0) it must; after READY2 (the hot key unregistered) it must not.
#import <AppKit/AppKit.h>
#import <Carbon/Carbon.h>
#include <stdio.h>
#include <stdlib.h>

extern uint32_t CGSMainConnectionID(void);
extern int CGSSetGlobalHotKeyOperatingMode(uint32_t cid, int mode);

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static BOOL g_swallow, g_inSendEvent;
static int g_seenPressed, g_seenReleased;
static NSInteger g_lastData1 = -1;
static NSInteger g_lastWindow = -1;
static NSEventModifierFlags g_lastFlags;

static int g_pressed, g_released, g_handlerInSendEvent, g_handlerEventOK;
static EventHotKeyID g_lastID;

@interface HotKeyApp : NSApplication
@end

@implementation HotKeyApp
- (void)sendEvent:(NSEvent*)event
{
	if ([event type] == NSEventTypeSystemDefined && ([event subtype] == 6 || [event subtype] == 9))
	{
		if ([event subtype] == 6)
			g_seenPressed++;
		else
			g_seenReleased++;
		g_lastData1 = [event data1];
		g_lastWindow = [event windowNumber];
		g_lastFlags = [event modifierFlags];
		if (g_swallow)
			return;
		g_inSendEvent = YES;
		[super sendEvent:event];
		g_inSendEvent = NO;
		return;
	}
	[super sendEvent:event];
}
@end

static OSStatus handler(EventHandlerCallRef next, EventRef e, void* ud)
{
	EventHotKeyID id;

	if (GetEventParameter(e, kEventParamDirectObject, typeEventHotKeyID, NULL, sizeof(id), NULL, &id) != noErr)
		return eventNotHandledErr;
	g_lastID = id;
	if (g_inSendEvent)
		g_handlerInSendEvent++;
	NSEvent* current = [NSApp currentEvent];
	if ([current type] == NSEventTypeSystemDefined && [current data1] == g_lastData1)
		g_handlerEventOK++;
	if (GetEventKind(e) == kEventHotKeyPressed)
		g_pressed++;
	else if (GetEventKind(e) == kEventHotKeyReleased)
		g_released++;
	return noErr;
}

static void pump(double seconds, BOOL untilBoth)
{
	NSDate* end = [NSDate dateWithTimeIntervalSinceNow:seconds];
	while ([end timeIntervalSinceNow] > 0 && !(untilBoth && g_pressed > 0 && g_released > 0))
	{
		NSEvent* e = [NSApp nextEventMatchingMask:NSAnyEventMask untilDate:[NSDate dateWithTimeIntervalSinceNow:0.1]
			inMode:NSDefaultRunLoopMode dequeue:YES];
		if (e)
			[NSApp sendEvent:e];
	}
}

static void post(int subtype, NSInteger data1)
{
	NSEvent* e = [NSEvent otherEventWithType:NSEventTypeSystemDefined location:NSZeroPoint modifierFlags:0
		timestamp:0 windowNumber:0 context:nil subtype:subtype data1:data1 data2:0];
	[NSApp postEvent:e atStart:NO];
}

// Carbon events made from CoreGraphics events (observed on macOS 26.7.3: a system-defined
// event gives class 'cgs ' and kind 14 = its CGEventType, and the CGEvent comes back)
static void testCGEventConversion(void)
{
	NSEvent* e = [NSEvent otherEventWithType:NSEventTypeSystemDefined location:NSMakePoint(10, 20)
		modifierFlags:NSEventModifierFlagControl timestamp:5 windowNumber:0 context:nil subtype:6 data1:42 data2:0];
	CGEventRef cg = [e CGEvent];
	EventRef ref = NULL;

	CHECK(cg != NULL && CGEventGetType(cg) == 14);
	CHECK(CreateEventWithCGEvent(NULL, cg, kEventAttributeNone, &ref) == noErr && ref != NULL);
	// The class and kind are not in the SDK headers: 'cgs ' and the CGEventType, observed on macOS 26.7.3
	// (mac-probes/v443-systemdefined-carbon-event.m, MC-V443-APPKIT-IMPORTS)
	CHECK(GetEventClass(ref) == 'cgs ' && GetEventKind(ref) == 14);
	CHECK(GetEventRetainCount(ref) == 1);

	CGEventRef back = CopyEventCGEvent(ref);
	CHECK(back != NULL && CGEventGetType(back) == 14);
	CHECK(back != NULL && (CGEventGetFlags(back) & kCGEventFlagMaskControl));
	if (back)
		CFRelease(back);

	// No handler wants it: not handled
	CHECK(SendEventToEventTarget(ref, GetEventDispatcherTarget()) == eventNotHandledErr);
	ReleaseEvent(ref);
}

int main(void)
{
	@autoreleasepool {
		testCGEventConversion();
		[HotKeyApp sharedApplication];
		CHECK([NSApp isKindOfClass:[HotKeyApp class]]);

		EventTypeSpec specs[2] = { { kEventClassKeyboard, kEventHotKeyPressed }, { kEventClassKeyboard, kEventHotKeyReleased } };
		EventHandlerRef h = NULL;
		EventHotKeyRef ref = NULL;
		EventHotKeyID id = { 'apkt', 3 };

		CHECK(InstallEventHandler(GetApplicationEventTarget(), handler, 2, specs, NULL, &h) == noErr);
		CHECK(RegisterEventHotKey(kVK_ANSI_A, controlKey | shiftKey, id, GetApplicationEventTarget(), 0, &ref) == noErr);

		// The uid is whatever the registration got: find it
		NSInteger uid = -1;
		for (NSInteger candidate = 1; candidate <= 32 && uid < 0; candidate++)
		{
			post(6, candidate);
			pump(0.3, NO);
			if (g_pressed > 0)
				uid = candidate;
		}
		CHECK(uid > 0);
		CHECK(g_pressed == 1 && g_released == 0);
		CHECK(g_lastID.signature == 'apkt' && g_lastID.id == 3);
		// The Carbon callback runs inside -sendEvent:, for the system-defined event being sent
		CHECK(g_handlerInSendEvent == 1 && g_handlerEventOK == 1);
		CHECK(g_lastWindow == 0);

		post(9, uid);
		pump(0.5, NO);
		CHECK(g_released == 1 && g_handlerInSendEvent == 2);
		CHECK(g_lastID.id == 3);

		// Other system-defined subtypes are not hot keys
		int before = g_pressed;
		NSEvent* other = [NSEvent otherEventWithType:NSEventTypeSystemDefined location:NSZeroPoint modifierFlags:0
			timestamp:0 windowNumber:0 context:nil subtype:8 data1:uid data2:0];
		[NSApp postEvent:other atStart:NO];
		pump(0.3, NO);
		CHECK(g_pressed == before);

		// An application that does not call super gets no Carbon callback
		g_swallow = YES;
		int seen = g_seenPressed;
		post(6, uid);
		pump(0.5, NO);
		CHECK(g_seenPressed == seen + 1);
		CHECK(g_pressed == before);
		g_swallow = NO;

		// A hot key that is gone no longer reaches its handler
		CHECK(UnregisterEventHotKey(ref) == noErr);
		post(6, uid);
		pump(0.5, NO);
		CHECK(g_pressed == before);

		const char* e2e = getenv("HOTKEY_E2E");
		if (e2e)
		{
			double seconds = atof(e2e);
			g_pressed = g_released = 0;
			g_seenPressed = g_seenReleased = 0;
			CHECK(RegisterEventHotKey(kVK_ANSI_A, controlKey | shiftKey, id, GetApplicationEventTarget(), 0, &ref) == noErr);
			// The second registration got a new uid
			printf("READY\n");
			fflush(stdout);
			pump(seconds, YES);
			CHECK(g_seenPressed == 1 && g_seenReleased == 1);
			CHECK(g_pressed == 1 && g_released == 1);
			CHECK(g_lastWindow == 0);
			CHECK((g_lastFlags & NSEventModifierFlagControl) && (g_lastFlags & NSEventModifierFlagShift));
			CHECK(g_lastID.signature == 'apkt' && g_lastID.id == 3);
			NSInteger realData1 = g_lastData1;
			printf("data1 of a real hot key event: %ld\n", (long) realData1);

			if (getenv("HOTKEY_CGS"))
			{
				uint32_t cid = CGSMainConnectionID();
				CGSSetGlobalHotKeyOperatingMode(cid, 1);
				g_pressed = g_released = 0;
				printf("READY_OFF\n");
				fflush(stdout);
				pump(seconds / 3, NO);
				CHECK(g_pressed == 0 && g_released == 0);
				CGSSetGlobalHotKeyOperatingMode(cid, 0);
				printf("READY_ON\n");
				fflush(stdout);
				pump(seconds, YES);
				CHECK(g_pressed == 1 && g_released == 1);
			}

			CHECK(UnregisterEventHotKey(ref) == noErr);
			g_pressed = g_released = 0;
			printf("READY2\n");
			fflush(stdout);
			pump(seconds / 3, NO);
			CHECK(g_pressed == 0 && g_released == 0);
		}
		RemoveEventHandler(h);
	}
	printf(g_failures ? "FAIL: %d checks failed\n" : "PASS\n", g_failures);
	return g_failures != 0;
}
