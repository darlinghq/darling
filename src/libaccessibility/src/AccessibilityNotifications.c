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

#include <CoreFoundation/CoreFoundation.h>

// Names of the notifications posted when an accessibility setting changes.
// Clients observe them on the local and Darwin notification centers.
//
// The string is not documented; it was read from the exported constant on macOS 26.7.3 and 10.14.4 (black-box observation).
const CFStringRef kAXSEnhanceTextLegibilityChangedNotification = CFSTR("com.apple.accessibility.text.legibility.status");

// Keys of the display settings in the accessibility preferences, as a change
// notification names them. The strings were read from the exported constants
// on macOS 26.7.3 (black-box observation).
CFStringRef kAXInterfaceReduceMotionKey = CFSTR("reduceMotion");
CFStringRef kAXInterfaceIncreaseContrastKey = CFSTR("increaseContrast");
CFStringRef kAXInterfaceDifferentiateWithoutColorKey = CFSTR("differentiateWithoutColor");
