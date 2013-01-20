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
typedef void* CGDisplayModeRef;

CGDirectDisplayID CGMainDisplayID();

// couple of missing functions (locating the displays)
CGError CGGetActiveDisplayList(uint32_t max, CGDirectDisplayID* list, uint32_t* countOut);
CGError CGGetOnlineDisplayList(uint32_t max, CGDirectDisplayID* list, uint32_t* countOut);
size_t CGDisplayPixelsWide(CGDirectDisplayID id);
size_t CGDisplayPixelsHigh(CGDirectDisplayID id);

// old mode APIs
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

}

#endif
