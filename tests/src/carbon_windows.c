// Copyright (C) 2026 Lubos Dolezel
//
// The Carbon Window Manager on CoreGraphics windows: creating windows of the classes, their attributes,
// bounds, title, refcon and properties, showing, hiding, stacking, the events of a window and, on 32-bit,
// the window's port: drawing with QDBeginCGContext() (read back with CGWindowListCreateImage()), the
// update events InvalWindowRect() causes, and the conversion between local and global coordinates.
// Needs a window system (X11 or Wayland). With the argument "interact" it then waits for a click in the
// window and a close request from the window system (see carbon_windows_x11.sh).
#include <Carbon/Carbon.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

// The private way to the CoreGraphics window of a Carbon window, for reading its pixels back
extern unsigned int _HIWindowGetCGSWindowID(WindowRef window);

// Every 64-bit window is a compositing window
#ifdef __LP64__
#define COMP kWindowCompositingAttribute
#else
#define COMP 0
#endif

static int failures, checks;
static int g_canPlace = 1;
#define CHECK(x) do { ++checks; if (!(x)) { ++failures; printf("FAIL line=%d %s\n", __LINE__, #x); fflush(stdout); } } while (0)

static void quitTimer(EventLoopTimerRef t, void* u) { QuitApplicationEventLoop(); }

static void spin(int ms)
{
	EventLoopTimerRef timer;
	InstallEventLoopTimer(GetMainEventLoop(), ms / 1000.0, 0, NewEventLoopTimerUPP(quitTimer), NULL, &timer);
	RunApplicationEventLoop();
	RemoveEventLoopTimer(timer);
}

static int g_counts[2048];
static WindowRef g_lastWindow;
static Rect g_lastCurrent, g_lastPrevious;
static UInt32 g_lastBoundsAttrs;
static int g_drawFill = 1;
static int g_clicks, g_closes, g_closed;
static Point g_clickPoint;
static UInt32 g_clickModifiers;

static void count(UInt32 kind) { if (kind < 2048) g_counts[kind]++; }

#ifndef __LP64__
static void drawContent(WindowRef window)
{
	CGContextRef ctx;
	Rect bounds;

	if (!g_drawFill || QDBeginCGContext(GetWindowPort(window), &ctx) != noErr)
		return;
	GetWindowPortBounds(window, &bounds);
	CGContextSetRGBFillColor(ctx, 0, 0, 1, 1);
	CGContextFillRect(ctx, CGRectMake(0, 0, bounds.right, bounds.bottom));
	// In the context's own coordinates the origin is the bottom left of the window
	CGContextSetRGBFillColor(ctx, 1, 0, 0, 1);
	CGContextFillRect(ctx, CGRectMake(10, 20, 40, 30));
	CGContextSynchronize(ctx);
	QDEndCGContext(GetWindowPort(window), &ctx);
}
#endif

static OSStatus handler(EventHandlerCallRef call, EventRef event, void* userData)
{
	UInt32 cls = GetEventClass(event), kind = GetEventKind(event);
	WindowRef window = NULL;

	if (cls != kEventClassWindow)
		return eventNotHandledErr;
	GetEventParameter(event, kEventParamDirectObject, typeWindowRef, NULL, sizeof(window), NULL, &window);
	g_lastWindow = window;
	count(kind);

	switch (kind)
	{
		case kEventWindowBoundsChanged:
			GetEventParameter(event, kEventParamCurrentBounds, typeQDRectangle, NULL, sizeof(Rect), NULL, &g_lastCurrent);
			GetEventParameter(event, kEventParamPreviousBounds, typeQDRectangle, NULL, sizeof(Rect), NULL, &g_lastPrevious);
			GetEventParameter(event, kEventParamAttributes, typeUInt32, NULL, sizeof(UInt32), NULL, &g_lastBoundsAttrs);
			break;
#ifndef __LP64__
		case kEventWindowDrawContent:
			drawContent(window);
			return noErr;
#endif
		case kEventWindowClickContentRgn:
			g_clicks++;
			GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(Point), NULL, &g_clickPoint);
			GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(UInt32), NULL, &g_clickModifiers);
			return noErr;
		case kEventWindowClose:
			g_closes++;
			return eventNotHandledErr;
		case kEventWindowClosed:
			g_closed++;
			return eventNotHandledErr;
	}
	return eventNotHandledErr;
}

