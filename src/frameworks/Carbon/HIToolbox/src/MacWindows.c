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

// The Window Manager. A Carbon window is a CoreGraphics (CGS) window: CoreGraphics creates, shows, orders,
// titles and moves it, whichever window system is behind it (X11 or Wayland), and keeps the
// pixels that are drawn into it. HIToolbox keeps what the Window Manager has on top of that: the class and
// attributes, the stacking of the layers, the active window, the refcon and properties, the update region
// and the event target of each window.
//
// What the window system tells (a close request, the focus, a new size) arrives as a CoreGraphics window
// event (_CGSSetWindowEventProc), the mouse as the CGEvents of the event stream (see CarbonEventStream.c).
// Both end up as Carbon events for the window's event target.
//
// On 32-bit a window has a QuickDraw port, an ordinary offscreen port that the window's pixels live in. What a
// program draws into it (QDBeginCGContext() or QuickDraw) is put on the window when the port is flushed
// (QDFlushPortBuffer(), which QDEndCGContext() calls).

#include <HIToolbox/MacWindows.h>
#include <HIToolbox/CarbonEvents.h>
#include <HIToolbox/Events.h>
#include "HIToolboxInternal.h"
#include "HIToolboxPrivate.h"
#include <CoreGraphics/CoreGraphicsPrivate.h>
#include <CoreGraphics/CGSServerSPI.h>
#include <AE/AEDataModel.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef __LP64__
// The QuickDraw SPI for the ports of the windows (QDPorts.c)
extern void QDPortSetGlobalOrigin(CGrafPtr port, short h, short v);
extern OSStatus _QDResizePort(CGrafPtr port, short width, short height);
#endif

#define kWindowMagic 0x57494e44 // 'WIND'

struct WindowProperty
{
	struct WindowProperty* next;
	OSType creator;
	OSType tag;
	ByteCount size;
	char data[];
};

struct OpaqueWindowPtr
{
	UInt32 magic;
	atomic_int retainCount;
	CGSWindowID cgsWindow;
	WindowClass windowClass;
	WindowAttributes attributes;
	Rect content;            // the content area in global coordinates, the origin at the top left
	CFStringRef title;
	long refCon;
	struct WindowProperty* properties;
	EventTargetRef target;
	bool visible;
	bool active;
	bool updatePending;      // a kEventWindowUpdate is queued
	bool inUpdate;
	RgnHandle updateRegion;
	CGrafPtr port;           // 32-bit
	struct OpaqueWindowPtr* next;   // behind this one
};

static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static struct OpaqueWindowPtr* g_windows;     // front to back
static struct OpaqueWindowPtr* g_activeWindow;
static struct OpaqueWindowPtr* g_mouseGrab;   // the window that got the mouse down, until the button is up
static bool g_eventProcRegistered;
static CGSWindowID g_keyWindow;     // the window of the process that has the keyboard focus, 0 for none
static bool g_focusCheckQueued;

// The default handler of every window: what a window does when its handlers did not handle an event
static OSStatus windowDefaultHandler(EventHandlerCallRef call, EventRef event, void* userData);


static CGSConnectionID connection(void)
{
	return CGSMainConnectionID();
}

#pragma mark Layers

// The layer a class lives in: the windows of a higher layer are in front of those of a lower one
static int layerOfClass(WindowClass cls)
{
	switch (cls)
	{
		case kOverlayWindowClass:
		case kHelpWindowClass:
			return 3;
		case kModalWindowClass:
		case kMovableModalWindowClass:
		case kAlertWindowClass:
		case kMovableAlertWindowClass:
		case kSheetAlertWindowClass:
			return 2;
		case kFloatingWindowClass:
		case kUtilityWindowClass:
		case kToolbarWindowClass:
			return 1;
		default:
			return 0;
	}
}

static bool isFloating(WindowRef window)
{
	return layerOfClass(window->windowClass) == 1;
}

// Takes the window out of the list (lock held)
static void unlinkWindow(WindowRef window)
{
	for (struct OpaqueWindowPtr** p = &g_windows; *p; p = &(*p)->next)
	{
		if (*p == window)
		{
			*p = window->next;
			break;
		}
	}
	window->next = NULL;
}

// Puts the window in front of the windows of its layer (lock held)
static void linkWindowInFront(WindowRef window)
{
	struct OpaqueWindowPtr** p = &g_windows;
	while (*p && layerOfClass((*p)->windowClass) > layerOfClass(window->windowClass))
		p = &(*p)->next;
	window->next = *p;
	*p = window;
}

static bool isLive(WindowRef window)
{
	for (struct OpaqueWindowPtr* w = g_windows; w; w = w->next)
	{
		if (w == window)
			return true;
	}
	return false;
}

static WindowRef windowForCGSWindow(CGSWindowID wid)
{
	WindowRef found = NULL;

	pthread_mutex_lock(&g_lock);
	for (struct OpaqueWindowPtr* w = g_windows; w; w = w->next)
	{
		if (w->cgsWindow == wid)
		{
			found = w;
			break;
		}
	}
	pthread_mutex_unlock(&g_lock);
	return found;
}

#pragma mark Events

static OSStatus sendWindowEvent(WindowRef window, UInt32 kind, void (^addParameters)(EventRef event))
{
	EventRef event;

	if (CreateEvent(NULL, kEventClassWindow, kind, _HICurrentTime(), kEventAttributeNone, &event) != noErr)
		return memFullErr;
	SetEventParameter(event, kEventParamDirectObject, typeWindowRef, sizeof(window), &window);
	if (addParameters)
		addParameters(event);
	OSStatus result = SendEventToEventTarget(event, window->target);
	ReleaseEvent(event);
	return result;
}

static void postWindowEvent(WindowRef window, UInt32 kind, void (^addParameters)(EventRef event))
{
	EventRef event;

	if (CreateEvent(NULL, kEventClassWindow, kind, _HICurrentTime(), kEventAttributeNone, &event) != noErr)
		return;
	SetEventParameter(event, kEventParamDirectObject, typeWindowRef, sizeof(window), &window);
	if (addParameters)
		addParameters(event);
	PostEventToQueue(GetMainEventQueue(), event, kEventPriorityStandard);
	ReleaseEvent(event);
}

