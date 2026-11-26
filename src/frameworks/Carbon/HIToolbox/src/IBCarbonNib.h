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

#ifndef HITOOLBOX_IBCARBONNIB_H
#define HITOOLBOX_IBCARBONNIB_H

// Not installed. The Carbon nib reader (IBCarbonNib.c) turns the objects of a nib into a neutral
// description of Core Foundation values, from which the Menu Manager (IBCarbonRuntime.c) builds
// menus, and the Window Manager and Control Manager build windows and controls
// (IBCarbonWindows.c).
//
// The description of an object is a CFDictionary with
//   "class"       the class name of the nib, for example "IBCarbonMenuItem"
//   "id"          the CFNumber id of the object in the nib, if it has one
// and one entry per attribute the object has in the nib, by its name ("title", "command", "bounds",
// "controlID", ...):
//   string        a CFString
//   int, unsigned_int, integer, float, double
//                 a CFNumber
//   boolean       kCFBooleanTrue or kCFBooleanFalse
//   ostype        a CFNumber with the four characters as a 32-bit integer, first character in the
//                 high byte
//   array         a CFArray of the descriptions of its elements
//   dictionary    a CFDictionary from the string keys to the values
//   object        the description of the object, as above
//   reference     the description of the object it refers to
// The children of a control, such as the "subviews" of an IBCarbonRootControl, are the elements of
// the array of that name.

#include <HIToolbox/IBCarbonRuntime.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

OSStatus _IBCarbonNibOpen(CFStringRef objectsPath, IBNibRef* outNib);
bool _IBCarbonNibExists(IBNibRef nib);
void _IBCarbonNibClose(IBNibRef nib);

// The nib of the IBNibRef: parses its objects.xib on first use. Returns NULL when there is none
// of the name.
CFDictionaryRef _IBCarbonNibCopyObjectNamed(IBNibRef nib, CFStringRef name);

// The description of the target framework of the nib ("IBCarbonFramework" for the nibs of the
// Carbon runtime), or NULL
CFStringRef _IBCarbonNibCopyTargetFramework(IBNibRef nib);

// Creates a window from the description of an IBCarbonWindow object with its controls. Provided
// by the Window Manager; without one, it fails with unimpErr.
OSStatus _HICreateWindowFromNibDescription(CFDictionaryRef description, WindowRef* outWindow);

// The menus of a nib menu bar, retained; the caller releases them and frees the array
OSStatus _HICreateMenuBarMenusFromNib(IBNibRef nib, CFStringRef name, MenuRef** outMenus, ItemCount* outCount);

#ifdef __cplusplus
}
#endif

#endif