static const EventTypeSpec kTypes[] = {
	{ kEventClassWindow, kEventWindowUpdate }, { kEventClassWindow, kEventWindowDrawContent }, { kEventClassWindow, kEventWindowPaint },
	{ kEventClassWindow, kEventWindowActivated }, { kEventClassWindow, kEventWindowDeactivated },
	{ kEventClassWindow, kEventWindowShowing }, { kEventClassWindow, kEventWindowShown },
	{ kEventClassWindow, kEventWindowHiding }, { kEventClassWindow, kEventWindowHidden },
	{ kEventClassWindow, kEventWindowBoundsChanged }, { kEventClassWindow, kEventWindowClose },
	{ kEventClassWindow, kEventWindowClosed }, { kEventClassWindow, kEventWindowClickContentRgn },
};

static void listen(WindowRef w)
{
	CHECK(InstallWindowEventHandler(w, NewEventHandlerUPP(handler), sizeof(kTypes) / sizeof(kTypes[0]), kTypes, NULL, NULL) == noErr);
}

#ifndef __LP64__
// The window's pixels as the window system shows them: RGBA, the top row first
static unsigned char* readWindowPixels(WindowRef w, size_t* width, size_t* height)
{
	CGImageRef image = CGWindowListCreateImage(CGRectNull, kCGWindowListOptionIncludingWindow, _HIWindowGetCGSWindowID(w),
		kCGWindowImageBoundsIgnoreFraming);

	if (!image)
		return NULL;
	*width = CGImageGetWidth(image);
	*height = CGImageGetHeight(image);

	unsigned char* pixels = calloc(*width * *height, 4);
	CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
	CGContextRef ctx = CGBitmapContextCreate(pixels, *width, *height, 8, *width * 4, space, kCGImageAlphaPremultipliedLast);

	CGContextSetBlendMode(ctx, kCGBlendModeCopy);
	CGContextDrawImage(ctx, CGRectMake(0, 0, *width, *height), image);
	CGContextRelease(ctx);
	CGColorSpaceRelease(space);
	CGImageRelease(image);
	return pixels;
}

static int pixelIs(const unsigned char* pixels, size_t width, size_t x, size_t y, int r, int g, int b, int a)
{
	const unsigned char* p = pixels + (y * width + x) * 4;

	return abs(p[0] - r) <= 2 && abs(p[1] - g) <= 2 && abs(p[2] - b) <= 2 && abs(p[3] - a) <= 2;
}
#endif

// ---- The behaviour of the Window Manager that its documentation does not give, observed on macOS 26.7.3
// x86_64 (see the QUIRK: comments in MacWindows.c). The Mojave window system was not available to the probes.

static int g_eventLog[64], g_eventCount;

static OSStatus logHandler(EventHandlerCallRef call, EventRef event, void* userData)
{
	if (g_eventCount < 64)
		g_eventLog[g_eventCount++] = (int) GetEventKind(event);
	return eventNotHandledErr;
}

static const EventTypeSpec kLogTypes[] = {
	{ kEventClassWindow, kEventWindowShowing }, { kEventClassWindow, kEventWindowShown },
	{ kEventClassWindow, kEventWindowHiding }, { kEventClassWindow, kEventWindowHidden },
	{ kEventClassWindow, kEventWindowActivated }, { kEventClassWindow, kEventWindowDeactivated },
	{ kEventClassWindow, kEventWindowHandleActivate }, { kEventClassWindow, kEventWindowHandleDeactivate },
	{ kEventClassWindow, kEventWindowBoundsChanging }, { kEventClassWindow, kEventWindowBoundsChanged },
	{ kEventClassWindow, kEventWindowClosed },
};

static int logIndexOf(int kind, int from)
{
	for (int i = from; i < g_eventCount; i++)
		if (g_eventLog[i] == kind)
			return i;
	return -1;
}

static void test_quirk_activation_events(void)
{
	WindowRef w;
	Rect r = { 100, 100, 200, 300 };

	CHECK(CreateNewWindow(kDocumentWindowClass, kWindowStandardDocumentAttributes | COMP, &r, &w) == noErr);
	CHECK(InstallWindowEventHandler(w, logHandler, sizeof(kLogTypes) / sizeof(kLogTypes[0]), kLogTypes, NULL, NULL) == noErr);
	g_eventCount = 0;
	ShowWindow(w);
	int showing = logIndexOf(kEventWindowShowing, 0), shown = logIndexOf(kEventWindowShown, 0);
	int activated = logIndexOf(kEventWindowActivated, 0), handle = logIndexOf(kEventWindowHandleActivate, 0);
#ifdef __LP64__
	CHECK(showing >= 0 && shown > showing && activated > shown && handle == activated + 1);
#else
	// no standard handler: no kEventWindowHandleActivate (observed on Mojave 10.14.4 i386)
	CHECK(showing >= 0 && shown > showing && activated > shown && handle < 0);
#endif
	g_eventCount = 0;
	HideWindow(w);
	int hiding = logIndexOf(kEventWindowHiding, 0), hidden = logIndexOf(kEventWindowHidden, 0);
	int deactivated = logIndexOf(kEventWindowDeactivated, 0), handleDeactivate = logIndexOf(kEventWindowHandleDeactivate, 0);
#ifdef __LP64__
	CHECK(hiding >= 0 && hidden > hiding && deactivated > hidden && handleDeactivate == deactivated + 1);
#else
	CHECK(hiding >= 0 && hidden > hiding && deactivated > hidden && handleDeactivate < 0);
#endif

	// And disposing of the window closes it (QUIRK: DisposeWindow sends kEventWindowClosed)
	g_eventCount = 0;
	DisposeWindow(w);
	CHECK(logIndexOf(kEventWindowClosed, 0) >= 0);
	CHECK(!IsValidWindowPtr(w));
}