// Asks for the window's content to be drawn: one kEventWindowUpdate is queued however many parts are invalidated
static void scheduleUpdate(WindowRef window)
{
	if (window->updatePending || !window->visible || (window->attributes & kWindowNoUpdatesAttribute))
		return;
	window->updatePending = true;
	postWindowEvent(window, kEventWindowUpdate, NULL);
}

static void setActive(WindowRef window, bool active)
{
	if (window->active == active)
		return;
	window->active = active;
	sendWindowEvent(window, active ? kEventWindowActivated : kEventWindowDeactivated, NULL);
}

static bool canActivate(WindowRef window)
{
	return !isFloating(window) && layerOfClass(window->windowClass) < 3 &&
		!(window->attributes & kWindowNoActivatesAttribute);
}

// The window that is active when none is chosen: the frontmost visible one that can be active
static WindowRef frontActivatable(void)
{
	WindowRef found = NULL;

	pthread_mutex_lock(&g_lock);
	for (struct OpaqueWindowPtr* w = g_windows; w; w = w->next)
	{
		if (w->visible && canActivate(w))
		{
			found = w;
			break;
		}
	}
	pthread_mutex_unlock(&g_lock);
	return found;
}

static void activateWindow(WindowRef window)
{
	if (!canActivate(window))
		return;

	WindowRef previous = g_activeWindow;
	g_activeWindow = window;
	if (previous && previous != window)
		setActive(previous, false);
	setActive(window, true);
}

#pragma mark CoreGraphics events

static void windowBoundsChanged(WindowRef window, const Rect* now, UInt32 flags)
{
	Rect previous = window->content;

	if (EqualRect(&previous, now))
		return;

	window->content = *now;
#ifndef __LP64__
	if (window->port)
	{
		QDPortSetGlobalOrigin(window->port, now->left, now->top);
		if (now->right - now->left != previous.right - previous.left || now->bottom - now->top != previous.bottom - previous.top)
			_QDResizePort(window->port, now->right - now->left, now->bottom - now->top);
	}
#endif
	if (now->right - now->left != previous.right - previous.left || now->bottom - now->top != previous.bottom - previous.top)
		flags |= kWindowBoundsChangeSizeChanged;
	if (now->left != previous.left || now->top != previous.top)
		flags |= kWindowBoundsChangeOriginChanged;

	Rect newBounds = *now;
	sendWindowEvent(window, kEventWindowBoundsChanged, ^(EventRef event) {
		SetEventParameter(event, kEventParamAttributes, typeUInt32, sizeof(flags), &flags);
		SetEventParameter(event, kEventParamOriginalBounds, typeQDRectangle, sizeof(previous), &previous);
		SetEventParameter(event, kEventParamPreviousBounds, typeQDRectangle, sizeof(previous), &previous);
		SetEventParameter(event, kEventParamCurrentBounds, typeQDRectangle, sizeof(newBounds), &newBounds);
	});

	// A new size needs the content drawn again
	if (flags & kWindowBoundsChangeSizeChanged)
	{
		Rect all = { 0, 0, now->bottom - now->top, now->right - now->left };
		RgnHandle rect = NewRgn();

		RectRgn(rect, &all);
		UnionRgn(window->updateRegion, rect, window->updateRegion);
		DisposeRgn(rect);
		scheduleUpdate(window);
	}
}

static void cgsWindowEvent(CGSWindowEventKind kind, CGSWindowID wid, CGRect rect, void* info)
{
	WindowRef window = windowForCGSWindow(wid);

	if (!window)
		return;

	switch (kind)
	{
		case kCGSWindowEventCloseRequested:
			// Like a click in the close box: the handlers decide (a window with the standard handler closes)
			postWindowEvent(window, kEventWindowClose, NULL);
			break;
		case kCGSWindowEventActivated:
			g_keyWindow = wid;
			if (window->visible && canActivate(window))
			{
				// The window system gave the window the focus: it is the active window, in front
				pthread_mutex_lock(&g_lock);
				unlinkWindow(window);
				linkWindowInFront(window);
				pthread_mutex_unlock(&g_lock);
				activateWindow(window);
			}
			break;
		case kCGSWindowEventDeactivated:
			if (g_keyWindow == wid)
			{
				g_keyWindow = 0;
				// The focus often goes on to another window of the process (a floating window): that is not
				// a deactivation. Once the events that came together are handled, a window system that
				// gave the focus to nobody of ours has deactivated the active window.
				if (!g_focusCheckQueued)
				{
					g_focusCheckQueued = true;
					CFRunLoopPerformBlock(CFRunLoopGetMain(), kCFRunLoopCommonModes, ^{
						g_focusCheckQueued = false;
						if (g_keyWindow == 0 && g_activeWindow && IsValidWindowPtr(g_activeWindow))
							setActive(g_activeWindow, false);
					});
				}
			}
			break;
		case kCGSWindowEventBounds:
		{
			Rect now = { (short) rect.origin.y, (short) rect.origin.x,
				(short) (rect.origin.y + rect.size.height), (short) (rect.origin.x + rect.size.width) };

			windowBoundsChanged(window, &now, 0);
			break;
		}
	}
}

#pragma mark Ports

#ifndef __LP64__
static CFStringRef kQDFlushName = CFSTR("com.darlinghq.QD.PortFlushed");

