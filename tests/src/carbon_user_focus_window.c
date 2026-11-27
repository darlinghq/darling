// Copyright (C) 2026 Lubos Dolezel
//
// The user focus of the Carbon Event Manager: the window that gets the keyboard events. By default it is the active
// window (kUserFocusAuto); SetUserFocusWindow() gives it to another window, GetUserFocusWindow() and
// GetUserFocusEventTarget() tell where it is, and a window that is disposed of gives it back to the active window.
// A key event sent to the event dispatcher goes to the window of the focus. (Carbon Event Manager Reference,
// "Keyboard focus".) Needs a window system (X11 or Wayland).
#include <Carbon/Carbon.h>
#include <stdio.h>

#ifdef __LP64__
#define COMP kWindowCompositingAttribute
#else
#define COMP 0
#endif

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void quitTimer(EventLoopTimerRef t, void *u) { QuitApplicationEventLoop(); }

static void spin(int ms)
{
	EventLoopTimerRef timer;
	InstallEventLoopTimer(GetMainEventLoop(), ms / 1000.0, 0, NewEventLoopTimerUPP(quitTimer), NULL, &timer);
	RunApplicationEventLoop();
	RemoveEventLoopTimer(timer);
}

static WindowRef g_receiver;

static OSStatus keyHandler(EventHandlerCallRef call, EventRef event, void *userData)
{
	g_receiver = (WindowRef) userData;
	return noErr;
}

// Sends a key down to the event dispatcher and tells which window got it
static WindowRef sendKey(void)
{
	EventRef event;
	UInt32 keyCode = 0;
	g_receiver = NULL;
	CHECK(CreateEvent(NULL, kEventClassKeyboard, kEventRawKeyDown, 0, kEventAttributeNone, &event) == noErr);
	SetEventParameter(event, kEventParamKeyCode, typeUInt32, sizeof(keyCode), &keyCode);
	SendEventToEventTarget(event, GetEventDispatcherTarget());
	ReleaseEvent(event);
	return g_receiver;
}

int main(void)
{
	static const EventTypeSpec keys[] = { { kEventClassKeyboard, kEventRawKeyDown } };
	Rect first = { 100, 100, 200, 300 }, second = { 100, 400, 200, 600 };
	WindowRef a = NULL, b = NULL;
	CHECK(CreateNewWindow(kDocumentWindowClass, kWindowStandardDocumentAttributes | COMP, &first, &a) == noErr);
	CHECK(CreateNewWindow(kDocumentWindowClass, kWindowStandardDocumentAttributes | COMP, &second, &b) == noErr);
	InstallWindowEventHandler(a, NewEventHandlerUPP(keyHandler), 1, keys, a, NULL);
	InstallWindowEventHandler(b, NewEventHandlerUPP(keyHandler), 1, keys, b, NULL);
	ShowWindow(a);
	ShowWindow(b);
	spin(500);
	SelectWindow(a);
	spin(300);

	// by default the focus is the active window
	WindowRef active = GetUserFocusWindow();
	CHECK(active != NULL && IsWindowActive(active));
	CHECK(GetUserFocusEventTarget() == GetWindowEventTarget(active));
	CHECK(sendKey() == active);

	// the other window can be given the focus
	WindowRef other = active == a ? b : a;
	CHECK(SetUserFocusWindow(other) == noErr);
	CHECK(GetUserFocusWindow() == other);
	CHECK(GetUserFocusEventTarget() == GetWindowEventTarget(other));
	CHECK(sendKey() == other);

	// kUserFocusAuto gives it back to the active window
	CHECK(SetUserFocusWindow(kUserFocusAuto) == noErr);
	CHECK(GetUserFocusWindow() == active);
	CHECK(sendKey() == active);

	// what is no window is refused, and does not change the focus
	CHECK(SetUserFocusWindow((WindowRef) 0x1000) == errInvalidWindowPtr);
	CHECK(SetUserFocusWindow(NULL) == errInvalidWindowPtr);
	CHECK(GetUserFocusWindow() == active);

	// a window that is disposed of while it has the focus gives it back
	CHECK(SetUserFocusWindow(other) == noErr);
	DisposeWindow(other);
	spin(200);
	WindowRef left = active;
	CHECK(GetUserFocusWindow() != other);
	WindowRef now = GetUserFocusWindow();
	CHECK(now == NULL || IsValidWindowPtr(now));
	CHECK(sendKey() == now);

	// with no window the application gets the focus
	DisposeWindow(left);
	spin(200);
	CHECK(GetUserFocusWindow() == NULL);
	CHECK(GetUserFocusEventTarget() == GetApplicationEventTarget());
	CHECK(sendKey() == NULL);

	printf(g_failures ? "FAIL carbon_user_focus_window\n" : "PASS carbon_user_focus_window\n");
	return g_failures != 0;
}