static void test_quirk_bounds_changing_event(void)
{
	WindowRef w;
	Rect r = { 100, 100, 200, 300 }, moved = { 110, 120, 230, 350 };

	CHECK(CreateNewWindow(kDocumentWindowClass, kWindowStandardDocumentAttributes | COMP, &r, &w) == noErr);
	CHECK(InstallWindowEventHandler(w, logHandler, sizeof(kLogTypes) / sizeof(kLogTypes[0]), kLogTypes, NULL, NULL) == noErr);
	g_eventCount = 0;
	CHECK(SetWindowBounds(w, kWindowContentRgn, &moved) == noErr);
	int changing = logIndexOf(kEventWindowBoundsChanging, 0), changed = logIndexOf(kEventWindowBoundsChanged, 0);
	CHECK(changing >= 0 && changed == changing + 1);
	DisposeWindow(w);
}

static void test_quirk_window_attributes_for_class(void)
{
	Rect r = { 100, 100, 200, 300 };
	WindowRef w = NULL;
	WindowAttributes attrs;

	// A class that has no frame takes no close box, ... ; the others take what a window of their kind has
	CHECK(CreateNewWindow(kAlertWindowClass, kWindowCloseBoxAttribute | COMP, &r, &w) == errUnsupportedWindowAttributesForClass);
	CHECK(w == NULL);
	CHECK(CreateNewWindow(kPlainWindowClass, kWindowResizableAttribute | COMP, &r, &w) == errUnsupportedWindowAttributesForClass);
	CHECK(CreateNewWindow(kOverlayWindowClass, kWindowStandardDocumentAttributes | COMP, &r, &w) == errUnsupportedWindowAttributesForClass);
	CHECK(CreateNewWindow(kMovableModalWindowClass, kWindowStandardDocumentAttributes | COMP, &r, &w) == errUnsupportedWindowAttributesForClass);
	CHECK(CreateNewWindow(kDocumentWindowClass, kWindowInWindowMenuAttribute | COMP, &r, &w) == noErr);
	DisposeWindow(w);
	CHECK(CreateNewWindow(kFloatingWindowClass, kWindowInWindowMenuAttribute | COMP, &r, &w) == errUnsupportedWindowAttributesForClass);
	CHECK(CreateNewWindow(kMovableModalWindowClass, kWindowCloseBoxAttribute | COMP, &r, &w) == noErr);
	DisposeWindow(w);
	CHECK(CreateNewWindow(kAlertWindowClass, kWindowStandardHandlerAttribute | kWindowIgnoreClicksAttribute | COMP, &r, &w) == noErr);
	DisposeWindow(w);
#ifdef __LP64__
	// A 64-bit window is a compositing window
	CHECK(CreateNewWindow(kDocumentWindowClass, 0, &r, &w) == errUnsupportedWindowAttributesForClass);
	CHECK(CreateNewWindow(kDocumentWindowClass, kWindowStandardDocumentAttributes, &r, &w) == errUnsupportedWindowAttributesForClass);
	// and has the attributes of its class
	CHECK(CreateNewWindow(kFloatingWindowClass, COMP, &r, &w) == noErr);
	CHECK(GetWindowAttributes(w, &attrs) == noErr && attrs == (COMP | kWindowHideOnSuspendAttribute));
	DisposeWindow(w);
	CHECK(CreateNewWindow(kOverlayWindowClass, COMP, &r, &w) == noErr);
	CHECK(GetWindowAttributes(w, &attrs) == noErr && attrs == (COMP | kWindowNoShadowAttribute));
	DisposeWindow(w);
#else
	CHECK(CreateNewWindow(kDocumentWindowClass, 0, &r, &w) == noErr);
	DisposeWindow(w);
	(void) attrs;
#endif
}