static CGImageRef createImageOfPort(CGrafPtr port)
{
	PixMapHandle pixMap = GetPortPixMap(port);
	Rect bounds;
	CGImageRef image = NULL;

	if (!pixMap || !LockPixels(pixMap))
		return NULL;
	GetPortBounds(port, &bounds);

	size_t width = bounds.right - bounds.left, height = bounds.bottom - bounds.top;
	size_t rowBytes = GetPixRowBytes(pixMap);
	const void* pixels = GetPixBaseAddr(pixMap);
	CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, pixels, rowBytes * height, NULL);
	CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();

	if (provider && space)
	{
		// The same layout QDBeginCGContext() draws in: the display depth is BGRA, depth 32 ARGB
		bool bgra = (*pixMap)->pixelFormat == 0x42475241; // 'BGRA'
		CGBitmapInfo info = bgra ? (kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst) :
			(kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedFirst);

		image = CGImageCreate(width, height, 8, 32, rowBytes, space, info, provider, NULL, false, kCGRenderingIntentDefault);
	}
	CGColorSpaceRelease(space);
	CGDataProviderRelease(provider);
	UnlockPixels(pixMap);
	return image;
}

// A port was flushed: what it holds for a window goes on the window
static void portFlushed(CFNotificationCenterRef center, void* observer, CFNotificationName name, const void* object, CFDictionaryRef userInfo)
{
	WindowRef window = NULL;

	pthread_mutex_lock(&g_lock);
	for (struct OpaqueWindowPtr* w = g_windows; w; w = w->next)
	{
		if (w->port == object)
		{
			window = w;
			break;
		}
	}
	pthread_mutex_unlock(&g_lock);

	if (!window)
		return;

	CGImageRef image = createImageOfPort((CGrafPtr) object);

	if (!image)
		return;

	CGPoint origin = CGPointZero;
	if (userInfo)
	{
		short edges[4] = { 0 };
		CFStringRef keys[4] = { CFSTR("top"), CFSTR("left"), CFSTR("bottom"), CFSTR("right") };

		for (int i = 0; i < 4; i++)
		{
			CFNumberRef number = CFDictionaryGetValue(userInfo, keys[i]);
			if (number)
				CFNumberGetValue(number, kCFNumberShortType, &edges[i]);
		}
		CGImageRef part = CGImageCreateWithImageInRect(image, CGRectMake(edges[1], edges[0], edges[3] - edges[1], edges[2] - edges[0]));
		CGImageRelease(image);
		image = part;
		origin = CGPointMake(edges[1], edges[0]);
	}
	if (image)
	{
		_CGSWindowDrawImage(connection(), window->cgsWindow, image, origin);
		CGImageRelease(image);
	}
}

static void addPortObserver(void)
{
	CFNotificationCenterAddObserver(CFNotificationCenterGetLocalCenter(), NULL, portFlushed, kQDFlushName, NULL,
		CFNotificationSuspensionBehaviorDeliverImmediately);
}

static void observePorts(void)
{
	static pthread_once_t once = PTHREAD_ONCE_INIT;

	pthread_once(&once, addPortObserver);
}
#endif

#pragma mark Creating windows

static OSStatus styleOfWindow(WindowClass cls, WindowAttributes attributes, uint32_t* style)
{
	uint32_t flags = 0;

	switch (cls)
	{
		case kAlertWindowClass:
		case kMovableAlertWindowClass:
		case kModalWindowClass:
		case kMovableModalWindowClass:
		case kSheetWindowClass:
		case kSheetAlertWindowClass:
		case kDrawerWindowClass:
			flags |= kCGSWindowStyleFixedSize;
			break;
		case kFloatingWindowClass:
		case kUtilityWindowClass:
			flags |= kCGSWindowStyleFloating | kCGSWindowStyleUtility;
			break;
		case kDocumentWindowClass:
			break;
		case kPlainWindowClass:
		case kAltPlainWindowClass:
		case kSimpleWindowClass:
		case kHelpWindowClass:
		case kToolbarWindowClass:
			flags |= kCGSWindowStyleBorderless;
			break;
		case kOverlayWindowClass:
			// Drawn without any frame over the others, transparent where it draws nothing, and pointer
			// input goes through it: the window system does not manage such a window
			flags |= kCGSWindowStyleUnmanaged | kCGSWindowStyleBorderless | kCGSWindowStyleTransparent |
				kCGSWindowStyleNoFocus | kCGSWindowStyleIgnoresMouse | kCGSWindowStyleFloating;
			break;
		default:
			return paramErr;
	}

	if (cls != kOverlayWindowClass && cls != kPlainWindowClass && cls != kAltPlainWindowClass && cls != kSimpleWindowClass &&
		!(attributes & kWindowResizableAttribute))
		flags |= kCGSWindowStyleFixedSize;
	if (!(attributes & kWindowCloseBoxAttribute))
		flags |= kCGSWindowStyleNoClose;
	if (attributes & kWindowNoActivatesAttribute)
		flags |= kCGSWindowStyleNoFocus;
	if (attributes & kWindowNoTitleBarAttribute)
		flags |= kCGSWindowStyleBorderless;
	if (attributes & kWindowIgnoreClicksAttribute)
		flags |= kCGSWindowStyleIgnoresMouse;
	*style = flags;
	return noErr;
}

static const EventTypeSpec kDefaultHandlerTypes[] = {
	{ kEventClassWindow, kEventWindowUpdate },
	{ kEventClassWindow, kEventWindowClose },
};

