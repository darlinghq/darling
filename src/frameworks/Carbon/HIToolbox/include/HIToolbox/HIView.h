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

#ifndef _HITOOLBOX_HIVIEW_H_
#define _HITOOLBOX_HIVIEW_H_

// HIView: the view hierarchy of a window's content. The Control Manager's controls are views.

#include <HIToolbox/Controls.h>
#include <HIToolbox/HIGeometry.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef ControlID HIViewID;
typedef ControlPartCode HIViewPartCode;
typedef UInt32 HIViewZOrderOp;

enum {
	kHIViewNoPart = 0,
	kHIViewEntireView = 0,
	kHIViewIndicatorPart = 129,
	kHIViewDisabledPart = 254,
	kHIViewInactivePart = 255,
	kHIViewZOrderAbove = 1,
	kHIViewZOrderBelow = 2,
	kHIViewFeatureAllowsSubviews = 2,
	kHIViewFeatureGetsFocusOnClick = 256,
	kHIViewFeatureIsOpaque = 33554432,
	kHIViewFeatureDoesNotDraw = 134217728,
	kHIViewFeatureDoesNotUseSpecialParts = 268435456,
	kHIViewFeatureIdlesWithTimer = 8388608,
	kHIViewFeatureSupportsGhosting = 1,
	kHIViewFeatureAutoToggles = 16384,
};

extern const HIViewID kHIViewWindowContentID;

OSStatus HIViewFindByID(HIViewRef inStartView, HIViewID inID, HIViewRef* outControl);
HIViewRef HIViewGetRoot(WindowRef inWindow);
WindowRef HIViewGetWindow(HIViewRef inView);
HIViewRef HIViewGetSuperview(HIViewRef inView);
OSStatus HIViewAddSubview(HIViewRef inParent, HIViewRef inNewChild);
OSStatus HIViewRemoveFromSuperview(HIViewRef inView);
OSStatus HIViewGetBounds(HIViewRef inView, HIRect* outRect);
OSStatus HIViewGetFrame(HIViewRef inView, HIRect* outRect);
OSStatus HIViewSetFrame(HIViewRef inView, const HIRect* inRect);
OSStatus HIViewMoveBy(HIViewRef inView, CGFloat inDX, CGFloat inDY);
OSStatus HIViewSetVisible(HIViewRef inView, Boolean inVisible);
Boolean HIViewIsVisible(HIViewRef inView);
OSStatus HIViewSetNeedsDisplay(HIViewRef inView, Boolean inNeedsDisplay);
OSStatus HIViewSetNeedsDisplayInRect(HIViewRef inView, const HIRect* inRect, Boolean inNeedsDisplay);
OSStatus HIViewSetEnabled(HIViewRef inView, Boolean inEnabled);
Boolean HIViewIsEnabled(HIViewRef inView);
SInt32 HIViewGetValue(HIViewRef inView);
OSStatus HIViewSetValue(HIViewRef inView, SInt32 inValue);
SInt32 HIViewGetMinimum(HIViewRef inView);
OSStatus HIViewSetMinimum(HIViewRef inView, SInt32 inValue);
SInt32 HIViewGetMaximum(HIViewRef inView);
OSStatus HIViewSetMaximum(HIViewRef inView, SInt32 inValue);
OSStatus HIViewSetHilite(HIViewRef inView, HIViewPartCode inHilitePart);
OSStatus HIViewGetViewForMouseEvent(HIViewRef inView, EventRef inEvent, HIViewRef* outView);
OSStatus HIViewConvertPoint(HIPoint* ioPoint, HIViewRef inSourceView, HIViewRef inDestView);
OSStatus HIViewConvertRect(HIRect* ioRect, HIViewRef inSourceView, HIViewRef inDestView);
OSStatus HIViewSimulateClick(HIViewRef inView, HIViewPartCode inPartToClick, OptionBits inModifiers, ControlPartCode* outPartClicked);

OSStatus HIViewRender(HIViewRef inView);
HIViewRef HIViewGetFirstSubview(HIViewRef inView);
#ifndef __LP64__
HIViewRef HIViewGetLastSubview(HIViewRef inView);
#endif
HIViewRef HIViewGetNextView(HIViewRef inView);
#ifndef __LP64__
HIViewRef HIViewGetPreviousView(HIViewRef inView);
#endif
OSStatus HIViewSetZOrder(HIViewRef inView, HIViewZOrderOp inOp, HIViewRef inOther);
#ifndef __LP64__
OSStatus HIViewGetSubviewHit(HIViewRef inView, const HIPoint* inPoint, Boolean inDeep, HIViewRef* outView);
#endif
#ifndef __LP64__
OSStatus HIViewClick(HIViewRef inView, EventRef inEvent);
#endif
Boolean HIViewIsActive(HIViewRef inView, Boolean* outIsLatentActive);
OSStatus HIViewSetActivated(HIViewRef inView, Boolean inSetActivated);
OSStatus HIViewSetCommandID(HIViewRef inView, UInt32 inCommandID);
OSStatus HIViewGetCommandID(HIViewRef inView, UInt32* outCommandID);
OSStatus HIViewSetBoundsOrigin(HIViewRef inView, CGFloat inX, CGFloat inY);
OSStatus HIViewSetDrawingEnabled(HIViewRef inView, Boolean inEnabled);
Boolean HIViewIsDrawingEnabled(HIViewRef inView);

#ifdef __cplusplus
}
#endif

#endif