static void test_quirk_window_structure_of_class(void)
{
	Rect r = { 100, 100, 200, 300 }, structure;
	WindowRef w;

	CHECK(CreateNewWindow(kDocumentWindowClass, COMP, &r, &w) == noErr);
	CHECK(GetWindowBounds(w, kWindowStructureRgn, &structure) == noErr && structure.top == 78 && structure.left == 100 && structure.bottom == 200 && structure.right == 300);
	CHECK(GetWindowBounds(w, kWindowTitleBarRgn, &structure) == noErr && structure.top == 78 && structure.bottom == 100);
	// setting the structure moves the content with it
	CHECK(SetWindowBounds(w, kWindowStructureRgn, &(Rect) { 50, 60, 200, 300 }) == noErr);
	CHECK(GetWindowBounds(w, kWindowContentRgn, &structure) == noErr && structure.bottom - structure.top == 128);
	if (g_canPlace)
		CHECK(structure.top == 72 && structure.left == 60);
	DisposeWindow(w);
	CHECK(CreateNewWindow(kDocumentWindowClass, kWindowMetalAttribute | COMP, &r, &w) == noErr);
	CHECK(GetWindowBounds(w, kWindowStructureRgn, &structure) == noErr && structure.top == 78 && structure.left == 95 && structure.bottom == 205 && structure.right == 305);
	DisposeWindow(w);
	CHECK(CreateNewWindow(kFloatingWindowClass, COMP, &r, &w) == noErr);
	CHECK(GetWindowBounds(w, kWindowStructureRgn, &structure) == noErr && structure.top == 88);
	DisposeWindow(w);
	CHECK(CreateNewWindow(kPlainWindowClass, COMP, &r, &w) == noErr);
	CHECK(GetWindowBounds(w, kWindowStructureRgn, &structure) == noErr && structure.top == 99 && structure.left == 99 && structure.bottom == 201 && structure.right == 301);
	CHECK(GetWindowBounds(w, kWindowTitleBarRgn, &structure) == errWindowRegionCodeInvalid);
	DisposeWindow(w);
	CHECK(CreateNewWindow(kAlertWindowClass, COMP, &r, &w) == noErr);
	CHECK(GetWindowBounds(w, kWindowStructureRgn, &structure) == noErr && structure.top == 100);
#ifdef __LP64__
	CHECK(GetWindowBounds(w, kWindowTitleBarRgn, &structure) == errWindowRegionCodeInvalid);
#else
	// an empty rectangle for an alert (and a modal dialog) in the 32-bit API (observed on Mojave 10.14.4 i386)
	CHECK(GetWindowBounds(w, kWindowTitleBarRgn, &structure) == noErr && structure.top == 0 && structure.bottom == 0 && structure.left == 0 && structure.right == 0);
#endif
	DisposeWindow(w);
}

static void test_quirk_empty_window_rect(void)
{
	WindowRef w = NULL;
	Rect empty = { 10, 10, 10, 10 }, turned = { 10, 30, 10, 20 }, got;

	CHECK(CreateNewWindow(kDocumentWindowClass, COMP, &empty, &w) == noErr && w != NULL);
	if (w)
	{
		CHECK(GetWindowBounds(w, kWindowContentRgn, &got) == noErr && EqualRect(&got, &empty));
		DisposeWindow(w);
	}
	w = NULL;
	CHECK(CreateNewWindow(kDocumentWindowClass, COMP, &turned, &w) == noErr && w != NULL);
	if (w)
		DisposeWindow(w);
}

static void test_quirk_null_window_title(void)
{
	WindowRef w;
	CFStringRef title = NULL;

	CHECK(CreateNewWindow(kDocumentWindowClass, COMP, &(Rect) { 100, 100, 200, 300 }, &w) == noErr);
	// an untitled window has the empty title
	CHECK(CopyWindowTitleAsCFString(w, &title) == noErr && title != NULL && CFStringGetLength(title) == 0);
	if (title)
		CFRelease(title);
	CHECK(SetWindowTitleWithCFString(w, CFSTR("Title")) == noErr);
	CHECK(SetWindowTitleWithCFString(w, NULL) == paramErr);
	CHECK(CopyWindowTitleAsCFString(w, &title) == noErr && title && CFStringCompare(title, CFSTR("Title"), 0) == kCFCompareEqualTo);
	if (title)
		CFRelease(title);
	DisposeWindow(w);
}