OSStatus CreateNewWindow(WindowClass cls, WindowAttributes attr, const Rect* rect, WindowRef* newWindow)
{
	if (!newWindow)
		return paramErr;

	*newWindow = NULL;

	if (!rect || rect->right <= rect->left || rect->bottom <= rect->top)
		return paramErr;

	uint32_t style;
	OSStatus status = styleOfWindow(cls, attr, &style);

	if (status != noErr)
		return status;

	if (!g_eventProcRegistered)
	{
		g_eventProcRegistered = true;
		_CGSSetWindowEventProc(cgsWindowEvent, NULL);
	}

	CGRect frame = CGRectMake(rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top);
	CGSRegionRef region = NULL;
	CGSWindowID wid = 0;

	if (CGSNewRegionWithRect(&frame, &region) != kCGErrorSuccess)
		return memFullErr;
	CGError error = CGSNewWindow(connection(), 0, 0, 0, region, &wid);
	CGSRegionRelease(region);
	if (error != kCGErrorSuccess)
		return paramErr;

	WindowRef window = calloc(1, sizeof(*window));

	if (!window)
	{
		CGSReleaseWindow(connection(), wid);
		return memFullErr;
	}
	window->magic = kWindowMagic;
	atomic_init(&window->retainCount, 1);
	window->cgsWindow = wid;
	window->windowClass = cls;
	window->attributes = attr;
	window->content = *rect;
	window->updateRegion = NewRgn();
	window->target = _HICreateEventTarget(GetApplicationEventTarget());

	_CGSWindowSetStyle(connection(), wid, style);

#ifndef __LP64__
	// The window's port holds its pixels: transparent for an overlay, white otherwise
	if (NewGWorld(&window->port, 0, &(Rect) { 0, 0, rect->bottom - rect->top, rect->right - rect->left }, NULL, NULL, 0) == noErr)
	{
		PixMapHandle pixMap = GetPortPixMap(window->port);

		if (cls != kOverlayWindowClass && pixMap && LockPixels(pixMap))
		{
			memset(GetPixBaseAddr(pixMap), 0xFF, GetPixRowBytes(pixMap) * (rect->bottom - rect->top));
			UnlockPixels(pixMap);
		}
		QDPortSetGlobalOrigin(window->port, rect->left, rect->top);
		observePorts();
	}
#endif

	InstallEventHandler(window->target, windowDefaultHandler, sizeof(kDefaultHandlerTypes) / sizeof(kDefaultHandlerTypes[0]),
		kDefaultHandlerTypes, window, NULL);

	pthread_mutex_lock(&g_lock);
	linkWindowInFront(window);
	pthread_mutex_unlock(&g_lock);

	*newWindow = window;
	return noErr;
}

static void destroyWindow(WindowRef window)
{
	pthread_mutex_lock(&g_lock);
	unlinkWindow(window);
	pthread_mutex_unlock(&g_lock);

	if (g_activeWindow == window)
		g_activeWindow = NULL;
	if (g_mouseGrab == window)
		g_mouseGrab = NULL;

	CGSReleaseWindow(connection(), window->cgsWindow);

	while (window->properties)
	{
		struct WindowProperty* property = window->properties;

		window->properties = property->next;
		free(property);
	}
	if (window->title)
		CFRelease(window->title);
	if (window->updateRegion)
		DisposeRgn(window->updateRegion);
#ifndef __LP64__
	if (window->port)
		DisposeGWorld(window->port);
#endif
	_HIDestroyEventTarget(window->target);
	window->magic = 0;
	free(window);
}

void DisposeWindow(WindowRef window)
{
	if (!IsValidWindowPtr(window))
		return;

	HideWindow(window);

	// With the window gone the one behind it is the active one
	bool wasActive = window == g_activeWindow;

	destroyWindow(window);
	if (wasActive)
	{
		WindowRef next = frontActivatable();
		if (next)
			activateWindow(next);
	}
}

OSStatus ReleaseWindow(WindowRef window)
{
	if (!IsValidWindowPtr(window))
		return errInvalidWindowPtr;
	DisposeWindow(window);
	return noErr;
}

Boolean IsValidWindowPtr(WindowRef window)
{
	if (!window)
		return false;

	pthread_mutex_lock(&g_lock);
	bool live = isLive(window);
	pthread_mutex_unlock(&g_lock);
	return live;
}

#pragma mark Showing and order

void ShowWindow(WindowRef window)
{
	if (!IsValidWindowPtr(window) || window->visible)
		return;

	sendWindowEvent(window, kEventWindowShowing, NULL);
	window->visible = true;
	CGSOrderWindow(connection(), window->cgsWindow, kCGSOrderAbove, 0);

	// Everything in it is to be drawn
	Rect all = { 0, 0, window->content.bottom - window->content.top, window->content.right - window->content.left };
	RgnHandle rect = NewRgn();

	RectRgn(rect, &all);
	UnionRgn(window->updateRegion, rect, window->updateRegion);
	DisposeRgn(rect);
	scheduleUpdate(window);
	sendWindowEvent(window, kEventWindowShown, NULL);

	// With no active window the window that appears is it
	if (!g_activeWindow)
		activateWindow(window);
}

void HideWindow(WindowRef window)
{
	if (!IsValidWindowPtr(window) || !window->visible)
		return;

	sendWindowEvent(window, kEventWindowHiding, NULL);
	window->visible = false;
	CGSOrderWindow(connection(), window->cgsWindow, kCGSOrderOut, 0);
	sendWindowEvent(window, kEventWindowHidden, NULL);

	if (window == g_activeWindow)
	{
		g_activeWindow = NULL;
		setActive(window, false);

		WindowRef next = frontActivatable();
		if (next)
			activateWindow(next);
	}
}

void ShowHide(WindowRef window, Boolean showFlag)
{
	if (showFlag)
		ShowWindow(window);
	else
		HideWindow(window);
}

Boolean IsWindowVisible(WindowRef window)
{
	return IsValidWindowPtr(window) && window->visible;
}

OSStatus SelectWindow(WindowRef window)
{
	if (!IsValidWindowPtr(window))
		return errInvalidWindowPtr;

	pthread_mutex_lock(&g_lock);
	unlinkWindow(window);
	linkWindowInFront(window);
	pthread_mutex_unlock(&g_lock);

	if (window->visible)
		CGSOrderWindow(connection(), window->cgsWindow, kCGSOrderAbove, 0);
	activateWindow(window);
	return noErr;
}

void BringToFront(WindowRef window)
{
	if (!IsValidWindowPtr(window))
		return;

	pthread_mutex_lock(&g_lock);
	unlinkWindow(window);
	linkWindowInFront(window);
	pthread_mutex_unlock(&g_lock);
	if (window->visible)
		CGSOrderWindow(connection(), window->cgsWindow, kCGSOrderAbove, 0);
}

