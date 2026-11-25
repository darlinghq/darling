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

#ifndef _HITOOLBOX_CONTROLS_H_
#define _HITOOLBOX_CONTROLS_H_

// The Control Manager: controls (views) in windows. The values and the functions that exist only
// in the 32-bit Carbon API (CarbonLib heritage) are guarded by !__LP64__, as on macOS.

#include <CoreServices/MacTypes.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <QD/QD.h>
#include <HIToolbox/HIObject.h>
#include <HIToolbox/MacWindows.h>
#include <HIToolbox/Events.h>
#include <HIToolbox/CarbonEventsCore.h>

#pragma pack(push, 2)

#ifdef __cplusplus
extern "C" {
#endif

typedef ControlRef HIViewRef;
typedef SInt16 ControlPartCode;
typedef UInt32 ControlFeatures;
typedef UInt32 ControlSize;
typedef OSType ControlPropertyTag;
typedef SInt16 ControlFocusPart;
typedef SInt16 ControlVariant;
typedef SInt32 ControlProcID;
typedef UInt32 ControlNavigationDirection;

typedef struct ControlID {
	OSType signature;
	SInt32 id;
} ControlID;

typedef struct ControlKind {
	OSType signature;
	OSType kind;
} ControlKind;

typedef struct ControlFontStyleRec {
	SInt16 flags;
	SInt16 font;
	SInt16 size;
	SInt16 style;
	SInt16 mode;
	SInt16 just;
	RGBColor foreColor;
	RGBColor backColor;
} ControlFontStyleRec;
typedef ControlFontStyleRec* ControlFontStylePtr;

typedef void (*ControlActionProcPtr)(ControlRef theControl, ControlPartCode partCode);
typedef ControlActionProcPtr ControlActionUPP;

// Control parts
enum {
	kControlNoPart = 0,
	kControlEntireControl = 0,
	kControlLabelPart = 1,
	kControlMenuPart = 2,
	kControlTrianglePart = 4,
	kControlEditTextPart = 5,
	kControlPicturePart = 6,
	kControlButtonPart = 10,
	kControlCheckBoxPart = 11,
	kControlUpButtonPart = 20,
	kControlDownButtonPart = 21,
	kControlPageUpPart = 22,
	kControlPageDownPart = 23,
	kControlListBoxPart = 24,
	kControlIndicatorPart = 129,
	kControlDisabledPart = 254,
	kControlInactivePart = 255,
	kControlFocusNoPart = 0,
	kControlFocusNextPart = -1,
	kControlFocusPrevPart = -2,
};

// Control features
enum {
	kControlSupportsGhosting = 1,
	kControlSupportsEmbedding = 2,
	kControlSupportsFocus = 4,
	kControlWantsIdle = 8,
	kControlWantsActivate = 16,
	kControlHandlesTracking = 32,
	kControlSupportsDataAccess = 64,
	kControlHasSpecialBackground = 128,
	kControlGetsFocusOnClick = 256,
	kControlSupportsCalcBestRect = 512,
	kControlSupportsLiveFeedback = 1024,
	kControlHasRadioBehavior = 2048,
	kControlSupportsDragAndDrop = 4096,
	kControlAutoToggles = 16384,
	kControlSupportsGetRegion = 131072,
	kControlSupportsFlattening = 524288,
	kControlSupportsSetCursor = 1048576,
	kControlSupportsContextualMenus = 2097152,
	kControlSupportsClickActivation = 4194304,
	kControlIdlesWithTimer = 8388608,
};

// Data tags every control has
enum {
	kControlFontStyleTag = 'font',
	kControlKeyFilterTag = 'fltr',
	kControlKindTag = 'kind',
	kControlSizeTag = 'size',
};

enum {
	kControlSizeNormal = 0,
	kControlSizeSmall = 1,
	kControlSizeLarge = 2,
	kControlSizeMini = 3,
	kControlSizeAuto = 65535,
};

enum {
	kControlFontBigSystemFont = -1,
	kControlFontSmallSystemFont = -2,
	kControlFontSmallBoldSystemFont = -3,
	kControlFontViewSystemFont = -4,
};


enum {
	kControlUsesOwningWindowsFontVariant = 1 << 3
};

// Making and disposing of controls
#ifndef __LP64__
ControlRef NewControl(WindowRef owningWindow, const Rect* boundsRect, ConstStr255Param controlTitle, Boolean initiallyVisible, SInt16 initialValue, SInt16 minimumValue, SInt16 maximumValue, SInt16 procID, SInt32 controlReference);
void DisposeControl(ControlRef theControl);
#endif
OSStatus CreateRootControl(WindowRef inWindow, ControlRef* outControl);
OSStatus GetRootControl(WindowRef inWindow, ControlRef* outControl);
OSStatus GetControlByID(WindowRef inWindow, const ControlID* inID, ControlRef* outControl);
#ifndef __LP64__
OSStatus GetControlID(ControlRef inControl, ControlID* outID);
OSStatus SetControlID(ControlRef inControl, const ControlID* inID);
#endif
WindowRef GetControlOwner(ControlRef control);
OSStatus GetControlKind(ControlRef inControl, ControlKind* outControlKind);
OSStatus GetControlFeatures(ControlRef inControl, UInt32* outFeatures);

// Values
SInt16 GetControlValue(ControlRef theControl);
void SetControlValue(ControlRef theControl, SInt16 newValue);
SInt32 GetControl32BitValue(ControlRef theControl);
#ifndef __LP64__
void SetControl32BitValue(ControlRef theControl, SInt32 newValue);
SInt16 GetControlMinimum(ControlRef theControl);
void SetControlMinimum(ControlRef theControl, SInt16 newMinimum);
SInt16 GetControlMaximum(ControlRef theControl);
void SetControlMaximum(ControlRef theControl, SInt16 newMaximum);
#endif
SInt32 GetControl32BitMinimum(ControlRef theControl);
void SetControl32BitMinimum(ControlRef theControl, SInt32 newMinimum);
SInt32 GetControl32BitMaximum(ControlRef theControl);
void SetControl32BitMaximum(ControlRef theControl, SInt32 newMaximum);
#ifndef __LP64__
SInt32 GetControlViewSize(ControlRef theControl);
void SetControlViewSize(ControlRef theControl, SInt32 newViewSize);
SInt32 GetControlReference(ControlRef theControl);
void SetControlReference(ControlRef theControl, SInt32 data);
#endif
OSStatus SetControlTitleWithCFString(ControlRef inControl, CFStringRef inString);
OSStatus CopyControlTitleAsCFString(ControlRef inControl, CFStringRef* outString);
#ifndef __LP64__
void SetControlTitle(ControlRef theControl, ConstStr255Param title);
void GetControlTitle(ControlRef theControl, Str255 title);
#endif

// Data of a control (the tags are those of the control definition)
OSStatus SetControlData(ControlRef inControl, ControlPartCode inPart, ResType inTagName, Size inSize, const void* inData);
OSStatus GetControlData(ControlRef inControl, ControlPartCode inPart, ResType inTagName, Size inBufferSize, void* inBuffer, Size* outActualSize);
OSStatus GetControlDataSize(ControlRef inControl, ControlPartCode inPart, ResType inTagName, Size* outMaxSize);

// Geometry and state, in window coordinates
Rect* GetControlBounds(ControlRef control, Rect* bounds);
#ifndef __LP64__
void SetControlBounds(ControlRef control, const Rect* bounds);
void MoveControl(ControlRef theControl, SInt16 h, SInt16 v);
void SizeControl(ControlRef theControl, SInt16 w, SInt16 h);
void ShowControl(ControlRef theControl);
void HideControl(ControlRef theControl);
#endif
Boolean IsControlVisible(ControlRef inControl);
OSStatus ActivateControl(ControlRef inControl);
OSStatus DeactivateControl(ControlRef inControl);
Boolean IsControlActive(ControlRef inControl);
OSStatus EnableControl(ControlRef inControl);
OSStatus DisableControl(ControlRef inControl);
Boolean IsControlEnabled(ControlRef inControl);
#ifndef __LP64__
void HiliteControl(ControlRef theControl, ControlPartCode hiliteState);
void DrawOneControl(ControlRef theControl);
void Draw1Control(ControlRef theControl);
void DrawControls(WindowRef theWindow);
void UpdateControls(WindowRef inWindow, RgnHandle inRegion);
#endif

// Mouse
#ifndef __LP64__
ControlPartCode HandleControlClick(ControlRef inControl, Point inWhere, EventModifiers inModifiers, ControlActionUPP inAction);
ControlPartCode TrackControl(ControlRef theControl, Point startPoint, ControlActionUPP actionProc);
ControlPartCode TestControl(ControlRef theControl, Point testPoint);
ControlPartCode FindControl(Point testPoint, WindowRef theWindow, ControlRef* theControl);
#endif
ControlRef FindControlUnderMouse(Point inWhere, WindowRef inWindow, ControlPartCode* outPart);

// Control action functions
#ifndef __LP64__
ControlActionUPP NewControlActionUPP(ControlActionProcPtr userRoutine);
void DisposeControlActionUPP(ControlActionUPP userUPP);
void SetControlAction(ControlRef theControl, ControlActionUPP actionProc);
ControlActionUPP GetControlAction(ControlRef theControl);
#endif

// Embedding
OSStatus EmbedControl(ControlRef inControl, ControlRef inContainer);
OSStatus AutoEmbedControl(ControlRef inControl, WindowRef inWindow);
OSStatus CountSubControls(ControlRef inControl, UInt16* outNumChildren);
OSStatus GetIndexedSubControl(ControlRef inControl, UInt16 inIndex, ControlRef* outSubControl);
OSStatus GetSuperControl(ControlRef inControl, ControlRef* outParent);

#ifdef __cplusplus
}
#endif

#pragma pack(pop)

#endif
