/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

#ifndef _Carbon_Carbon_Events_H_
#define _Carbon_Carbon_Events_H_

#include <CoreFoundation/CFBase.h>
#include <CoreServices/MacTypes.h>
#include <stdbool.h>
#include <HIToolbox/CarbonEventsCore.h>
#include <HIToolbox/HIObject.h>
#include <QD/QD.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct EventHotKeyID {
    OSType signature;
    UInt32 id;
} EventHotKeyID;
typedef struct OpaqueEventHotKeyRef *EventHotKeyRef;

enum {
	kEventHotKeyPressed = 5,
	kEventHotKeyReleased = 6,
};

enum {
	typeEventHotKeyID = 'hkid',
};

enum {
	kEventHotKeyNoOptions = 0,
	kEventHotKeyExclusive = 1 << 0,
};

EventTargetRef GetApplicationEventTarget(void);

#define InstallApplicationEventHandler(handler, numTypes, typeList, userData, outHandlerRef) \
	InstallEventHandler(GetApplicationEventTarget(), (handler), (numTypes), (typeList), (userData), (outHandlerRef))
EventTargetRef GetEventDispatcherTarget(void);
EventTargetRef GetEventMonitorTarget(void);

// The event target of the user focus: the window that has it, or the application
EventTargetRef GetUserFocusEventTarget(void);

OSStatus ProcessHICommand(const HICommand * a);

// Runs the event loop of the application: events of the main queue are sent to the event
// dispatcher target until QuitApplicationEventLoop() is called or a quit command is handled.
void RunApplicationEventLoop(void);
void QuitApplicationEventLoop(void);

OSStatus RegisterEventHotKey(UInt32 inHotKeyCode, UInt32 inHotKeyModifiers, EventHotKeyID inHotKeyID, EventTargetRef inTarget, OptionBits inOptions, EventHotKeyRef *outRef);
OSStatus UnregisterEventHotKey(EventHotKeyRef inHotKey);

// Window and control events (class, kinds, parameters and parameter types)
enum {
	kEventClassControl = 'cntl',
	kEventClassWindow = 'wind',
};

enum {
	kEventWindowUpdate = 1,
	kEventWindowDrawContent = 2,
	kEventWindowActivated = 5,
	kEventWindowDeactivated = 6,
	kEventWindowGetClickActivation = 7,
	kEventWindowGetClickModality = 8,
	kEventWindowShowing = 22,
	kEventWindowHiding = 23,
	kEventWindowShown = 24,
	kEventWindowHidden = 25,
	kEventWindowBoundsChanging = 26,
	kEventWindowBoundsChanged = 27,
	kEventWindowResizeStarted = 28,
	kEventWindowResizeCompleted = 29,
	kEventWindowDragStarted = 30,
	kEventWindowDragCompleted = 31,
	kEventWindowClickDragRgn = 32,
	kEventWindowClickResizeRgn = 33,
	kEventWindowClickCollapseRgn = 34,
	kEventWindowClickCloseRgn = 35,
	kEventWindowClickZoomRgn = 36,
	kEventWindowClickContentRgn = 37,
	kEventWindowClickProxyIconRgn = 38,
	kEventWindowCursorChange = 40,
	kEventWindowClickToolbarButtonRgn = 41,
	kEventWindowClickStructureRgn = 42,
	kEventWindowCollapse = 66,
	kEventWindowCollapsed = 67,
	kEventWindowCollapseAll = 68,
	kEventWindowExpand = 69,
	kEventWindowExpanded = 70,
	kEventWindowExpandAll = 71,
	kEventWindowClose = 72,
	kEventWindowClosed = 73,
	kEventWindowCloseAll = 74,
	kEventWindowZoom = 75,
	kEventWindowZoomed = 76,
	kEventWindowZoomAll = 77,
	kEventWindowContextualMenuSelect = 78,
	kEventWindowPathSelect = 79,
	kEventWindowGetIdealSize = 80,
	kEventWindowGetMinimumSize = 81,
	kEventWindowGetMaximumSize = 82,
	kEventWindowConstrain = 83,
	kEventWindowHandleContentClick = 85,
	kEventWindowHandleActivate = 91,
	kEventWindowHandleDeactivate = 92,
	kEventWindowProxyBeginDrag = 128,
	kEventWindowProxyEndDrag = 129,
	kEventWindowFocusAcquired = 200,
	kEventWindowFocusRelinquish = 201,
	kEventWindowDefDrawFrame = 1000,
	kEventWindowDrawFrame = 1000,
	kEventWindowDefDrawPart = 1001,
	kEventWindowDrawPart = 1001,
	kEventWindowDefGetRegion = 1002,
	kEventWindowGetRegion = 1002,
	kEventWindowDefHitTest = 1003,
	kEventWindowHitTest = 1003,
	kEventWindowDefInit = 1004,
	kEventWindowInit = 1004,
	kEventWindowDefDispose = 1005,
	kEventWindowDispose = 1005,
	kEventWindowStateChanged = 1009,
	kEventWindowPaint = 1013,
};