void SendBehind(WindowRef window, WindowRef behindWindow)
{
	if (!IsValidWindowPtr(window))
		return;

	pthread_mutex_lock(&g_lock);
	unlinkWindow(window);
	struct OpaqueWindowPtr** p = &g_windows;
	if (behindWindow)
	{
		while (*p && *p != behindWindow)
			p = &(*p)->next;
		if (*p)
			p = &(*p)->next;
	}
	else
	{
		while (*p)
			p = &(*p)->next;
	}
	window->next = *p;
	*p = window;
	pthread_mutex_unlock(&g_lock);
}

WindowRef FrontWindow(void)
{
	WindowRef found = NULL;

	pthread_mutex_lock(&g_lock);
	for (struct OpaqueWindowPtr* w = g_windows; w; w = w->next)
	{
		if (w->visible)
		{
			found = w;
			break;
		}
	}
	pthread_mutex_unlock(&g_lock);
	return found;
}

WindowRef FrontNonFloatingWindow(void)
{
	return frontActivatable();
}

WindowRef ActiveNonFloatingWindow(void)
{
	return g_activeWindow && !isFloating(g_activeWindow) ? g_activeWindow : NULL;
}

WindowRef GetNextWindow(WindowRef window)
{
	WindowRef next = NULL;

	pthread_mutex_lock(&g_lock);
	if (isLive(window))
	{
		for (next = window->next; next && !next->visible; next = next->next)
			;
	}
	pthread_mutex_unlock(&g_lock);
	return next;
}

Boolean IsWindowActive(WindowRef window)
{
	return IsValidWindowPtr(window) && window->active;
}

OSStatus ActivateWindow(WindowRef window, Boolean inActivate)
{
	if (!IsValidWindowPtr(window))
		return errInvalidWindowPtr;

	if (inActivate)
	{
		if (g_activeWindow && g_activeWindow != window && !isFloating(window))
			setActive(g_activeWindow, false);
		if (!isFloating(window))
			g_activeWindow = window;
		setActive(window, true);
	}
	else
	{
		if (g_activeWindow == window)
			g_activeWindow = NULL;
		setActive(window, false);
	}
	return noErr;
}

#pragma mark Class, attributes, title, refcon, properties

OSStatus GetWindowClass(WindowRef window, WindowClass* outClass)
{
	if (!IsValidWindowPtr(window))
		return errInvalidWindowPtr;
	if (!outClass)
		return paramErr;
	*outClass = window->windowClass;
	return noErr;
}

OSStatus GetWindowAttributes(WindowRef window, WindowAttributes* outAttributes)
{
	if (!IsValidWindowPtr(window))
		return errInvalidWindowPtr;
	if (!outAttributes)
		return paramErr;
	*outAttributes = window->attributes;
	return noErr;
}

OSStatus ChangeWindowAttributes(WindowRef window, WindowAttributes setTheseAttributes, WindowAttributes clearTheseAttributes)
{
	if (!IsValidWindowPtr(window))
		return errInvalidWindowPtr;
	if (setTheseAttributes & clearTheseAttributes)
		return paramErr;

	window->attributes = (window->attributes | setTheseAttributes) & ~clearTheseAttributes;

	uint32_t style;
	if (styleOfWindow(window->windowClass, window->attributes, &style) == noErr)
		_CGSWindowSetStyle(connection(), window->cgsWindow, style);
	return noErr;
}

OSStatus SetWindowTitleWithCFString(WindowRef window, CFStringRef title)
{
	if (!IsValidWindowPtr(window))
		return errInvalidWindowPtr;

	CFStringRef copy = title ? CFStringCreateCopy(NULL, title) : NULL;

	if (window->title)
		CFRelease(window->title);
	window->title = copy;
	CGSSetWindowTitle(connection(), window->cgsWindow, copy ? copy : CFSTR(""));
	return noErr;
}

OSStatus CopyWindowTitleAsCFString(WindowRef window, CFStringRef* outString)
{
	if (!IsValidWindowPtr(window))
		return errInvalidWindowPtr;
	if (!outString)
		return paramErr;

	*outString = window->title ? CFRetain(window->title) : CFStringCreateWithCString(NULL, "", kCFStringEncodingUTF8);
	return noErr;
}

long GetWRefCon(WindowRef window)
{
	return IsValidWindowPtr(window) ? window->refCon : 0;
}

OSStatus SetWindowProperty(WindowRef window, OSType propertyCreator, OSType propertyTag, ByteCount propertySize, const void* propertyBuffer)
{
	if (!IsValidWindowPtr(window))
		return errInvalidWindowPtr;
	if (propertySize && !propertyBuffer)
		return paramErr;

	struct WindowProperty* property = malloc(sizeof(*property) + propertySize);

	if (!property)
		return memFullErr;
	property->creator = propertyCreator;
	property->tag = propertyTag;
	property->size = propertySize;
	if (propertySize)
		memcpy(property->data, propertyBuffer, propertySize);

	// A property that is already there is replaced
	for (struct WindowProperty** p = &window->properties; *p; p = &(*p)->next)
	{
		if ((*p)->creator == propertyCreator && (*p)->tag == propertyTag)
		{
			property->next = (*p)->next;
			free(*p);
			*p = property;
			return noErr;
		}
	}
	property->next = window->properties;
	window->properties = property;
	return noErr;
}

static struct WindowProperty* findProperty(WindowRef window, OSType creator, OSType tag)
{
	for (struct WindowProperty* p = window->properties; p; p = p->next)
	{
		if (p->creator == creator && p->tag == tag)
			return p;
	}
	return NULL;
}

OSStatus GetWindowProperty(WindowRef window, OSType propertyCreator, OSType propertyTag, ByteCount bufferSize, ByteCount* actualSize, void* propertyBuffer)
{
	if (!IsValidWindowPtr(window))
		return errInvalidWindowPtr;

	struct WindowProperty* property = findProperty(window, propertyCreator, propertyTag);

	if (!property)
		return errWindowPropertyNotFound;
	if (actualSize)
		*actualSize = property->size;
	if (bufferSize < property->size)
		return buffersTooSmall;
	if (property->size)
	{
		if (!propertyBuffer)
			return paramErr;
		memcpy(propertyBuffer, property->data, property->size);
	}
	return noErr;
}

