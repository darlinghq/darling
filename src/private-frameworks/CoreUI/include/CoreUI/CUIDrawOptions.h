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

#ifndef _CoreUI_CUIDrawOptions_H_
#define _CoreUI_CUIDrawOptions_H_

#include <CoreFoundation/CoreFoundation.h>

// Keys and values of the options dictionary that clients (AppKit, WebKit)
// pass to CUIDraw to describe the widget to draw. Compare them by value
// (CFEqual), as dictionary keys do.

// Option keys of a CUIDraw options dictionary
extern const CFStringRef kCUIWidgetKey;
extern const CFStringRef kCUIStateKey;
extern const CFStringRef kCUIValueKey;
extern const CFStringRef kCUISizeKey;
extern const CFStringRef kCUIScaleKey;
extern const CFStringRef kCUIOrientationKey;
extern const CFStringRef kCUIPresentationStateKey;
extern const CFStringRef kCUIUserInterfaceLayoutDirectionKey;
extern const CFStringRef kCUIIsFlippedKey;
extern const CFStringRef kCUIMaskOnlyKey;
extern const CFStringRef kCUIAnimationStartTimeKey;
extern const CFStringRef kCUIAnimationTimeKey;

// Values of kCUIWidgetKey
extern const CFStringRef kCUIWidgetButtonComboBox;
extern const CFStringRef kCUIWidgetButtonLittleArrows;
extern const CFStringRef kCUIWidgetProgressBar;
extern const CFStringRef kCUIWidgetProgressIndeterminateBar;
extern const CFStringRef kCUIWidgetScrollBarTrackCorner;
extern const CFStringRef kCUIWidgetSwitchBorder;
extern const CFStringRef kCUIWidgetSwitchFill;
extern const CFStringRef kCUIWidgetSwitchFillMask;
extern const CFStringRef kCUIWidgetSwitchKnob;
extern const CFStringRef kCUIWidgetSwitchOnOffLabel;

// Values of kCUIStateKey
extern const CFStringRef kCUIStateActive;
extern const CFStringRef kCUIStateDisabled;
extern const CFStringRef kCUIStatePressed;

// Values of kCUISizeKey
extern const CFStringRef kCUISizeMini;
extern const CFStringRef kCUISizeSmall;
extern const CFStringRef kCUISizeRegular;

// Values of kCUIOrientationKey
extern const CFStringRef kCUIOrientHorizontal;

// Values of kCUIPresentationStateKey
extern const CFStringRef kCUIPresentationStateActiveKey;
extern const CFStringRef kCUIPresentationStateInactive;

// Values of kCUIUserInterfaceLayoutDirectionKey
extern const CFStringRef kCUIUserInterfaceLayoutDirectionLeftToRight;
extern const CFStringRef kCUIUserInterfaceLayoutDirectionRightToLeft;

#endif