static void test_quirk_window_property_edges(void)
{
	WindowRef w;
	unsigned char value[4] = { 0xEF, 0xBE, 0xAD, 0xDE }, buffer[4];
	ByteCount actual = 99;

	CHECK(CreateNewWindow(kDocumentWindowClass, COMP, &(Rect) { 100, 100, 200, 300 }, &w) == noErr);
	CHECK(GetWindowProperty(w, 'DARL', 'none', sizeof(buffer), &actual, buffer) == errWindowPropertyNotFound);
	CHECK(actual == 99);	// left alone
	CHECK(SetWindowProperty(w, 'DARL', 'prop', sizeof(value), value) == noErr);
	memset(buffer, 0xEE, sizeof(buffer));
	CHECK(GetWindowProperty(w, 'DARL', 'prop', 2, &actual, buffer) == noErr);	// small buffer: the first bytes
	CHECK(actual == 4 && buffer[0] == 0xEF && buffer[1] == 0xBE && buffer[2] == 0xEE);
	CHECK(GetWindowProperty(w, 'DARL', 'prop', 2, NULL, buffer) == paramErr);	// and nowhere to tell the size
	CHECK(SetWindowProperty(w, 'DARL', 'zero', 4, NULL) == noErr);
#ifdef __LP64__
	CHECK(GetWindowProperty(w, 'DARL', 'zero', 4, &actual, buffer) == noErr && actual == 4 && buffer[0] == 0 && buffer[3] == 0);
#else
	// no data and no size in the 32-bit API (observed on Mojave 10.14.4 i386)
	memset(buffer, 0xEE, sizeof(buffer));
	CHECK(GetWindowProperty(w, 'DARL', 'zero', 4, &actual, buffer) == noErr && actual == 0 && buffer[0] == 0xEE);
#endif
	CHECK(RemoveWindowProperty(w, 'DARL', 'prop') == noErr);
	CHECK(RemoveWindowProperty(w, 'DARL', 'prop') == noErr);	// not there any more
	DisposeWindow(w);
	// a window that is not there
	CHECK(SetWindowProperty(NULL, 'DARL', 'prop', 4, value) == errInvalidWindowPtr);
	CHECK(GetWindowProperty(NULL, 'DARL', 'prop', 4, NULL, buffer) == errInvalidWindowPtr);
}

static void test_quirk_get_window_bounds_null_window(void)
{
	Rect got;

	CHECK(GetWindowBounds(NULL, kWindowContentRgn, &got) == paramErr);
	CHECK(GetWindowClass(NULL, &(WindowClass) { 0 }) == errInvalidWindowPtr);
}

static void runQuirkTests(void)
{
	test_quirk_activation_events();
	test_quirk_bounds_changing_event();
	test_quirk_window_attributes_for_class();
	test_quirk_window_structure_of_class();
	test_quirk_empty_window_rect();
	test_quirk_null_window_title();
	test_quirk_window_property_edges();
	test_quirk_get_window_bounds_null_window();
}