OSStatus GetWindowPropertySize(WindowRef window, OSType propertyCreator, OSType propertyTag, ByteCount* size)
{
	if (!IsValidWindowPtr(window))
		return errInvalidWindowPtr;

	struct WindowProperty* property = findProperty(window, propertyCreator, propertyTag);

	if (!property)
		return errWindowPropertyNotFound;
	if (size)
		*size = property->size;
	return noErr;
}

OSStatus RemoveWindowProperty(WindowRef window, OSType propertyCreator, OSType propertyTag)
{
	if (!IsValidWindowPtr(window))
		return errInvalidWindowPtr;

	for (struct WindowProperty** p = &window->properties; *p; p = &(*p)->next)
	{
		if ((*p)->creator == propertyCreator && (*p)->tag == propertyTag)
		{
			struct WindowProperty* property = *p;

			*p = property->next;
			free(property);
			return noErr;
		}
	}
	return errWindowPropertyNotFound;
}

#pragma mark Position and size

// The structure region is the content with the title bar the window system adds; its height is
// that of a standard title bar, and none for the windows that have no frame
#define kTitleBarHeight 22

static bool hasTitleBar(WindowRef window)
{
	switch (window->windowClass)
	{
		case kDocumentWindowClass:
		case kFloatingWindowClass:
		case kUtilityWindowClass:
		case kModalWindowClass:
		case kMovableModalWindowClass:
		case kAlertWindowClass:
		case kMovableAlertWindowClass:
			return !(window->attributes & kWindowNoTitleBarAttribute);
		default:
			return false;
	}
}

OSStatus GetWindowBounds(WindowRef window, WindowRegionCode regionCode, Rect* globalBounds)
{
	if (!IsValidWindowPtr(window))
		return errInvalidWindowPtr;
	if (!globalBounds)
		return paramErr;

	switch (regionCode)
	{
		case kWindowContentRgn:
		case kWindowGlobalPortRgn:
			*globalBounds = window->content;
			return noErr;
		case kWindowStructureRgn:
			*globalBounds = window->content;
			if (hasTitleBar(window))
				globalBounds->top -= kTitleBarHeight;
			return noErr;
		case kWindowTitleBarRgn:
			if (!hasTitleBar(window))
				return errWindowRegionCodeInvalid;
			*globalBounds = (Rect) { window->content.top - kTitleBarHeight, window->content.left, window->content.top, window->content.right };
			return noErr;
		default:
			return errWindowRegionCodeInvalid;
	}
}

static OSStatus applyBounds(WindowRef window, const Rect* content)
{
	if (content->right <= content->left || content->bottom <= content->top)
		return paramErr;

	CGRect frame = CGRectMake(content->left, content->top, content->right - content->left, content->bottom - content->top);
	CGSRegionRef region = NULL;

	if (CGSNewRegionWithRect(&frame, &region) != kCGErrorSuccess)
		return memFullErr;
	CGError error = CGSSetWindowShape(connection(), window->cgsWindow, 0, 0, region);
	CGSRegionRelease(region);

	Rect now = *content;

	if (error == kCGErrorNotImplemented)
	{
		// A window system that does not let a program move a window (Wayland) keeps the place it gave
		// the window: the size is asked for there, and that place is what the program sees afterwards
		CGRect realized;

		if (CGSGetWindowBounds(connection(), window->cgsWindow, &realized) == kCGErrorSuccess)
		{
			frame = CGRectMake(realized.origin.x, realized.origin.y, frame.size.width, frame.size.height);
			if (CGSNewRegionWithRect(&frame, &region) == kCGErrorSuccess)
			{
				CGSSetWindowShape(connection(), window->cgsWindow, 0, 0, region);
				CGSRegionRelease(region);
			}
			now = (Rect) { (short) frame.origin.y, (short) frame.origin.x,
				(short) (frame.origin.y + frame.size.height), (short) (frame.origin.x + frame.size.width) };
		}
	}

	// Programmatic changes are reported at once, as the Window Manager does; the window system's own
	// report of the same change is then not a change any more
	windowBoundsChanged(window, &now, 0);
	return noErr;
}

OSStatus SetWindowBounds(WindowRef window, WindowRegionCode regionCode, const Rect* globalBounds)
{
	if (!IsValidWindowPtr(window))
		return errInvalidWindowPtr;
	if (!globalBounds)
		return paramErr;

	Rect content = *globalBounds;

	switch (regionCode)
	{
		case kWindowContentRgn:
		case kWindowGlobalPortRgn:
			break;
		case kWindowStructureRgn:
			if (hasTitleBar(window))
				content.top += kTitleBarHeight;
			break;
		default:
			return errWindowRegionCodeInvalid;
	}
	return applyBounds(window, &content);
}

OSStatus MoveWindowStructure(WindowRef window, SInt16 hGlobal, SInt16 vGlobal)
{
	if (!IsValidWindowPtr(window))
		return errInvalidWindowPtr;

	Rect content = window->content;

	OffsetRect(&content, hGlobal - content.left, vGlobal + (hasTitleBar(window) ? kTitleBarHeight : 0) - content.top);
	return applyBounds(window, &content);
}

void MoveWindow(WindowRef window, short hGlobal, short vGlobal, Boolean front)
{
	if (!IsValidWindowPtr(window))
		return;

	Rect content = window->content;

	OffsetRect(&content, hGlobal - content.left, vGlobal - content.top);
	applyBounds(window, &content);
	if (front)
		SelectWindow(window);
}

void SizeWindow(WindowRef window, short w, short h, Boolean fUpdate)
{
	if (!IsValidWindowPtr(window))
		return;

	Rect content = window->content;

	content.right = content.left + w;
	content.bottom = content.top + h;
	applyBounds(window, &content);
}

