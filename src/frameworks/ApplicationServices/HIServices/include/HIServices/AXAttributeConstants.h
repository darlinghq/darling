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

#ifndef _HISERVICES_AXATTRIBUTECONSTANTS_H_
#define _HISERVICES_AXATTRIBUTECONSTANTS_H_

#include <CoreFoundation/CoreFoundation.h>

// Attribute names of the accessibility API. The parameterized text attributes
// have the same names as their NSAccessibility counterparts in AppKit.

// parameterized text attributes
#define kAXLineForIndexParameterizedAttribute             CFSTR("AXLineForIndex")
#define kAXRangeForLineParameterizedAttribute             CFSTR("AXRangeForLine")
#define kAXStringForRangeParameterizedAttribute           CFSTR("AXStringForRange")
#define kAXRangeForPositionParameterizedAttribute         CFSTR("AXRangeForPosition")
#define kAXRangeForIndexParameterizedAttribute            CFSTR("AXRangeForIndex")
#define kAXBoundsForRangeParameterizedAttribute           CFSTR("AXBoundsForRange")
#define kAXRTFForRangeParameterizedAttribute              CFSTR("AXRTFForRange")
#define kAXAttributedStringForRangeParameterizedAttribute CFSTR("AXAttributedStringForRange")
#define kAXStyleRangeForIndexParameterizedAttribute       CFSTR("AXStyleRangeForIndex")

// menu item attributes
#define kAXMenuItemCmdCharAttribute                       CFSTR("AXMenuItemCmdChar")
#define kAXMenuItemCmdVirtualKeyAttribute                 CFSTR("AXMenuItemCmdVirtualKey")
#define kAXMenuItemCmdGlyphAttribute                      CFSTR("AXMenuItemCmdGlyph")
#define kAXMenuItemCmdModifiersAttribute                  CFSTR("AXMenuItemCmdModifiers")
#define kAXMenuItemMarkCharAttribute                      CFSTR("AXMenuItemMarkChar")
#define kAXMenuItemPrimaryUIElementAttribute              CFSTR("AXMenuItemPrimaryUIElement")

#endif
