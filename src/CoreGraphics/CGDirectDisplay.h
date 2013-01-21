/*
This file is part of Darling.

Copyright (C) 2013 Lubos Dolezel

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

#ifndef CGDIRECTDISPLAY_H
#define CGDIRECTDISPLAY_H
#include <stdint.h>
#include <Foundation/NSString.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFArray.h>
#include <CoreGraphics/CGGeometry.h>
#include "CGError.h"

extern "C"
{

typedef uint32_t CGCaptureOptions;
typedef uint32_t CGDirectDisplayID;
typedef CFDictionaryRef CGDisplayModeRef;
typedef float CGGammaValue;

CGDirectDisplayID CGMainDisplayID();

// couple of missing functions (locating the displays)
CGError CGGetActiveDisplayList(uint32_t max, CGDirectDisplayID* list, uint32_t* countOut);
CGError CGGetOnlineDisplayList(uint32_t max, CGDirectDisplayID* list, uint32_t* countOut);
size_t CGDisplayPixelsWide(CGDirectDisplayID id);
size_t CGDisplayPixelsHigh(CGDirectDisplayID id);

// old mode APIs
extern CFStringRef kCGDisplayWidth;
extern CFStringRef kCGDisplayHeight;
extern CFStringRef kCGDisplayMode;
extern CFStringRef kCGDisplayBitsPerPixel;
extern CFStringRef kCGDisplayBitsPerSample;
extern CFStringRef kCGDisplaySamplesPerPixel;
extern CFStringRef kCGDisplayRefreshRate;
extern CFStringRef kCGDisplayModeUsableForDesktopGUI;
extern CFStringRef kCGDisplayIOFlags;
extern CFStringRef kCGDisplayBytesPerRow;

CFDictionaryRef CGDisplayCurrentMode(CGDirectDisplayID id);
CGError CGDisplaySwitchToMode(CGDirectDisplayID id, CFDictionaryRef mode);

// new mode APIs
CFArrayRef CGDisplayCopyAllDisplayModes(CGDirectDisplayID id);
CGDisplayModeRef CGDisplayCopyDisplayMode(CGDirectDisplayID id);
CGDisplayModeRef CGDisplayModeRetain(CGDisplayModeRef mode);
void CGDisplayModeRelease(CGDisplayModeRef mode);
CFStringRef CGDisplayModeCopyPixelEncoding(CGDisplayModeRef mode);
size_t CGDisplayModeGetHeight(CGDisplayModeRef mode);
double CGDisplayModeGetRefreshRate(CGDisplayModeRef mode);
CFTypeID CGDisplayModeGetTypeID(); // typeid of CGDisplayModeRef
size_t CGDisplayModeGetWidth(CGDisplayModeRef mode);
bool CGDisplayModeIsUsableForDesktopGUI(CGDisplayModeRef mode);

bool CGDisplayIsActive(CGDirectDisplayID id);
bool CGDisplayIsAlwaysInMirrorSet(CGDirectDisplayID id);
bool CGDisplayIsAsleep(CGDirectDisplayID id);
bool CGDisplayIsBuiltin(CGDirectDisplayID id);
bool CGDisplayIsCaptured(CGDirectDisplayID id);
bool CGDisplayIsInHWMirrorSet(CGDirectDisplayID id);
bool CGDisplayIsInMirrorSet(CGDirectDisplayID id);
bool CGDisplayIsMain(CGDirectDisplayID id);
bool CGDisplayIsOnline(CGDirectDisplayID id);
bool CGDisplayIsStereo(CGDirectDisplayID id);
CGDirectDisplayID CGDisplayMirrorsDisplay(CGDirectDisplayID id);
uint32_t CGDisplayModelNumber(CGDirectDisplayID);
CGDirectDisplayID CGDisplayPrimaryDisplay(CGDirectDisplayID id);
double CGDisplayRotation(CGDirectDisplayID id);
CGSize CGDisplayScreenSize(CGDirectDisplayID id);
uint32_t CGDisplaySerialNumber(CGDirectDisplayID id);
CGError CGDisplaySetDisplayMode(CGDirectDisplayID id, CGDisplayModeRef mode, CFDictionaryRef unused);
uint32_t CGDisplayUnitNumber(CGDirectDisplayID id);
bool CGDisplayUsesOpenGLAcceleration(CGDirectDisplayID id);
uint32_t CGDisplayVendorNumber(CGDirectDisplayID id);

CGError CGCaptureAllDisplays();
CGError CGCaptureAllDisplaysWithOptions(CGCaptureOptions opts);
bool CGDisplayIsCaptured(CGDirectDisplayID id);
CGError CGDisplayRelease(CGDirectDisplayID id);
CGError CGReleaseAllDisplays();

CGError CGDisplayHideCursor(CGDirectDisplayID id);
CGError CGDisplayShowCursor(CGDirectDisplayID id);

CGError CGDisplayMoveCursorToPoint(CGDirectDisplayID id, CGPoint pt);
void CGGetLastMouseDelta(int32_t* x, int32_t* y);

// gamma
CGError CGSetDisplayTransferByFormula(CGDirectDisplayID id, CGGammaValue redMin, CGGammaValue redMax, CGGammaValue redGamma,
		CGGammaValue greenMin, CGGammaValue greenMax, CGGammaValue greenGamma, CGGammaValue blueMin, CGGammaValue blueMax, CGGammaValue blueGamma);
CGError CGGetDisplayTransferByFormula(CGDirectDisplayID id, CGGammaValue* redMin, CGGammaValue* redMax, CGGammaValue* redGamma,
		CGGammaValue* greenMin, CGGammaValue* greenMax, CGGammaValue* greenGamma, CGGammaValue* blueMin, CGGammaValue* blueMax, CGGammaValue* blueGamma);

uint32_t CGDisplayGammaTableCapacity(CGDirectDisplayID id);
CGError CGSetDisplayTransferByTable(CGDirectDisplayID id, uint32_t tableSize, const CGGammaValue* redTable, const CGGammaValue* greenTable, const CGGammaValue* blueTable);
CGError CGGetDisplayTransferByTable(CGDirectDisplayID id, uint32_t capacity, CGGammaValue* redTable, CGGammaValue* greenTable, CGGammaValue* blueTable, uint32_t* sampleCount);

}

#endif