OSStatus RepositionWindow(WindowRef window, WindowRef parentWindow, WindowPositionMethod method)
{
	if (!IsValidWindowPtr(window))
		return errInvalidWindowPtr;

	CGDirectDisplayID display = CGMainDisplayID();
	CGRect screen = CGDisplayBounds(display);
	Rect content = window->content;
	short width = content.right - content.left, height = content.bottom - content.top;
	short left, top;

	switch (method)
	{
		case kWindowCenterOnMainScreen:
		case kWindowCenterOnParentWindow:
		case kWindowCenterOnParentWindowScreen:
			left = (short) (screen.origin.x + (screen.size.width - width) / 2);
			top = (short) (screen.origin.y + (screen.size.height - height) / 2);
			break;
		case kWindowCascadeOnMainScreen:
		case kWindowCascadeOnParentWindow:
		case kWindowCascadeOnParentWindowScreen:
		case kWindowCascadeStartAtParentWindowScreen:
			left = (short) (screen.origin.x + 40);
			top = (short) (screen.origin.y + 60);
			break;
		case kWindowAlertPositionOnMainScreen:
		case kWindowAlertPositionOnParentWindow:
		case kWindowAlertPositionOnParentWindowScreen:
			left = (short) (screen.origin.x + (screen.size.width - width) / 2);
			top = (short) (screen.origin.y + (screen.size.height - height) / 3);
			break;
		default:
			return paramErr;
	}
	OffsetRect(&content, left - content.left, top - content.top);
	return applyBounds(window, &content);
}

Rect* GetWindowPortBounds(WindowRef window, Rect* bounds)
{
	if (!bounds)
		return NULL;
	if (!IsValidWindowPtr(window))
	{
		SetRect(bounds, 0, 0, 0, 0);
		return bounds;
	}
	SetRect(bounds, 0, 0, window->content.right - window->content.left, window->content.bottom - window->content.top);
	return bounds;
}

#pragma mark Event target

EventTargetRef GetWindowEventTarget(WindowRef window)
{
	return IsValidWindowPtr(window) ? window->target : NULL;
}

// The default handling of events for which a window's handlers did nothing
static OSStatus windowDefaultHandler(EventHandlerCallRef call, EventRef event, void* userData)
{
	WindowRef window = userData;

	if (!IsValidWindowPtr(window))
		return eventNotHandledErr;

	switch (GetEventKind(event))
	{
		case kEventWindowUpdate:
			window->updatePending = false;
#ifndef __LP64__
			BeginUpdate(window);
#endif
			// The drawing is asked of the handlers
			sendWindowEvent(window, kEventWindowDrawContent, NULL);
#ifndef __LP64__
			EndUpdate(window);
#endif
			return noErr;
		case kEventWindowClose:
			// A window with the standard handler closes itself
			if (window->attributes & kWindowStandardHandlerAttribute)
			{
				sendWindowEvent(window, kEventWindowClosed, NULL);
				DisposeWindow(window);
				return noErr;
			}
			return eventNotHandledErr;
	}
	return eventNotHandledErr;
}

#pragma mark Invalidation (32-bit)

#ifndef __LP64__
OSStatus InvalWindowRect(WindowRef window, const Rect* bounds)
{
	if (!IsValidWindowPtr(window))
		return errInvalidWindowPtr;
	if (!bounds)
		return paramErr;

	RgnHandle rect = NewRgn();

	RectRgn(rect, bounds);
	UnionRgn(window->updateRegion, rect, window->updateRegion);
	DisposeRgn(rect);
	scheduleUpdate(window);
	return noErr;
}

OSStatus InvalWindowRgn(WindowRef window, RgnHandle region)
{
	if (!IsValidWindowPtr(window))
		return errInvalidWindowPtr;
	if (!region)
		return paramErr;

	UnionRgn(window->updateRegion, region, window->updateRegion);
	scheduleUpdate(window);
	return noErr;
}

OSStatus ValidWindowRect(WindowRef window, const Rect* bounds)
{
	if (!IsValidWindowPtr(window))
		return errInvalidWindowPtr;
	if (!bounds)
		return paramErr;

	RgnHandle rect = NewRgn();

	RectRgn(rect, bounds);
	DiffRgn(window->updateRegion, rect, window->updateRegion);
	DisposeRgn(rect);
	return noErr;
}

OSStatus ValidWindowRgn(WindowRef window, RgnHandle region)
{
	if (!IsValidWindowPtr(window))
		return errInvalidWindowPtr;
	if (!region)
		return paramErr;

	DiffRgn(window->updateRegion, region, window->updateRegion);
	return noErr;
}

void BeginUpdate(WindowRef window)
{
	if (IsValidWindowPtr(window))
		window->inUpdate = true;
}

void EndUpdate(WindowRef window)
{
	if (!IsValidWindowPtr(window))
		return;
	// What the handlers drew is what was asked for: nothing is left to update
	window->inUpdate = false;
	SetEmptyRgn(window->updateRegion);
}

CGrafPtr GetWindowPort(WindowRef window)
{
	return IsValidWindowPtr(window) ? window->port : NULL;
}

WindowRef GetWindowFromPort(CGrafPtr port)
{
	WindowRef found = NULL;

	if (!port)
		return NULL;

	pthread_mutex_lock(&g_lock);
	for (struct OpaqueWindowPtr* w = g_windows; w; w = w->next)
	{
		if (w->port == port)
		{
			found = w;
			break;
		}
	}
	pthread_mutex_unlock(&g_lock);
	return found;
}

void SetPortWindowPort(WindowRef window)
{
	if (IsValidWindowPtr(window) && window->port)
		SetPort(window->port);
}

void SetWRefCon(WindowRef window, long data)
{
	if (IsValidWindowPtr(window))
		window->refCon = data;
}
#endif

#pragma mark Mouse events