enum {
	kEventControlHit = 1,
	kEventControlSimulateHit = 2,
	kEventControlHitTest = 3,
	kEventControlDraw = 4,
	kEventControlApplyBackground = 5,
	kEventControlApplyTextColor = 6,
	kEventControlSetFocusPart = 7,
	kEventControlGetFocusPart = 8,
	kEventControlActivate = 9,
	kEventControlDeactivate = 10,
	kEventControlSetCursor = 11,
	kEventControlContextualMenuClick = 12,
	kEventControlClick = 13,
	kEventControlGetClickActivation = 17,
	kEventControlTrack = 51,
	kEventControlGetScrollToHereStartPoint = 52,
	kEventControlGetIndicatorDragConstraint = 53,
	kEventControlIndicatorMoved = 54,
	kEventControlGhostingFinished = 55,
	kEventControlGetActionProcPart = 56,
	kEventControlGetPartRegion = 101,
	kEventControlGetPartBounds = 102,
	kEventControlSetData = 103,
	kEventControlGetData = 104,
	kEventControlValueFieldChanged = 151,
	kEventControlAddedSubControl = 152,
	kEventControlRemovingSubControl = 153,
	kEventControlBoundsChanged = 154,
	kEventControlVisibilityChanged = 157,
	kEventControlTitleChanged = 158,
	kEventControlOwningWindowChanged = 159,
	kEventControlHiliteChanged = 160,
	kEventControlEnabledStateChanged = 161,
	kEventControlInitialize = 1000,
	kEventControlDispose = 1001,
	kEventControlGetOptimalBounds = 1003,
};

enum {
	kEventParamAttributes = 'attr',
	kEventParamClickActivation = 'clac',
	kEventParamControlAction = 'caup',
	kEventParamControlCurrentPart = 'cnpc',
	kEventParamControlDataBuffer = 'cdbf',
	kEventParamControlDataBufferSize = 'cdbs',
	kEventParamControlDataTag = 'cdtg',
	kEventParamControlFeatures = 'cftr',
	kEventParamControlIndicatorOffset = 'ciof',
	kEventParamControlMessage = 'cmsg',
	kEventParamControlOptimalBaselineOffset = 'cobo',
	kEventParamControlOptimalBounds = 'cobn',
	kEventParamControlParam = 'cprm',
	kEventParamControlPart = 'cprt',
	kEventParamControlPartBounds = 'cpbd',
	kEventParamCGContextRef = 'cntx',
	kEventParamControlRef = 'ctrl',
	kEventParamControlRegion = 'crgn',
	kEventParamControlResult = 'crsl',
	kEventParamControlSubControl = 'csub',
	kEventParamCurrentBounds = 'crct',
	kEventParamCurrentWindow = 'curw',
	kEventParamOriginalBounds = 'orct',
	kEventParamPreviousBounds = 'prct',
	kEventParamRgnHandle = 'rgnh',
	kEventParamWindowDefPart = 'wdpc',
	kEventParamWindowMouseLocation = 'wmou',
	kEventParamWindowPartCode = 'wpar',
	kEventParamWindowRef = 'wind',
	kEventParamWindowRegionCode = 'wshp',
};

