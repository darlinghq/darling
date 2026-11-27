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

#ifndef MACWINDOWS_H
#define MACWINDOWS_H

#include <CarbonCore/MacErrors.h>
#include <HIToolbox/HIGeometry.h>
#include <HIToolbox/CarbonEventsCore.h>
#include "CoreServices/MacTypes.h"
#include <CoreFoundation/CoreFoundation.h>
#include <stdint.h>
#include <QD/QD.h>

#pragma pack(push, 2)

#ifdef __cplusplus
extern "C" {
#endif

typedef WindowRef HIWindowRef;
typedef struct OpaqueWindowGroupRef* WindowGroupRef;

typedef UInt32 WindowClass;
typedef UInt32 WindowAttributes;
typedef UInt32 WindowGroupAttributes;
typedef UInt16 WindowRegionCode;
typedef UInt32 WindowModality;
typedef UInt32 WindowPositionMethod;
typedef UInt32 WindowActivationScope;
typedef UInt32 WindowPartCode;

typedef void* TransitionWindowOptions;
typedef uint32_t WindowTransitionAction;
typedef uint32_t WindowTransitionEffect;

enum {
	kAlertWindowClass = 1,
	kMovableAlertWindowClass = 2,
	kModalWindowClass = 3,
	kMovableModalWindowClass = 4,
	kFloatingWindowClass = 5,
	kDocumentWindowClass = 6,
	kUtilityWindowClass = 8,
	kHelpWindowClass = 10,
	kSheetWindowClass = 11,
	kToolbarWindowClass = 12,
	kPlainWindowClass = 13,
	kOverlayWindowClass = 14,
	kSheetAlertWindowClass = 15,
	kAltPlainWindowClass = 16,
	kSimpleWindowClass = 18,
	kDrawerWindowClass = 20,
	kAllWindowClasses = 0xFFFFFFFFu
};

enum {
	kWindowNoAttributes = 0,
	kWindowCloseBoxAttribute = 1u << 0,
	kWindowHorizontalZoomAttribute = 1u << 1,
	kWindowVerticalZoomAttribute = 1u << 2,
	kWindowFullZoomAttribute = kWindowHorizontalZoomAttribute | kWindowVerticalZoomAttribute,
	kWindowCollapseBoxAttribute = 1u << 3,
	kWindowResizableAttribute = 1u << 4,
	kWindowSideTitlebarAttribute = 1u << 5,
	kWindowToolbarAttribute = 1u << 6,
	kWindowMetalAttribute = 1u << 8,
	kWindowNoTitleBarAttribute = 1u << 9,
	kWindowTexturedSquareCornersAttribute = 1u << 10,
	kWindowMetalNoContentSeparatorAttribute = 1u << 11,
	kWindowDoesNotCycleAttribute = 1u << 15,
	kWindowNoUpdatesAttribute = 1u << 16,
	kWindowNoActivatesAttribute = 1u << 17,
	kWindowOpaqueForEventsAttribute = 1u << 18,
	kWindowCompositingAttribute = 1u << 19,
	kWindowNoShadowAttribute = 1u << 21,
	kWindowHideOnSuspendAttribute = 1u << 24,
	kWindowStandardHandlerAttribute = 1u << 25,
	kWindowHideOnFullScreenAttribute = 1u << 26,
	kWindowInWindowMenuAttribute = 1u << 27,
	kWindowLiveResizeAttribute = 1u << 28,
	kWindowIgnoreClicksAttribute = 1u << 29,
	kWindowNoConstrainAttribute = 1u << 31,
	kWindowStandardDocumentAttributes = kWindowCloseBoxAttribute | kWindowFullZoomAttribute | kWindowCollapseBoxAttribute | kWindowResizableAttribute,
	kWindowStandardFloatingAttributes = kWindowCloseBoxAttribute | kWindowCollapseBoxAttribute
};

// Window regions (GetWindowBounds() and others)
enum {
	kWindowTitleBarRgn = 0,
	kWindowTitleTextRgn = 1,
	kWindowCloseBoxRgn = 2,
	kWindowZoomBoxRgn = 3,
	kWindowDragRgn = 5,
	kWindowGrowRgn = 6,
	kWindowCollapseBoxRgn = 7,
	kWindowTitleProxyIconRgn = 8,
	kWindowStructureRgn = 32,
	kWindowContentRgn = 33,
	kWindowUpdateRgn = 34,
	kWindowOpaqueRgn = 35,
	kWindowGlobalPortRgn = 40,
	kWindowToolbarButtonRgn = 41
};

// Window parts (FindWindow() and the click events)
enum {
	inDesk = 0,
	inNoWindow = 0,
	inMenuBar = 1,
	inSysWindow = 2,
	inContent = 3,
	inDrag = 4,
	inGrow = 5,
	inGoAway = 6,
	inZoomIn = 7,
	inZoomOut = 8,
	inCollapseBox = 11,
	inProxyIcon = 12,
	inToolbarButton = 13,
	inStructure = 15
};

enum {
	kWindowModalityNone = 0,
	kWindowModalitySystemModal = 1,
	kWindowModalityAppModal = 2,
	kWindowModalityWindowModal = 3
};

enum {
	kWindowCenterOnMainScreen = 1,
	kWindowCenterOnParentWindow = 2,
	kWindowCenterOnParentWindowScreen = 3,
	kWindowCascadeOnMainScreen = 4,
	kWindowCascadeOnParentWindow = 5,
	kWindowCascadeOnParentWindowScreen = 6,
	kWindowCascadeStartAtParentWindowScreen = 10,
	kWindowAlertPositionOnMainScreen = 7,
	kWindowAlertPositionOnParentWindow = 8,
	kWindowAlertPositionOnParentWindowScreen = 9
};

enum {
	kWindowActivationScopeNone = 0,
	kWindowActivationScopeIndependent = 1,
	kWindowActivationScopeAll = 2
};

enum {
	kFirstWindowOfClass = -1,
	kLastWindowOfClass = 0
};

enum {
	kWindowDefProcType = 'WDEF'
};

// Windows and their properties
OSStatus CreateNewWindow(WindowClass cls, WindowAttributes attr, const Rect* rect, WindowRef* newWindow);
void DisposeWindow(WindowRef wnd);
OSStatus ReleaseWindow(WindowRef a);
Boolean IsValidWindowPtr(WindowRef a);

void ShowWindow(WindowRef wnd);
void HideWindow(WindowRef wnd);
void ShowHide(WindowRef wnd, Boolean showFlag);
Boolean IsWindowVisible(WindowRef wnd);

void SelectWindow(WindowRef a);
OSStatus CollapseWindow(WindowRef window, Boolean collapse);
Boolean IsWindowCollapsed(WindowRef window);
OSStatus CollapseAllWindows(Boolean collapse);
void BringToFront(WindowRef window);
void SendBehind(WindowRef window, WindowRef behindWindow);
WindowRef FrontWindow(void);

// The window that gets the keyboard events: by default (kUserFocusAuto) the active window
#define kUserFocusAuto ((WindowRef) -1)
OSStatus SetUserFocusWindow(WindowRef inWindow);
WindowRef GetUserFocusWindow(void);
WindowRef FrontNonFloatingWindow(void);
WindowRef ActiveNonFloatingWindow(void);
WindowRef GetNextWindow(WindowRef window);
Boolean IsWindowActive(WindowRef window);
OSStatus ActivateWindow(WindowRef window, Boolean inActivate);

OSStatus GetWindowClass(WindowRef window, WindowClass* outClass);
OSStatus GetWindowAttributes(WindowRef window, WindowAttributes* outAttributes);
OSStatus ChangeWindowAttributes(WindowRef window, WindowAttributes setTheseAttributes, WindowAttributes clearTheseAttributes);

OSStatus SetWindowTitleWithCFString(WindowRef a, CFStringRef b);
OSStatus CopyWindowTitleAsCFString(WindowRef inWindow, CFStringRef* outString);

OSStatus SetWindowProperty(WindowRef window, OSType propertyCreator, OSType propertyTag, ByteCount propertySize, const void* propertyBuffer);
OSStatus GetWindowProperty(WindowRef window, OSType propertyCreator, OSType propertyTag, ByteCount bufferSize, ByteCount* actualSize, void* propertyBuffer);
OSStatus GetWindowPropertySize(WindowRef window, OSType propertyCreator, OSType propertyTag, ByteCount* size);
OSStatus RemoveWindowProperty(WindowRef window, OSType propertyCreator, OSType propertyTag);

long GetWRefCon(WindowRef window);

// Position and size, in global (screen) coordinates
OSStatus GetWindowBounds(WindowRef a, WindowRegionCode b, Rect * c);
OSStatus SetWindowBounds(WindowRef window, WindowRegionCode regionCode, const Rect* globalBounds);
OSStatus MoveWindowStructure(WindowRef window, SInt16 hGlobal, SInt16 vGlobal);
OSStatus RepositionWindow(WindowRef window, WindowRef parentWindow, WindowPositionMethod method);
void MoveWindow(WindowRef window, short hGlobal, short vGlobal, Boolean front);
void SizeWindow(WindowRef window, short w, short h, Boolean fUpdate);
OSStatus GetWindowGreatestAreaDevice(WindowRef inWindow, WindowRegionCode inRegion, void* outGreatestDevice, Rect* outGreatestDeviceRect);

Rect* GetWindowPortBounds(WindowRef window, Rect * bounds);

// Window groups
OSStatus SetWindowContentColor(WindowRef a, const RGBColor * b);
OSStatus SetWindowGroup(WindowRef a, WindowGroupRef b);
OSStatus SetWindowGroupLevel(WindowGroupRef a, SInt32 b);
OSStatus GetWindowGroupLevel(WindowGroupRef a, SInt32 * b);
OSStatus CreateWindowGroup(WindowGroupAttributes a, WindowGroupRef *b);
WindowGroupRef GetWindowGroupOfClass(WindowClass a);
OSStatus TransitionWindowWithOptions(WindowRef a, WindowTransitionEffect b, WindowTransitionAction c, const HIRect * d, Boolean e, TransitionWindowOptions * f);

// Invalidation. The window's update region collects what must be redrawn; the redraw is requested
// of the window's event handlers (kEventWindowDrawContent, kEventWindowUpdate).
#ifndef __LP64__
OSStatus InvalWindowRect(WindowRef window, const Rect* bounds);
OSStatus InvalWindowRgn(WindowRef window, RgnHandle region);
OSStatus ValidWindowRect(WindowRef window, const Rect* bounds);
OSStatus ValidWindowRgn(WindowRef window, RgnHandle region);

// A window's port (32-bit only)
CGrafPtr GetWindowPort(WindowRef a);
WindowRef GetWindowFromPort(CGrafPtr port);
void SetPortWindowPort(WindowRef window);
void SetWRefCon(WindowRef window, long data);
OSStatus SetThemeWindowBackground(WindowRef window, SInt16 brush, Boolean update);
void BeginUpdate(WindowRef window);
void EndUpdate(WindowRef window);
#endif

#ifdef __cplusplus
}
#endif

#pragma pack(pop)

#endif