// The events of the mouse go to the window they are for: that of the pointer, and from a mouse down to
// the next mouse up that of the mouse down. A click in a window makes it the active one.
const EventTypeSpec _HIStandardWindowEventTypes[kHIStandardWindowEventTypeCount] = {
	{ kEventClassMouse, kEventMouseDown },
	{ kEventClassMouse, kEventMouseUp },
	{ kEventClassMouse, kEventMouseDragged },
	{ kEventClassMouse, kEventMouseMoved },
	{ kEventClassMouse, kEventMouseWheelMoved },
	{ kEventClassKeyboard, kEventRawKeyDown },
	{ kEventClassKeyboard, kEventRawKeyRepeat },
	{ kEventClassKeyboard, kEventRawKeyUp },
	{ kEventClassKeyboard, kEventRawKeyModifiersChanged },
	{ kEventClassWindow, kEventWindowUpdate },
	{ kEventClassWindow, kEventWindowDrawContent },
	{ kEventClassWindow, kEventWindowClose },
	{ kEventClassWindow, kEventWindowClosed },
	{ kEventClassWindow, kEventWindowActivated },
	{ kEventClassWindow, kEventWindowDeactivated },
	{ kEventClassWindow, kEventWindowBoundsChanged },
	{ kEventClassWindow, kEventWindowShown },
	{ kEventClassWindow, kEventWindowHidden },
};

OSStatus _HIStandardWindowEventHandler(EventHandlerCallRef call, EventRef event, void* userData)
{
	OSType eventClass = GetEventClass(event);
	UInt32 kind = GetEventKind(event);

	// The events that are made for a window (they name it)
	if (eventClass == kEventClassWindow)
	{
		WindowRef window = NULL;

		if (GetEventParameter(event, kEventParamDirectObject, typeWindowRef, NULL, sizeof(window), NULL, &window) != noErr ||
			!IsValidWindowPtr(window))
			return eventNotHandledErr;
		return SendEventToEventTargetWithOptions(event, window->target, kEventTargetDontPropagate);
	}

	if (eventClass == kEventClassKeyboard)
	{
		if (!g_activeWindow)
			return eventNotHandledErr;
		return SendEventToEventTargetWithOptions(event, g_activeWindow->target, kEventTargetDontPropagate);
	}

	// The mouse
	CGEventRef cgEvent = CopyEventCGEvent(event);

	if (!cgEvent)
		return eventNotHandledErr;

	WindowRef window = windowForCGSWindow((CGSWindowID) CGEventGetIntegerValueField(cgEvent, kCGMouseEventWindowUnderMousePointer));
	CGPoint location = CGEventGetLocation(cgEvent);
	CFRelease(cgEvent);

	if (kind != kEventMouseDown && g_mouseGrab && IsValidWindowPtr(g_mouseGrab))
		window = g_mouseGrab;
	if (!window)
		return eventNotHandledErr;

	if (kind == kEventMouseDown)
	{
		g_mouseGrab = window;
		if (!window->active && canActivate(window) && !(window->attributes & kWindowIgnoreClicksAttribute))
			SelectWindow(window);
	}
	else if (kind == kEventMouseUp)
		g_mouseGrab = NULL;

	HIPoint inWindow = { location.x - window->content.left, location.y - window->content.top };
	UInt32 part = inContent;

	SetEventParameter(event, kEventParamWindowRef, typeWindowRef, sizeof(window), &window);
	SetEventParameter(event, kEventParamWindowPartCode, typeWindowPartCode, sizeof(part), &part);
	SetEventParameter(event, kEventParamWindowMouseLocation, typeHIPoint, sizeof(inWindow), &inWindow);

	// Only the window's own handlers: the application's come next in the dispatcher's chain
	OSStatus result = SendEventToEventTargetWithOptions(event, window->target, kEventTargetDontPropagate);

	if (result == eventNotHandledErr && kind == kEventMouseDown)
	{
		// Nobody wanted the click: it is a click in the content region, for the handlers of that
		Point global = { (short) location.y, (short) location.x };
		UInt32 modifiers = 0;
		EventMouseButton button = kEventMouseButtonPrimary;
		UInt32 clicks = 1;

		GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(modifiers), NULL, &modifiers);
		GetEventParameter(event, kEventParamMouseButton, typeMouseButton, NULL, sizeof(button), NULL, &button);
		GetEventParameter(event, kEventParamClickCount, typeUInt32, NULL, sizeof(clicks), NULL, &clicks);

		result = sendWindowEvent(window, kEventWindowClickContentRgn, ^(EventRef click) {
			SetEventParameter(click, kEventParamMouseLocation, typeQDPoint, sizeof(global), &global);
			SetEventParameter(click, kEventParamKeyModifiers, typeUInt32, sizeof(modifiers), &modifiers);
			SetEventParameter(click, kEventParamMouseButton, typeMouseButton, sizeof(button), &button);
			SetEventParameter(click, kEventParamClickCount, typeUInt32, sizeof(clicks), &clicks);
		});
	}
	return result;
}

#pragma mark Other

OSStatus SetWindowContentColor(WindowRef window, const RGBColor* color)
{
	if (!IsValidWindowPtr(window))
		return errInvalidWindowPtr;
	return noErr;
}

OSStatus SetWindowGroup(WindowRef a, WindowGroupRef b)
{
	return noErr;
}

OSStatus SetWindowGroupLevel(WindowGroupRef a, SInt32 b)
{
	return noErr;
}

OSStatus TransitionWindowWithOptions(WindowRef a, WindowTransitionEffect b, WindowTransitionAction c, const HIRect* d, Boolean e, TransitionWindowOptions* f)
{
	return noErr;
}

OSStatus CreateWindowGroup(WindowGroupAttributes a, WindowGroupRef* b)
{
	if (b)
		*b = NULL;
	return noErr;
}

OSStatus GetWindowGroupLevel(WindowGroupRef a, SInt32* b)
{
	if (b)
		*b = 0;
	return noErr;
}

WindowGroupRef GetWindowGroupOfClass(WindowClass a)
{
	return NULL;
}

// For the code that needs the CoreGraphics window of a Carbon window
CGSWindowID _HIWindowGetCGSWindowID(WindowRef window)
{
	return IsValidWindowPtr(window) ? window->cgsWindow : 0;
}
