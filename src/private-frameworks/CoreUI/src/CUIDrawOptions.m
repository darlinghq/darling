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

#include <CoreUI/CUIDrawOptions.h>

// Apple does not document the strings behind these constants. Each was read
// from the exported constant on macOS 26.7.3 (black-box observation); all but
// the kCUIWidgetSwitch* ones are exported, with the same strings, by 10.14.4.

// Option keys of a CUIDraw options dictionary
const CFStringRef kCUIWidgetKey = CFSTR("widget");
const CFStringRef kCUIStateKey = CFSTR("state");
const CFStringRef kCUIValueKey = CFSTR("value");
const CFStringRef kCUISizeKey = CFSTR("size");
const CFStringRef kCUIScaleKey = CFSTR("kCUIScaleKey");
const CFStringRef kCUIOrientationKey = CFSTR("kCUIOrientationKey");
const CFStringRef kCUIPresentationStateKey = CFSTR("kCUIPresentationStateKey");
const CFStringRef kCUIUserInterfaceLayoutDirectionKey = CFSTR("kCUIUserInterfaceLayoutDirectionKey");
const CFStringRef kCUIIsFlippedKey = CFSTR("is.flipped");
const CFStringRef kCUIMaskOnlyKey = CFSTR("kCUIMaskOnlyKey");
const CFStringRef kCUIAnimationStartTimeKey = CFSTR("animstart");
const CFStringRef kCUIAnimationTimeKey = CFSTR("animend");

// Values of kCUIWidgetKey
const CFStringRef kCUIWidgetButtonComboBox = CFSTR("combobox");
const CFStringRef kCUIWidgetButtonLittleArrows = CFSTR("littlearrows");
const CFStringRef kCUIWidgetProgressBar = CFSTR("kCUIWidgetProgressBar");
const CFStringRef kCUIWidgetProgressIndeterminateBar = CFSTR("kCUIWidgetIndeterminateBar");
const CFStringRef kCUIWidgetScrollBarTrackCorner = CFSTR("kCUIWidgetScrollBarTrackCorner");
const CFStringRef kCUIWidgetSwitchBorder = CFSTR("kCUIWidgetSwitchBorder");
const CFStringRef kCUIWidgetSwitchFill = CFSTR("kCUIWidgetSwitchFill");
const CFStringRef kCUIWidgetSwitchFillMask = CFSTR("kCUIWidgetSwitchFillMask");
const CFStringRef kCUIWidgetSwitchKnob = CFSTR("kCUIWidgetSwitchKnob");
const CFStringRef kCUIWidgetSwitchOnOffLabel = CFSTR("kCUIWidgetSwitchOnOffLabel");

// Values of kCUIStateKey
const CFStringRef kCUIStateActive = CFSTR("normal");
const CFStringRef kCUIStateDisabled = CFSTR("disabled");
const CFStringRef kCUIStatePressed = CFSTR("pressed");

// Values of kCUISizeKey
const CFStringRef kCUISizeMini = CFSTR("mini");
const CFStringRef kCUISizeSmall = CFSTR("small");
const CFStringRef kCUISizeRegular = CFSTR("regular");

// Values of kCUIOrientationKey
const CFStringRef kCUIOrientHorizontal = CFSTR("kCUIOrientHorizontal");

// Values of kCUIPresentationStateKey
const CFStringRef kCUIPresentationStateActiveKey = CFSTR("kCUIPresentationStateActiveKey");
const CFStringRef kCUIPresentationStateInactive = CFSTR("kCUIPresentationStateInactive");

// Values of kCUIUserInterfaceLayoutDirectionKey
const CFStringRef kCUIUserInterfaceLayoutDirectionLeftToRight = CFSTR("kCUIUserInterfaceLayoutDirectionLeftToRight");
const CFStringRef kCUIUserInterfaceLayoutDirectionRightToLeft = CFSTR("kCUIUserInterfaceLayoutDirectionRightToLeft");