int main(int argc, char** argv)
{
	int interact = argc > 1 && strncmp(argv[1], "interact", 8) == 0;
	// A window system that does not let a program place its windows (Wayland) keeps the place it gave
	int canPlace = !(getenv("DARLING_APPKIT_BACKEND") && strcmp(getenv("DARLING_APPKIT_BACKEND"), "wayland") == 0);
	g_canPlace = canPlace;
	int closeByUser = argc > 1 && strcmp(argv[1], "interact") == 0;
	Rect r = { 100, 100, 300, 400 };
	WindowRef doc = NULL, pal = NULL, over = NULL, sentinel = (WindowRef) (uintptr_t) 0x1234, w;
	OSStatus e;

	printf("WINDOWS bits=%zu\n", sizeof(void*) * 8);
	runQuirkTests();

	// Arguments
	w = sentinel;
	CHECK(CreateNewWindow(kDocumentWindowClass, 0, NULL, &w) == paramErr);
	CHECK(w == NULL);
	CHECK(CreateNewWindow(kDocumentWindowClass, 0, &r, NULL) == paramErr);
		w = sentinel;
	CHECK(CreateNewWindow(99, COMP, &r, &w) == errUnrecognizedWindowClass);
	CHECK(w == NULL);
	w = sentinel;
	CHECK(CreateNewWindow(kAllWindowClasses, COMP, &r, &w) == errUnrecognizedWindowClass);
	CHECK(!IsValidWindowPtr(NULL));
	CHECK(!IsWindowVisible(NULL));
	DisposeWindow(NULL);
	ShowWindow(NULL);
	HideWindow(NULL);
	ShowHide(NULL, 1);

	// A document window
	e = CreateNewWindow(kDocumentWindowClass, kWindowStandardDocumentAttributes | kWindowStandardHandlerAttribute | COMP, &r, &doc);
	CHECK(e == noErr);
	CHECK(doc != NULL);
	if (!doc)
	{
		printf("WINDOWS_CONTRACT checks=%d failures=%d\n", checks, failures);
		return 1;
	}
	CHECK(IsValidWindowPtr(doc));
	CHECK(!IsWindowVisible(doc));
	CHECK(!IsWindowActive(doc));
	listen(doc);

	WindowClass cls = 0;
	WindowAttributes attrs = 0;
	CHECK(GetWindowClass(doc, &cls) == noErr && cls == kDocumentWindowClass);
	#ifdef __LP64__
	// the compositing attribute and the attribute of the class (QUIRK: see test_quirk_window_attributes_for_class)
	CHECK(GetWindowAttributes(doc, &attrs) == noErr && attrs == (kWindowStandardDocumentAttributes | kWindowStandardHandlerAttribute | COMP | kWindowInWindowMenuAttribute));
#else
	// the attribute of the class too (observed on Mojave 10.14.4 i386)
	CHECK(GetWindowAttributes(doc, &attrs) == noErr && attrs == (kWindowStandardDocumentAttributes | kWindowStandardHandlerAttribute | kWindowInWindowMenuAttribute));
#endif

	Rect content, structure, portBounds;
	CHECK(GetWindowBounds(doc, kWindowContentRgn, &content) == noErr);
	CHECK(content.top == 100 && content.left == 100 && content.bottom == 300 && content.right == 400);
	CHECK(GetWindowBounds(doc, kWindowStructureRgn, &structure) == noErr);
	CHECK(structure.top < content.top && structure.left <= content.left && structure.bottom == content.bottom && structure.right >= content.right);
	CHECK(GetWindowBounds(doc, 7777, &content) == errWindowRegionCodeInvalid);
	CHECK(GetWindowPortBounds(doc, &portBounds) == &portBounds);
	CHECK(portBounds.top == 0 && portBounds.left == 0 && portBounds.bottom == 200 && portBounds.right == 300);
	CHECK(GetWindowClass(NULL, &cls) == errInvalidWindowPtr);

	// Title
	CFStringRef title = NULL;
	CHECK(SetWindowTitleWithCFString(doc, CFSTR("Carbon Window Test")) == noErr);
	CHECK(CopyWindowTitleAsCFString(doc, &title) == noErr);
	CHECK(title && CFStringCompare(title, CFSTR("Carbon Window Test"), 0) == kCFCompareEqualTo);
	if (title)
		CFRelease(title);

	// Properties
	UInt32 value = 0xDEADBEEF, got = 0;
	ByteCount actual = 99;
	char small[2];
	CHECK(GetWindowProperty(doc, 'DARL', 'prop', sizeof(got), &actual, &got) == errWindowPropertyNotFound);
	CHECK(SetWindowProperty(doc, 'DARL', 'prop', sizeof(value), &value) == noErr);
	CHECK(GetWindowProperty(doc, 'DARL', 'prop', sizeof(got), &actual, &got) == noErr && got == 0xDEADBEEF && actual == sizeof(got));
	actual = 0;
	CHECK(GetWindowProperty(doc, 'DARL', 'prop', sizeof(small), &actual, small) == noErr);	// QUIRK, see test_quirk_window_property_edges
	CHECK(actual == sizeof(value));
	CHECK(GetWindowPropertySize(doc, 'DARL', 'prop', &actual) == noErr && actual == sizeof(value));
	value = 7;
	CHECK(SetWindowProperty(doc, 'DARL', 'prop', sizeof(value), &value) == noErr);
	CHECK(GetWindowProperty(doc, 'DARL', 'prop', sizeof(got), NULL, &got) == noErr && got == 7);
	CHECK(GetWindowProperty(doc, 'DARL', 'othr', sizeof(got), NULL, &got) == errWindowPropertyNotFound);
	CHECK(RemoveWindowProperty(doc, 'DARL', 'prop') == noErr);
	CHECK(RemoveWindowProperty(doc, 'DARL', 'prop') == noErr);	// QUIRK, see test_quirk_window_property_edges

#ifndef __LP64__
	// Refcon
	CHECK(GetWRefCon(doc) == 0);
	SetWRefCon(doc, 0x1234);
	CHECK(GetWRefCon(doc) == 0x1234);

	// The port
	CGrafPtr port = GetWindowPort(doc);
	CHECK(port != NULL);
	CHECK(GetWindowFromPort(port) == doc);
	SetPortWindowPort(doc);
	GrafPtr current = NULL;
	GetPort(&current);
	CHECK(current == port);
	Rect portRect;
	GetPortBounds(port, &portRect);
	CHECK(portRect.top == 0 && portRect.left == 0 && portRect.bottom == 200 && portRect.right == 300);
	Point point = { 50, 60 };
	QDLocalToGlobalPoint(port, &point);
	CHECK(point.v == 150 && point.h == 160);
	QDGlobalToLocalPoint(port, &point);
	CHECK(point.v == 50 && point.h == 60);
	Rect local = { 0, 0, 10, 10 };
	QDLocalToGlobalRect(port, &local);
	CHECK(local.top == 100 && local.left == 100 && local.bottom == 110 && local.right == 110);
	QDGlobalToLocalRect(port, &local);
	CHECK(local.top == 0 && local.left == 0 && local.bottom == 10 && local.right == 10);
#else
	CHECK(GetWRefCon(doc) == 0);
#endif

	// Showing: the events come in order, the content is drawn
	CHECK(FrontNonFloatingWindow() == NULL);
	ShowWindow(doc);
	CHECK(IsWindowVisible(doc));
	CHECK(g_counts[kEventWindowShowing] == 1);
	CHECK(g_counts[kEventWindowShown] == 1);
	CHECK(g_lastWindow == doc);
	CHECK(FrontNonFloatingWindow() == doc);
	CHECK(FrontWindow() == doc);
	spin(600);
	CHECK(IsWindowActive(doc));
#ifndef __LP64__
	CHECK(g_counts[kEventWindowDrawContent] == 1);
	// test_quirk_window_events_32bit: a shown window gets kEventWindowPaint, not kEventWindowUpdate (Mojave i386)
	CHECK(g_counts[kEventWindowPaint] == 1 && g_counts[kEventWindowUpdate] == 0);

	size_t width, height;
	unsigned char* pixels = readWindowPixels(doc, &width, &height);
	CHECK(pixels != NULL);
	if (pixels)
	{
		CHECK(width == 300 && height == 200);
		// The red rectangle: x 10..50 and, counted from the bottom, y 20..50
		CHECK(pixelIs(pixels, width, 30, height - 35, 255, 0, 0, 255));
		CHECK(pixelIs(pixels, width, 30, 10, 0, 0, 255, 255));
		CHECK(pixelIs(pixels, width, 5, height - 35, 0, 0, 255, 255));
		CHECK(pixelIs(pixels, width, 30, height - 60, 0, 0, 255, 255));
		free(pixels);
	}

	// Nothing is drawn again until something is invalidated
	spin(300);
	CHECK(g_counts[kEventWindowDrawContent] == 1);
	Rect inval = { 10, 10, 50, 50 };
	CHECK(InvalWindowRect(doc, &inval) == noErr);
	CHECK(g_counts[kEventWindowDrawContent] == 1);	// asked for, not done in the call
	CHECK(InvalWindowRect(doc, &inval) == noErr);	// one draw however often it is asked
	spin(300);
	CHECK(g_counts[kEventWindowDrawContent] == 2);
	CHECK(InvalWindowRect(NULL, &inval) == errInvalidWindowPtr);
#endif

	// Bounds
	g_counts[kEventWindowBoundsChanged] = 0;
	Rect before;
	GetWindowBounds(doc, kWindowContentRgn, &before);
	Rect moved = { 120, 130, 320, 500 };
	CHECK(SetWindowBounds(doc, kWindowContentRgn, &moved) == noErr);
	CHECK(GetWindowBounds(doc, kWindowContentRgn, &content) == noErr);
	CHECK(content.right - content.left == 370 && content.bottom - content.top == 200);
	if (canPlace)
		CHECK(content.top == 120 && content.left == 130);
	CHECK(g_counts[kEventWindowBoundsChanged] == 1);
	CHECK(EqualRect(&g_lastCurrent, &content));
	CHECK(EqualRect(&g_lastPrevious, &before));
	CHECK(g_lastBoundsAttrs & kWindowBoundsChangeSizeChanged);
	if (canPlace)
		CHECK(g_lastBoundsAttrs & kWindowBoundsChangeOriginChanged);
	CHECK(!(g_lastBoundsAttrs & (kWindowBoundsChangeUserDrag | kWindowBoundsChangeUserResize)));
	GetWindowPortBounds(doc, &portBounds);
	CHECK(portBounds.right == 370 && portBounds.bottom == 200);
	spin(400);
#ifndef __LP64__
	CHECK(g_counts[kEventWindowDrawContent] >= 3);	// the new size is drawn
	point = (Point) { 0, 0 };
	QDLocalToGlobalPoint(port, &point);
	GetWindowBounds(doc, kWindowContentRgn, &content);
	CHECK(point.h == content.left && point.v == content.top);
#endif
	g_counts[kEventWindowBoundsChanged] = 0;
	SizeWindow(doc, 250, 150, true);
	GetWindowBounds(doc, kWindowContentRgn, &content);
	CHECK(content.right - content.left == 250 && content.bottom - content.top == 150);
	CHECK(g_counts[kEventWindowBoundsChanged] == 1);
	CHECK(g_lastBoundsAttrs & kWindowBoundsChangeSizeChanged);
	CHECK(!(g_lastBoundsAttrs & kWindowBoundsChangeOriginChanged));
	spin(300);

	// The classes: a floating window stays in front of the documents, without being the active window
	CHECK(CreateNewWindow(kFloatingWindowClass, kWindowStandardFloatingAttributes | COMP, &(Rect) { 120, 500, 220, 600 }, &pal) == noErr);
	listen(pal);
	ShowWindow(pal);
	spin(300);
	CHECK(FrontWindow() == pal);
	CHECK(FrontNonFloatingWindow() == doc);
	CHECK(ActiveNonFloatingWindow() == doc);
	CHECK(IsWindowActive(doc));
	SelectWindow(doc);
	CHECK(FrontWindow() == pal);
	CHECK(GetNextWindow(pal) == doc);

	WindowRef doc2 = NULL;
	CHECK(CreateNewWindow(kDocumentWindowClass, kWindowStandardDocumentAttributes | COMP, &(Rect) { 150, 150, 350, 450 }, &doc2) == noErr);
	listen(doc2);
	g_counts[kEventWindowDeactivated] = 0;
	ShowWindow(doc2);
	SelectWindow(doc2);
	CHECK(FrontNonFloatingWindow() == doc2);
	CHECK(IsWindowActive(doc2) && !IsWindowActive(doc));
	CHECK(g_counts[kEventWindowDeactivated] == 1);
	SelectWindow(doc);
	CHECK(FrontNonFloatingWindow() == doc);
	CHECK(IsWindowActive(doc) && !IsWindowActive(doc2));
	DisposeWindow(doc2);
	CHECK(!IsValidWindowPtr(doc2));
	CHECK(IsWindowActive(doc));

	// An overlay window: shown over the others, transparent where nothing is drawn
	CHECK(CreateNewWindow(kOverlayWindowClass, kWindowHideOnSuspendAttribute | kWindowIgnoreClicksAttribute | COMP, &(Rect) { 100, 100, 300, 400 }, &over) == noErr);
	ShowWindow(over);
	spin(300);
	CHECK(IsWindowVisible(over));
#ifdef __LP64__
	CHECK(!IsWindowActive(over));
#else
	// test_quirk_overlay_window_active: it is active in the 32-bit API (observed on Mojave 10.14.4 i386)
	CHECK(IsWindowActive(over));
#endif
	CHECK(FrontNonFloatingWindow() == doc);
#ifndef __LP64__
	{
		CGContextRef ctx;

		CHECK(QDBeginCGContext(GetWindowPort(over), &ctx) == noErr);
		CGContextSetRGBFillColor(ctx, 0, 1, 0, 1);
		CGContextFillRect(ctx, CGRectMake(50, 50, 20, 20));
		CGContextSynchronize(ctx);
		CHECK(QDEndCGContext(GetWindowPort(over), &ctx) == noErr);
		CHECK(ctx == NULL);
		pixels = readWindowPixels(over, &width, &height);
		CHECK(pixels != NULL);
		if (pixels)
		{
			CHECK(pixelIs(pixels, width, 60, height - 60, 0, 255, 0, 255));
			CHECK(pixelIs(pixels, width, 5, 5, 0, 0, 0, 0));
			free(pixels);
		}
	}
#endif
	DisposeWindow(over);

	HideWindow(doc);
	CHECK(!IsWindowVisible(doc));
	CHECK(g_counts[kEventWindowHidden] >= 1);
	CHECK(FrontNonFloatingWindow() == NULL);
	ShowWindow(doc);
	spin(300);
	DisposeWindow(pal);

	if (interact)
	{
		// Wait for a click in the content and a close request from the window system
		GetWindowBounds(doc, kWindowContentRgn, &content);
		printf("READY %d %d %d %d\n", content.left, content.top, content.right, content.bottom);
		fflush(stdout);
		for (int i = 0; i < 400 && g_clicks == 0; i++)
			spin(100);
		CHECK(g_clicks >= 1);
		if (g_clicks)
		{
			printf("CLICK %d %d\n", g_clickPoint.h, g_clickPoint.v);
			fflush(stdout);
			CHECK(g_clickPoint.h >= content.left && g_clickPoint.h < content.right);
			CHECK(g_clickPoint.v >= content.top && g_clickPoint.v < content.bottom);
		}
		// The windows disposed of so far also got kEventWindowClosed: count from here
		g_closes = g_closed = 0;
		printf("READY_CLOSE\n");
		fflush(stdout);
		if (closeByUser)
		{
			for (int i = 0; i < 100 && g_closed == 0; i++)
				spin(100);
			CHECK(g_closes >= 1);
			CHECK(g_closed == 1);	// the standard handler closed the window
			CHECK(!IsValidWindowPtr(doc));
		}
		else
			DisposeWindow(doc);
	}
	else
	{
		DisposeWindow(doc);
		CHECK(!IsValidWindowPtr(doc));
	}

	printf("WINDOWS_CONTRACT bits=%zu checks=%d failures=%d\n", sizeof(void*) * 8, checks, failures);
	return failures != 0;
}
