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

// Windows from nibs. The nib reader gives the description of the IBCarbonWindow object with its
// controls (see IBCarbonNib.h); making the window and the controls out of it is up to the Window
// Manager and the Control Manager, through _HICreateWindowFromNibDescription(): the Window Manager makes
// the window, the Control Manager makes its controls.

#include <HIToolbox/IBCarbonRuntime.h>
#include <HIToolbox/MacWindows.h>
#include <stdio.h>
#include <CarbonCore/MacErrors.h>
#include "IBCarbonNib.h"
#include "ControlsInternal.h"

// The attributes of a nib window object: a boolean that is not in the nib is false
static bool nibFlag(CFDictionaryRef description, CFStringRef key, bool absent)
{
	CFTypeRef value = CFDictionaryGetValue(description, key);

	return value && CFGetTypeID(value) == CFBooleanGetTypeID() ? CFBooleanGetValue(value) : absent;
}

static bool nibInt(CFDictionaryRef description, CFStringRef key, int* out)
{
	CFTypeRef value = CFDictionaryGetValue(description, key);

	return value && CFGetTypeID(value) == CFNumberGetTypeID() && CFNumberGetValue(value, kCFNumberIntType, out);
}

// The window of the description: its class, attributes, bounds and title. The controls of the window
// are made by the Control Manager.
OSStatus _HICreateWindowFromNibDescription(CFDictionaryRef description, WindowRef* outWindow)
{
	*outWindow = NULL;

	// "windowRect" is the content rectangle, "top left bottom right"
	CFStringRef rectString = CFDictionaryGetValue(description, CFSTR("windowRect"));
	char text[128];
	int top, left, bottom, right;

	if (!rectString || CFGetTypeID(rectString) != CFStringGetTypeID() ||
		!CFStringGetCString(rectString, text, sizeof(text), kCFStringEncodingUTF8) ||
		sscanf(text, "%d %d %d %d", &top, &left, &bottom, &right) != 4)
		return paramErr;

	int windowClass = kDocumentWindowClass;
	nibInt(description, CFSTR("carbonWindowClass"), &windowClass);

	WindowAttributes attributes = kWindowNoAttributes;

	if (nibFlag(description, CFSTR("hasCloseBox"), false))
		attributes |= kWindowCloseBoxAttribute;
	if (nibFlag(description, CFSTR("hasCollapseBox"), false))
		attributes |= kWindowCollapseBoxAttribute;
	if (nibFlag(description, CFSTR("hasHorizontalZoom"), false))
		attributes |= kWindowHorizontalZoomAttribute;
	if (nibFlag(description, CFSTR("hasVerticalZoom"), false))
		attributes |= kWindowVerticalZoomAttribute;
	if (nibFlag(description, CFSTR("isResizable"), false))
		attributes |= kWindowResizableAttribute;
	if (nibFlag(description, CFSTR("liveResize"), false))
		attributes |= kWindowLiveResizeAttribute;
	if (nibFlag(description, CFSTR("hideOnSuspend"), false))
		attributes |= kWindowHideOnSuspendAttribute;
	if (nibFlag(description, CFSTR("hideOnFullScreen"), false))
		attributes |= kWindowHideOnFullScreenAttribute;
	if (!nibFlag(description, CFSTR("isConstrained"), true))
		attributes |= kWindowNoConstrainAttribute;
	if (!nibFlag(description, CFSTR("receiveUpdates"), true))
		attributes |= kWindowNoUpdatesAttribute;
	// The windows of a nib have the standard window handler unless the nib says otherwise
	if (nibFlag(description, CFSTR("standardHandler"), true))
		attributes |= kWindowStandardHandlerAttribute;

	// The windows of a nib are compositing windows when the nib says so; in 64-bit they all are
	if (nibFlag(description, CFSTR("compositing"), false))
		attributes |= kWindowCompositingAttribute;
#ifdef __LP64__
	attributes |= kWindowCompositingAttribute;
#endif
	Rect bounds = { (short) top, (short) left, (short) bottom, (short) right };
	WindowRef window;
	OSStatus status = CreateNewWindow((WindowClass) windowClass, attributes, &bounds, &window);

	if (status != noErr)
		return status;

	CFStringRef title = CFDictionaryGetValue(description, CFSTR("title"));

	if (title && CFGetTypeID(title) == CFStringGetTypeID())
		SetWindowTitleWithCFString(window, title);

	int brush;

	if (nibInt(description, CFSTR("themeBrush"), &brush))
		_HIWindowSetBackgroundBrush(window, (SInt16) brush);

	// The controls of the window are those below its root control
	CFDictionaryRef rootControl = CFDictionaryGetValue(description, CFSTR("rootControl"));

	if (rootControl && CFGetTypeID(rootControl) == CFDictionaryGetTypeID())
		_HICreateControlsFromNibDescription(rootControl, window);
	*outWindow = window;
	return noErr;
}

OSStatus CreateWindowFromNib(IBNibRef inNibRef, CFStringRef inName, WindowRef* outWindow)
{
	if (!inNibRef || !inName || !outWindow)
		return paramErr;
	*outWindow = NULL;

	CFDictionaryRef description = _IBCarbonNibCopyObjectNamed(inNibRef, inName);

	if (!description)
		return kIBCarbonRuntimeCantFindObject;

	CFStringRef className = CFDictionaryGetValue(description, CFSTR("class"));
	OSStatus status;

	if (!className || CFStringCompare(className, CFSTR("IBCarbonWindow"), 0) != kCFCompareEqualTo)
		status = kIBCarbonRuntimeObjectNotOfRequestedType;
	else
		status = _HICreateWindowFromNibDescription(description, outWindow);
	CFRelease(description);
	return status;
}