enum {
	typeCFArrayRef = 'cfar',
	typeCFBooleanRef = 'cftf',
	typeCFDictionaryRef = 'cfdc',
	typeCFMutableStringRef = 'cfms',
	typeCFStringRef = 'cfst',
	typeCFTypeRef = 'cfty',
	typeCGContextRef = 'cntx',
	typeCGImageRef = 'cgim',
	typeControlActionUPP = 'caup',
	typeControlPartCode = 'cprt',
	typeControlRef = 'ctrl',
	typeDragRef = 'drag',
	typeEventRef = 'evrf',
	typeEventTargetRef = 'etrg',
	typeGrafPtr = 'graf',
	typeHIRect = 'hirc',
	typeHIShapeRef = 'shap',
	typeHISize = 'hisz',
	typeQDPoint = 'QDpt',
	typeQDRectangle = 'qdrt',
	typeQDRgnHandle = 'rgnh',
	typeWindowDefPartCode = 'wdpt',
	typeWindowPartCode = 'wpar',
	typeWindowRef = 'wind',
	typeWindowRegionCode = 'wshp',
};

// kEventParamAttributes of kEventWindowBoundsChanged
enum {
	kWindowBoundsChangeUserDrag = 1 << 0,
	kWindowBoundsChangeUserResize = 1 << 1,
	kWindowBoundsChangeSizeChanged = 1 << 2,
	kWindowBoundsChangeOriginChanged = 1 << 3,
	kWindowBoundsChangeZoom = 1 << 4
};

// The event target of a window or of a control: the window's handlers, then the application's.
EventTargetRef GetWindowEventTarget(WindowRef inWindow);
EventTargetRef GetControlEventTarget(ControlRef inControl);

#define InstallWindowEventHandler(window, handler, numTypes, typeList, userData, outHandlerRef) \
	InstallEventHandler(GetWindowEventTarget(window), (handler), (numTypes), (typeList), (userData), (outHandlerRef))
#define InstallControlEventHandler(control, handler, numTypes, typeList, userData, outHandlerRef) \
	InstallEventHandler(GetControlEventTarget(control), (handler), (numTypes), (typeList), (userData), (outHandlerRef))
#define SendEventToWindow(event, window) SendEventToEventTarget((event), GetWindowEventTarget(window))
#define SendEventToControl(event, control) SendEventToEventTarget((event), GetControlEventTarget(control))

#ifndef __LP64__
// Tracking the mouse (32-bit only)
typedef UInt16 MouseTrackingResult;
enum {
	kMouseTrackingMouseDown = 1,
	kMouseTrackingMouseUp = 2,
	kMouseTrackingMouseExited = 3,
	kMouseTrackingMouseEntered = 4,
	kMouseTrackingMouseDragged = 5,
	kMouseTrackingKeyModifiersChanged = 6,
	kMouseTrackingUserCancelled = 7,
	kMouseTrackingTimedOut = 8,
	kMouseTrackingMouseMoved = 9,
	kMouseTrackingClientEvent = 10,
	kMouseTrackingScrollWheel = 11
};
enum {
	kTrackMouseLocationOptionDontConsumeMouseUp = (1 << 0),
	kTrackMouseLocationOptionIncludeScrollWheel = (1 << 3)
};
OSStatus TrackMouseLocation(GrafPtr inPort, Point* outPt, MouseTrackingResult* outResult);
OSStatus TrackMouseLocationWithOptions(GrafPtr inPort, OptionBits inOptions, EventTimeout inTimeout, Point* outPt, UInt32* outModifiers, MouseTrackingResult* outResult);
#endif

OSStatus EnableSecureEventInput(void);
OSStatus DisableSecureEventInput(void);
Boolean IsSecureEventInputEnabled(void);

#ifdef __cplusplus
}
#endif

#endif
