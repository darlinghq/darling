/*
 This file is part of Darling.

 Copyright (C) 2021 Lubos Dolezel

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


#include <SkyLight/SkyLight.h>
#include <stdlib.h>
#include <stdio.h>

const CFStringRef kSLSGestureScrollTrackpadDevicesKey = CFSTR("kSLSGestureScrollTrackpadDevicesKey");

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* SLAcquireDisplayFadeReservation(void)
{
    if (verbose) puts("STUB: SLAcquireDisplayFadeReservation called");
    return NULL;
}

void* SLAssociateMouseAndMouseCursorPosition(void)
{
    if (verbose) puts("STUB: SLAssociateMouseAndMouseCursorPosition called");
    return NULL;
}

void* SLBeginDisplayConfiguration(void)
{
    if (verbose) puts("STUB: SLBeginDisplayConfiguration called");
    return NULL;
}

void* SLCFDictionaryGetBoolean(void)
{
    if (verbose) puts("STUB: SLCFDictionaryGetBoolean called");
    return NULL;
}

void* SLCFDictionaryGetFloat(void)
{
    if (verbose) puts("STUB: SLCFDictionaryGetFloat called");
    return NULL;
}

void* SLCFDictionaryGetInteger(void)
{
    if (verbose) puts("STUB: SLCFDictionaryGetInteger called");
    return NULL;
}

void* SLCFDictionaryGetNumber(void)
{
    if (verbose) puts("STUB: SLCFDictionaryGetNumber called");
    return NULL;
}

void* SLCFDictionaryGetRect(void)
{
    if (verbose) puts("STUB: SLCFDictionaryGetRect called");
    return NULL;
}

void* SLCFDictionaryGetSize(void)
{
    if (verbose) puts("STUB: SLCFDictionaryGetSize called");
    return NULL;
}

void* SLCancelDisplayConfiguration(void)
{
    if (verbose) puts("STUB: SLCancelDisplayConfiguration called");
    return NULL;
}

void* SLCaptureAllDisplays(void)
{
    if (verbose) puts("STUB: SLCaptureAllDisplays called");
    return NULL;
}

void* SLCaptureAllDisplaysWithOptions(void)
{
    if (verbose) puts("STUB: SLCaptureAllDisplaysWithOptions called");
    return NULL;
}

void* SLCompleteDisplayConfiguration(void)
{
    if (verbose) puts("STUB: SLCompleteDisplayConfiguration called");
    return NULL;
}

void* SLConfigureDisplayFadeEffect(void)
{
    if (verbose) puts("STUB: SLConfigureDisplayFadeEffect called");
    return NULL;
}

void* SLConfigureDisplayMirrorOfDisplay(void)
{
    if (verbose) puts("STUB: SLConfigureDisplayMirrorOfDisplay called");
    return NULL;
}

void* SLConfigureDisplayMode(void)
{
    if (verbose) puts("STUB: SLConfigureDisplayMode called");
    return NULL;
}

void* SLConfigureDisplayOrigin(void)
{
    if (verbose) puts("STUB: SLConfigureDisplayOrigin called");
    return NULL;
}

void* SLConfigureDisplayStereoOperation(void)
{
    if (verbose) puts("STUB: SLConfigureDisplayStereoOperation called");
    return NULL;
}

void* SLConfigureDisplayWithDisplayMode(void)
{
    if (verbose) puts("STUB: SLConfigureDisplayWithDisplayMode called");
    return NULL;
}

void* SLContextCopyDisplayCaptureContentsToRect(void)
{
    if (verbose) puts("STUB: SLContextCopyDisplayCaptureContentsToRect called");
    return NULL;
}

void* SLContextCopyWindowCaptureContentsToRect(void)
{
    if (verbose) puts("STUB: SLContextCopyWindowCaptureContentsToRect called");
    return NULL;
}

void* SLContextCopyWindowContentsToRect(void)
{
    if (verbose) puts("STUB: SLContextCopyWindowContentsToRect called");
    return NULL;
}

void* SLCursorIsDrawnInFramebuffer(void)
{
    if (verbose) puts("STUB: SLCursorIsDrawnInFramebuffer called");
    return NULL;
}

void* SLCursorIsVisible(void)
{
    if (verbose) puts("STUB: SLCursorIsVisible called");
    return NULL;
}

void* SLDirectDisplayCopyCurrentMetalDevice(void)
{
    if (verbose) puts("STUB: SLDirectDisplayCopyCurrentMetalDevice called");
    return NULL;
}

void* SLDisplayAddressForPosition(void)
{
    if (verbose) puts("STUB: SLDisplayAddressForPosition called");
    return NULL;
}

void* SLDisplayAvailableModes(void)
{
    if (verbose) puts("STUB: SLDisplayAvailableModes called");
    return NULL;
}

void* SLDisplayBaseAddress(void)
{
    if (verbose) puts("STUB: SLDisplayBaseAddress called");
    return NULL;
}

void* SLDisplayBeamPosition(void)
{
    if (verbose) puts("STUB: SLDisplayBeamPosition called");
    return NULL;
}

void* SLDisplayBestModeForParameters(void)
{
    if (verbose) puts("STUB: SLDisplayBestModeForParameters called");
    return NULL;
}

void* SLDisplayBestModeForParametersAndRefreshRate(void)
{
    if (verbose) puts("STUB: SLDisplayBestModeForParametersAndRefreshRate called");
    return NULL;
}

void* SLDisplayBestModeForParametersAndRefreshRateWithProperty(void)
{
    if (verbose) puts("STUB: SLDisplayBestModeForParametersAndRefreshRateWithProperty called");
    return NULL;
}

void* SLDisplayBitsPerPixel(void)
{
    if (verbose) puts("STUB: SLDisplayBitsPerPixel called");
    return NULL;
}

void* SLDisplayBitsPerSample(void)
{
    if (verbose) puts("STUB: SLDisplayBitsPerSample called");
    return NULL;
}

void* SLDisplayBounds(void)
{
    if (verbose) puts("STUB: SLDisplayBounds called");
    return NULL;
}

void* SLDisplayBytesPerRow(void)
{
    if (verbose) puts("STUB: SLDisplayBytesPerRow called");
    return NULL;
}

void* SLDisplayCanSetPalette(void)
{
    if (verbose) puts("STUB: SLDisplayCanSetPalette called");
    return NULL;
}

void* SLDisplayCapture(void)
{
    if (verbose) puts("STUB: SLDisplayCapture called");
    return NULL;
}

void* SLDisplayCaptureWithOptions(void)
{
    if (verbose) puts("STUB: SLDisplayCaptureWithOptions called");
    return NULL;
}

void* SLDisplayCopyAllDisplayModes(void)
{
    if (verbose) puts("STUB: SLDisplayCopyAllDisplayModes called");
    return NULL;
}

void* SLDisplayCopyColorSpace(void)
{
    if (verbose) puts("STUB: SLDisplayCopyColorSpace called");
    return NULL;
}

void* SLDisplayCopyDisplayMode(void)
{
    if (verbose) puts("STUB: SLDisplayCopyDisplayMode called");
    return NULL;
}

void* SLDisplayCreateImage(void)
{
    if (verbose) puts("STUB: SLDisplayCreateImage called");
    return NULL;
}

void* SLDisplayCreateImageForRect(void)
{
    if (verbose) puts("STUB: SLDisplayCreateImageForRect called");
    return NULL;
}

void* SLDisplayCurrentMode(void)
{
    if (verbose) puts("STUB: SLDisplayCurrentMode called");
    return NULL;
}

void* SLDisplayDefaultVisualBell(void)
{
    if (verbose) puts("STUB: SLDisplayDefaultVisualBell called");
    return NULL;
}

void* SLDisplayDoesNotDim(void)
{
    if (verbose) puts("STUB: SLDisplayDoesNotDim called");
    return NULL;
}

void* SLDisplayFade(void)
{
    if (verbose) puts("STUB: SLDisplayFade called");
    return NULL;
}

void* SLDisplayFadeOperationInProgress(void)
{
    if (verbose) puts("STUB: SLDisplayFadeOperationInProgress called");
    return NULL;
}

void* SLDisplayForceToGray(void)
{
    if (verbose) puts("STUB: SLDisplayForceToGray called");
    return NULL;
}

void* SLDisplayGammaTableCapacity(void)
{
    if (verbose) puts("STUB: SLDisplayGammaTableCapacity called");
    return NULL;
}

void* SLDisplayGetDrawingContext(void)
{
    if (verbose) puts("STUB: SLDisplayGetDrawingContext called");
    return NULL;
}

void* SLDisplayHideCursor(void)
{
    if (verbose) puts("STUB: SLDisplayHideCursor called");
    return NULL;
}

void* SLDisplayIDToOpenGLDisplayMask(void)
{
    if (verbose) puts("STUB: SLDisplayIDToOpenGLDisplayMask called");
    return NULL;
}

void* SLDisplayIOServicePort(void)
{
    if (verbose) puts("STUB: SLDisplayIOServicePort called");
    return NULL;
}

void* SLDisplayIsActive(void)
{
    if (verbose) puts("STUB: SLDisplayIsActive called");
    return NULL;
}

void* SLDisplayIsAlwaysInMirrorSet(void)
{
    if (verbose) puts("STUB: SLDisplayIsAlwaysInMirrorSet called");
    return NULL;
}

void* SLDisplayIsAsleep(void)
{
    if (verbose) puts("STUB: SLDisplayIsAsleep called");
    return NULL;
}

void* SLDisplayIsBuiltin(void)
{
    if (verbose) puts("STUB: SLDisplayIsBuiltin called");
    return NULL;
}

void* SLDisplayIsCaptured(void)
{
    if (verbose) puts("STUB: SLDisplayIsCaptured called");
    return NULL;
}

void* SLDisplayIsInHWMirrorSet(void)
{
    if (verbose) puts("STUB: SLDisplayIsInHWMirrorSet called");
    return NULL;
}

void* SLDisplayIsInMirrorSet(void)
{
    if (verbose) puts("STUB: SLDisplayIsInMirrorSet called");
    return NULL;
}

void* SLDisplayIsMain(void)
{
    if (verbose) puts("STUB: SLDisplayIsMain called");
    return NULL;
}

void* SLDisplayIsOnline(void)
{
    if (verbose) puts("STUB: SLDisplayIsOnline called");
    return NULL;
}

void* SLDisplayIsStereo(void)
{
    if (verbose) puts("STUB: SLDisplayIsStereo called");
    return NULL;
}

void* SLDisplayMirrorsDisplay(void)
{
    if (verbose) puts("STUB: SLDisplayMirrorsDisplay called");
    return NULL;
}

void* SLDisplayModeCopyPixelEncoding(void)
{
    if (verbose) puts("STUB: SLDisplayModeCopyPixelEncoding called");
    return NULL;
}

void* SLDisplayModeCreate(void)
{
    if (verbose) puts("STUB: SLDisplayModeCreate called");
    return NULL;
}

void* SLDisplayModeGetHeight(void)
{
    if (verbose) puts("STUB: SLDisplayModeGetHeight called");
    return NULL;
}

void* SLDisplayModeGetIODisplayModeID(void)
{
    if (verbose) puts("STUB: SLDisplayModeGetIODisplayModeID called");
    return NULL;
}

void* SLDisplayModeGetIOFlags(void)
{
    if (verbose) puts("STUB: SLDisplayModeGetIOFlags called");
    return NULL;
}

void* SLDisplayModeGetPixelDensity(void)
{
    if (verbose) puts("STUB: SLDisplayModeGetPixelDensity called");
    return NULL;
}

void* SLDisplayModeGetPixelHeight(void)
{
    if (verbose) puts("STUB: SLDisplayModeGetPixelHeight called");
    return NULL;
}

void* SLDisplayModeGetPixelWidth(void)
{
    if (verbose) puts("STUB: SLDisplayModeGetPixelWidth called");
    return NULL;
}

void* SLDisplayModeGetPixelsHigh(void)
{
    if (verbose) puts("STUB: SLDisplayModeGetPixelsHigh called");
    return NULL;
}

void* SLDisplayModeGetPixelsWide(void)
{
    if (verbose) puts("STUB: SLDisplayModeGetPixelsWide called");
    return NULL;
}

void* SLDisplayModeGetRefreshRate(void)
{
    if (verbose) puts("STUB: SLDisplayModeGetRefreshRate called");
    return NULL;
}

void* SLDisplayModeGetResolution(void)
{
    if (verbose) puts("STUB: SLDisplayModeGetResolution called");
    return NULL;
}

void* SLDisplayModeGetTypeID(void)
{
    if (verbose) puts("STUB: SLDisplayModeGetTypeID called");
    return NULL;
}

void* SLDisplayModeGetWidth(void)
{
    if (verbose) puts("STUB: SLDisplayModeGetWidth called");
    return NULL;
}

void* SLDisplayModeIsUsableForDesktopGUI(void)
{
    if (verbose) puts("STUB: SLDisplayModeIsUsableForDesktopGUI called");
    return NULL;
}

void* SLDisplayModeRelease(void)
{
    if (verbose) puts("STUB: SLDisplayModeRelease called");
    return NULL;
}

void* SLDisplayModeRetain(void)
{
    if (verbose) puts("STUB: SLDisplayModeRetain called");
    return NULL;
}

void* SLDisplayModelNumber(void)
{
    if (verbose) puts("STUB: SLDisplayModelNumber called");
    return NULL;
}

void* SLDisplayMoveCursorToPoint(void)
{
    if (verbose) puts("STUB: SLDisplayMoveCursorToPoint called");
    return NULL;
}

void* SLDisplayPixelsHigh(void)
{
    if (verbose) puts("STUB: SLDisplayPixelsHigh called");
    return NULL;
}

void* SLDisplayPixelsWide(void)
{
    if (verbose) puts("STUB: SLDisplayPixelsWide called");
    return NULL;
}

void* SLDisplayPrimaryDisplay(void)
{
    if (verbose) puts("STUB: SLDisplayPrimaryDisplay called");
    return NULL;
}

void* SLDisplayRegisterReconfigurationCallback(void)
{
    if (verbose) puts("STUB: SLDisplayRegisterReconfigurationCallback called");
    return NULL;
}

void* SLDisplayRelease(void)
{
    if (verbose) puts("STUB: SLDisplayRelease called");
    return NULL;
}

void* SLDisplayRemoveReconfigurationCallback(void)
{
    if (verbose) puts("STUB: SLDisplayRemoveReconfigurationCallback called");
    return NULL;
}

void* SLDisplayRestoreColorSyncSettings(void)
{
    if (verbose) puts("STUB: SLDisplayRestoreColorSyncSettings called");
    return NULL;
}

void* SLDisplayRotation(void)
{
    if (verbose) puts("STUB: SLDisplayRotation called");
    return NULL;
}

void* SLDisplaySamplesPerPixel(void)
{
    if (verbose) puts("STUB: SLDisplaySamplesPerPixel called");
    return NULL;
}

void* SLDisplayScreenSize(void)
{
    if (verbose) puts("STUB: SLDisplayScreenSize called");
    return NULL;
}

void* SLDisplaySerialNumber(void)
{
    if (verbose) puts("STUB: SLDisplaySerialNumber called");
    return NULL;
}

void* SLDisplaySetDisplayMode(void)
{
    if (verbose) puts("STUB: SLDisplaySetDisplayMode called");
    return NULL;
}

void* SLDisplaySetInvertedPolarity(void)
{
    if (verbose) puts("STUB: SLDisplaySetInvertedPolarity called");
    return NULL;
}

void* SLDisplaySetPalette(void)
{
    if (verbose) puts("STUB: SLDisplaySetPalette called");
    return NULL;
}

void* SLDisplaySetStereoOperation(void)
{
    if (verbose) puts("STUB: SLDisplaySetStereoOperation called");
    return NULL;
}

void* SLDisplayShowCursor(void)
{
    if (verbose) puts("STUB: SLDisplayShowCursor called");
    return NULL;
}

void* SLDisplayStreamCreate(void)
{
    if (verbose) puts("STUB: SLDisplayStreamCreate called");
    return NULL;
}

void* SLDisplayStreamCreateWithDispatchQueue(void)
{
    if (verbose) puts("STUB: SLDisplayStreamCreateWithDispatchQueue called");
    return NULL;
}

void* SLDisplayStreamGetRunLoopSource(void)
{
    if (verbose) puts("STUB: SLDisplayStreamGetRunLoopSource called");
    return NULL;
}

void* SLDisplayStreamGetTypeID(void)
{
    if (verbose) puts("STUB: SLDisplayStreamGetTypeID called");
    return NULL;
}

void* SLDisplayStreamStart(void)
{
    if (verbose) puts("STUB: SLDisplayStreamStart called");
    return NULL;
}

void* SLDisplayStreamStop(void)
{
    if (verbose) puts("STUB: SLDisplayStreamStop called");
    return NULL;
}

void* SLDisplayStreamUpdateCreateMergedUpdate(void)
{
    if (verbose) puts("STUB: SLDisplayStreamUpdateCreateMergedUpdate called");
    return NULL;
}

void* SLDisplayStreamUpdateGetDropCount(void)
{
    if (verbose) puts("STUB: SLDisplayStreamUpdateGetDropCount called");
    return NULL;
}

void* SLDisplayStreamUpdateGetMovedRectsDelta(void)
{
    if (verbose) puts("STUB: SLDisplayStreamUpdateGetMovedRectsDelta called");
    return NULL;
}

void* SLDisplayStreamUpdateGetRects(void)
{
    if (verbose) puts("STUB: SLDisplayStreamUpdateGetRects called");
    return NULL;
}

void* SLDisplayStreamUpdateGetTypeID(void)
{
    if (verbose) puts("STUB: SLDisplayStreamUpdateGetTypeID called");
    return NULL;
}

void* SLDisplaySwitchToMode(void)
{
    if (verbose) puts("STUB: SLDisplaySwitchToMode called");
    return NULL;
}

void* SLDisplayUnitNumber(void)
{
    if (verbose) puts("STUB: SLDisplayUnitNumber called");
    return NULL;
}

void* SLDisplayUsesForceToGray(void)
{
    if (verbose) puts("STUB: SLDisplayUsesForceToGray called");
    return NULL;
}

void* SLDisplayUsesInvertedPolarity(void)
{
    if (verbose) puts("STUB: SLDisplayUsesInvertedPolarity called");
    return NULL;
}

void* SLDisplayUsesOpenGLAcceleration(void)
{
    if (verbose) puts("STUB: SLDisplayUsesOpenGLAcceleration called");
    return NULL;
}

void* SLDisplayVendorNumber(void)
{
    if (verbose) puts("STUB: SLDisplayVendorNumber called");
    return NULL;
}

void* SLDisplayVisualBell(void)
{
    if (verbose) puts("STUB: SLDisplayVisualBell called");
    return NULL;
}

void* SLDisplayWaitForBeamPositionOutsideLines(void)
{
    if (verbose) puts("STUB: SLDisplayWaitForBeamPositionOutsideLines called");
    return NULL;
}

void* SLEnableEventStateCombining(void)
{
    if (verbose) puts("STUB: SLEnableEventStateCombining called");
    return NULL;
}

void* SLEventCopyIOHIDEvent(void)
{
    if (verbose) puts("STUB: SLEventCopyIOHIDEvent called");
    return NULL;
}

void* SLEventCopyNextEvent(void)
{
    if (verbose) puts("STUB: SLEventCopyNextEvent called");
    return NULL;
}

void* SLEventCreate(void)
{
    if (verbose) puts("STUB: SLEventCreate called");
    return NULL;
}

void* SLEventCreateCopy(void)
{
    if (verbose) puts("STUB: SLEventCreateCopy called");
    return NULL;
}

void* SLEventCreateData(void)
{
    if (verbose) puts("STUB: SLEventCreateData called");
    return NULL;
}

void* SLEventCreateFromData(void)
{
    if (verbose) puts("STUB: SLEventCreateFromData called");
    return NULL;
}

void* SLEventCreateKeyboardEvent(void)
{
    if (verbose) puts("STUB: SLEventCreateKeyboardEvent called");
    return NULL;
}

void* SLEventCreateMouseEvent(void)
{
    if (verbose) puts("STUB: SLEventCreateMouseEvent called");
    return NULL;
}

void* SLEventCreateNextEvent(void)
{
    if (verbose) puts("STUB: SLEventCreateNextEvent called");
    return NULL;
}

void* SLEventCreateScrollWheelEvent(void)
{
    if (verbose) puts("STUB: SLEventCreateScrollWheelEvent called");
    return NULL;
}

void* SLEventCreateScrollWheelEvent2(void)
{
    if (verbose) puts("STUB: SLEventCreateScrollWheelEvent2 called");
    return NULL;
}

void* SLEventCreateSourceFromEvent(void)
{
    if (verbose) puts("STUB: SLEventCreateSourceFromEvent called");
    return NULL;
}

void* SLEventCreateWithEventRecord(void)
{
    if (verbose) puts("STUB: SLEventCreateWithEventRecord called");
    return NULL;
}

void* SLEventGetDoubleValueField(void)
{
    if (verbose) puts("STUB: SLEventGetDoubleValueField called");
    return NULL;
}

void* SLEventGetEventRecord(void)
{
    if (verbose) puts("STUB: SLEventGetEventRecord called");
    return NULL;
}

void* SLEventGetFlags(void)
{
    if (verbose) puts("STUB: SLEventGetFlags called");
    return NULL;
}

void* SLEventGetIntegerValueField(void)
{
    if (verbose) puts("STUB: SLEventGetIntegerValueField called");
    return NULL;
}

void* SLEventGetLocation(void)
{
    if (verbose) puts("STUB: SLEventGetLocation called");
    return NULL;
}

void* SLEventGetTimestamp(void)
{
    if (verbose) puts("STUB: SLEventGetTimestamp called");
    return NULL;
}

void* SLEventGetType(void)
{
    if (verbose) puts("STUB: SLEventGetType called");
    return NULL;
}

void* SLEventGetTypeID(void)
{
    if (verbose) puts("STUB: SLEventGetTypeID called");
    return NULL;
}

void* SLEventGetUnflippedLocation(void)
{
    if (verbose) puts("STUB: SLEventGetUnflippedLocation called");
    return NULL;
}

void* SLEventGetUnflippedWindowLocation(void)
{
    if (verbose) puts("STUB: SLEventGetUnflippedWindowLocation called");
    return NULL;
}

void* SLEventGetWindowLocation(void)
{
    if (verbose) puts("STUB: SLEventGetWindowLocation called");
    return NULL;
}

void* SLEventKeyboardGetUnicodeString(void)
{
    if (verbose) puts("STUB: SLEventKeyboardGetUnicodeString called");
    return NULL;
}

void* SLEventKeyboardSetUnicodeString(void)
{
    if (verbose) puts("STUB: SLEventKeyboardSetUnicodeString called");
    return NULL;
}

void* SLEventPost(void)
{
    if (verbose) puts("STUB: SLEventPost called");
    return NULL;
}

void* SLEventPostEventToPSN(void)
{
    if (verbose) puts("STUB: SLEventPostEventToPSN called");
    return NULL;
}

void* SLEventPostToPSN(void)
{
    if (verbose) puts("STUB: SLEventPostToPSN called");
    return NULL;
}

void* SLEventPostToPid(void)
{
    if (verbose) puts("STUB: SLEventPostToPid called");
    return NULL;
}

void* SLEventRecordPointer(void)
{
    if (verbose) puts("STUB: SLEventRecordPointer called");
    return NULL;
}

void* SLEventSetDoubleValueField(void)
{
    if (verbose) puts("STUB: SLEventSetDoubleValueField called");
    return NULL;
}

void* SLEventSetFlags(void)
{
    if (verbose) puts("STUB: SLEventSetFlags called");
    return NULL;
}

void* SLEventSetIOHIDEvent(void)
{
    if (verbose) puts("STUB: SLEventSetIOHIDEvent called");
    return NULL;
}

void* SLEventSetIntegerValueField(void)
{
    if (verbose) puts("STUB: SLEventSetIntegerValueField called");
    return NULL;
}

void* SLEventSetLocation(void)
{
    if (verbose) puts("STUB: SLEventSetLocation called");
    return NULL;
}

void* SLEventSetSource(void)
{
    if (verbose) puts("STUB: SLEventSetSource called");
    return NULL;
}

void* SLEventSetTimestamp(void)
{
    if (verbose) puts("STUB: SLEventSetTimestamp called");
    return NULL;
}

void* SLEventSetType(void)
{
    if (verbose) puts("STUB: SLEventSetType called");
    return NULL;
}

void* SLEventSetWindowLocation(void)
{
    if (verbose) puts("STUB: SLEventSetWindowLocation called");
    return NULL;
}

void* SLEventSourceButtonState(void)
{
    if (verbose) puts("STUB: SLEventSourceButtonState called");
    return NULL;
}

void* SLEventSourceCounterForEventType(void)
{
    if (verbose) puts("STUB: SLEventSourceCounterForEventType called");
    return NULL;
}

void* SLEventSourceCreate(void)
{
    if (verbose) puts("STUB: SLEventSourceCreate called");
    return NULL;
}

void* SLEventSourceFlagsState(void)
{
    if (verbose) puts("STUB: SLEventSourceFlagsState called");
    return NULL;
}

void* SLEventSourceGetKeyboardType(void)
{
    if (verbose) puts("STUB: SLEventSourceGetKeyboardType called");
    return NULL;
}

void* SLEventSourceGetLocalEventsFilterDuringSuppressionState(void)
{
    if (verbose) puts("STUB: SLEventSourceGetLocalEventsFilterDuringSuppressionState called");
    return NULL;
}

void* SLEventSourceGetLocalEventsSuppressionInterval(void)
{
    if (verbose) puts("STUB: SLEventSourceGetLocalEventsSuppressionInterval called");
    return NULL;
}

void* SLEventSourceGetPixelsPerLine(void)
{
    if (verbose) puts("STUB: SLEventSourceGetPixelsPerLine called");
    return NULL;
}

void* SLEventSourceGetSourceStateID(void)
{
    if (verbose) puts("STUB: SLEventSourceGetSourceStateID called");
    return NULL;
}

void* SLEventSourceGetTypeID(void)
{
    if (verbose) puts("STUB: SLEventSourceGetTypeID called");
    return NULL;
}

void* SLEventSourceGetUserData(void)
{
    if (verbose) puts("STUB: SLEventSourceGetUserData called");
    return NULL;
}

void* SLEventSourceKeyState(void)
{
    if (verbose) puts("STUB: SLEventSourceKeyState called");
    return NULL;
}

void* SLEventSourceSecondsSinceLastEventType(void)
{
    if (verbose) puts("STUB: SLEventSourceSecondsSinceLastEventType called");
    return NULL;
}

void* SLEventSourceSetKeyboardType(void)
{
    if (verbose) puts("STUB: SLEventSourceSetKeyboardType called");
    return NULL;
}

void* SLEventSourceSetLocalEventsFilterDuringSuppressionState(void)
{
    if (verbose) puts("STUB: SLEventSourceSetLocalEventsFilterDuringSuppressionState called");
    return NULL;
}

void* SLEventSourceSetLocalEventsSuppressionInterval(void)
{
    if (verbose) puts("STUB: SLEventSourceSetLocalEventsSuppressionInterval called");
    return NULL;
}

void* SLEventSourceSetPixelsPerLine(void)
{
    if (verbose) puts("STUB: SLEventSourceSetPixelsPerLine called");
    return NULL;
}

void* SLEventSourceSetUserData(void)
{
    if (verbose) puts("STUB: SLEventSourceSetUserData called");
    return NULL;
}

void* SLEventTapCreate(void)
{
    if (verbose) puts("STUB: SLEventTapCreate called");
    return NULL;
}

void* SLEventTapCreateForPSN(void)
{
    if (verbose) puts("STUB: SLEventTapCreateForPSN called");
    return NULL;
}

void* SLEventTapCreateForPid(void)
{
    if (verbose) puts("STUB: SLEventTapCreateForPid called");
    return NULL;
}

void* SLEventTapEnable(void)
{
    if (verbose) puts("STUB: SLEventTapEnable called");
    return NULL;
}

void* SLEventTapIDEnable(void)
{
    if (verbose) puts("STUB: SLEventTapIDEnable called");
    return NULL;
}

void* SLEventTapIDIsEnabled(void)
{
    if (verbose) puts("STUB: SLEventTapIDIsEnabled called");
    return NULL;
}

void* SLEventTapIsEnabled(void)
{
    if (verbose) puts("STUB: SLEventTapIsEnabled called");
    return NULL;
}

void* SLEventTapPostEvent(void)
{
    if (verbose) puts("STUB: SLEventTapPostEvent called");
    return NULL;
}

void* SLGetActiveDisplayList(void)
{
    if (verbose) puts("STUB: SLGetActiveDisplayList called");
    return NULL;
}

void* SLGetDisplayTransferByFormula(void)
{
    if (verbose) puts("STUB: SLGetDisplayTransferByFormula called");
    return NULL;
}

void* SLGetDisplayTransferByTable(void)
{
    if (verbose) puts("STUB: SLGetDisplayTransferByTable called");
    return NULL;
}

void* SLGetDisplaysWithOpenGLDisplayMask(void)
{
    if (verbose) puts("STUB: SLGetDisplaysWithOpenGLDisplayMask called");
    return NULL;
}

void* SLGetDisplaysWithPoint(void)
{
    if (verbose) puts("STUB: SLGetDisplaysWithPoint called");
    return NULL;
}

void* SLGetDisplaysWithRect(void)
{
    if (verbose) puts("STUB: SLGetDisplaysWithRect called");
    return NULL;
}

void* SLGetEventTapList(void)
{
    if (verbose) puts("STUB: SLGetEventTapList called");
    return NULL;
}

void* SLGetLastMouseDelta(void)
{
    if (verbose) puts("STUB: SLGetLastMouseDelta called");
    return NULL;
}

void* SLGetOnlineDisplayList(void)
{
    if (verbose) puts("STUB: SLGetOnlineDisplayList called");
    return NULL;
}

void* SLInhibitLocalEvents(void)
{
    if (verbose) puts("STUB: SLInhibitLocalEvents called");
    return NULL;
}

void* SLMainDisplayID(void)
{
    if (verbose) puts("STUB: SLMainDisplayID called");
    return NULL;
}

void* SLOpenGLDisplayMaskToDisplayID(void)
{
    if (verbose) puts("STUB: SLOpenGLDisplayMaskToDisplayID called");
    return NULL;
}

void* SLPSBringNextToFront(void)
{
    if (verbose) puts("STUB: SLPSBringNextToFront called");
    return NULL;
}

void* SLPSCompleteServiceReq(void)
{
    if (verbose) puts("STUB: SLPSCompleteServiceReq called");
    return NULL;
}

void* SLPSCompleteServiceReqOnConnection(void)
{
    if (verbose) puts("STUB: SLPSCompleteServiceReqOnConnection called");
    return NULL;
}

void* SLPSDequeueServiceReq(void)
{
    if (verbose) puts("STUB: SLPSDequeueServiceReq called");
    return NULL;
}

void* SLPSDequeueServiceReqOnConnection(void)
{
    if (verbose) puts("STUB: SLPSDequeueServiceReqOnConnection called");
    return NULL;
}

void* SLPSEnableForegroundOperation(void)
{
    if (verbose) puts("STUB: SLPSEnableForegroundOperation called");
    return NULL;
}

void* SLPSEqualProcess(void)
{
    if (verbose) puts("STUB: SLPSEqualProcess called");
    return NULL;
}

void* SLPSExitSpecialKeyMode(void)
{
    if (verbose) puts("STUB: SLPSExitSpecialKeyMode called");
    return NULL;
}

void* SLPSFindProcessByDisplayName(void)
{
    if (verbose) puts("STUB: SLPSFindProcessByDisplayName called");
    return NULL;
}

void* SLPSFindProcessByPID(void)
{
    if (verbose) puts("STUB: SLPSFindProcessByPID called");
    return NULL;
}

void* SLPSGetCurrentProcess(void)
{
    if (verbose) puts("STUB: SLPSGetCurrentProcess called");
    return NULL;
}

void* SLPSGetFrontProcess(void)
{
    if (verbose) puts("STUB: SLPSGetFrontProcess called");
    return NULL;
}

void* SLPSGetKeyBinding(void)
{
    if (verbose) puts("STUB: SLPSGetKeyBinding called");
    return NULL;
}

void* SLPSGetKeyFocusProcess(void)
{
    if (verbose) puts("STUB: SLPSGetKeyFocusProcess called");
    return NULL;
}

void* SLPSGetNextProcess(void)
{
    if (verbose) puts("STUB: SLPSGetNextProcess called");
    return NULL;
}

void* SLPSGetNextToFrontProcess(void)
{
    if (verbose) puts("STUB: SLPSGetNextToFrontProcess called");
    return NULL;
}

void* SLPSGetProcessInfo(void)
{
    if (verbose) puts("STUB: SLPSGetProcessInfo called");
    return NULL;
}

void* SLPSGetTypingFocusProcess(void)
{
    if (verbose) puts("STUB: SLPSGetTypingFocusProcess called");
    return NULL;
}

void* SLPSGetWindowOwner(void)
{
    if (verbose) puts("STUB: SLPSGetWindowOwner called");
    return NULL;
}

void* SLPSGetWindowPresenter(void)
{
    if (verbose) puts("STUB: SLPSGetWindowPresenter called");
    return NULL;
}

void* SLPSMarkChildWindow(void)
{
    if (verbose) puts("STUB: SLPSMarkChildWindow called");
    return NULL;
}

void* SLPSModifyConnectionNotifications(void)
{
    if (verbose) puts("STUB: SLPSModifyConnectionNotifications called");
    return NULL;
}

void* SLPSOverrideKeyRegistration(void)
{
    if (verbose) puts("STUB: SLPSOverrideKeyRegistration called");
    return NULL;
}

void* SLPSPBGetProcessInfo(void)
{
    if (verbose) puts("STUB: SLPSPBGetProcessInfo called");
    return NULL;
}

void* SLPSPostEventRecordTo(void)
{
    if (verbose) puts("STUB: SLPSPostEventRecordTo called");
    return NULL;
}

void* SLPSPostHideMostReq(void)
{
    if (verbose) puts("STUB: SLPSPostHideMostReq called");
    return NULL;
}

void* SLPSPostHideReq(void)
{
    if (verbose) puts("STUB: SLPSPostHideReq called");
    return NULL;
}

void* SLPSPostShowReq(void)
{
    if (verbose) puts("STUB: SLPSPostShowReq called");
    return NULL;
}

void* SLPSProcessInformationCopyDictionary(void)
{
    if (verbose) puts("STUB: SLPSProcessInformationCopyDictionary called");
    return NULL;
}

void* SLPSRegisterForKey(void)
{
    if (verbose) puts("STUB: SLPSRegisterForKey called");
    return NULL;
}

void* SLPSRegisterForKeyOnConnection(void)
{
    if (verbose) puts("STUB: SLPSRegisterForKeyOnConnection called");
    return NULL;
}

void* SLPSRegisterWithServer(void)
{
    if (verbose) puts("STUB: SLPSRegisterWithServer called");
    return NULL;
}

void* SLPSReleaseKeyFocus(void)
{
    if (verbose) puts("STUB: SLPSReleaseKeyFocus called");
    return NULL;
}

void* SLPSReleaseKeyFocusOnConnection(void)
{
    if (verbose) puts("STUB: SLPSReleaseKeyFocusOnConnection called");
    return NULL;
}

void* SLPSReleaseKeyFocusWithID(void)
{
    if (verbose) puts("STUB: SLPSReleaseKeyFocusWithID called");
    return NULL;
}

void* SLPSSetDefaultBootstrapPort(void)
{
    if (verbose) puts("STUB: SLPSSetDefaultBootstrapPort called");
    return NULL;
}

void* SLPSSetFrontProcess(void)
{
    if (verbose) puts("STUB: SLPSSetFrontProcess called");
    return NULL;
}

void* SLPSSetFrontProcessWithOptions(void)
{
    if (verbose) puts("STUB: SLPSSetFrontProcessWithOptions called");
    return NULL;
}

void* SLPSSetMainApplicationConnection(void)
{
    if (verbose) puts("STUB: SLPSSetMainApplicationConnection called");
    return NULL;
}

void* SLPSSetNotifications(void)
{
    if (verbose) puts("STUB: SLPSSetNotifications called");
    return NULL;
}

void* SLPSSetProcessName(void)
{
    if (verbose) puts("STUB: SLPSSetProcessName called");
    return NULL;
}

void* SLPSSetProcessToBringForwardAtNextApplicationExit(void)
{
    if (verbose) puts("STUB: SLPSSetProcessToBringForwardAtNextApplicationExit called");
    return NULL;
}

void* SLPSStealKeyFocus(void)
{
    if (verbose) puts("STUB: SLPSStealKeyFocus called");
    return NULL;
}

void* SLPSStealKeyFocusOnConnection(void)
{
    if (verbose) puts("STUB: SLPSStealKeyFocusOnConnection called");
    return NULL;
}

void* SLPSStealKeyFocusReturningID(void)
{
    if (verbose) puts("STUB: SLPSStealKeyFocusReturningID called");
    return NULL;
}

void* SLPSUnoverrideKeyRegistration(void)
{
    if (verbose) puts("STUB: SLPSUnoverrideKeyRegistration called");
    return NULL;
}

void* SLPaletteCreateCopy(void)
{
    if (verbose) puts("STUB: SLPaletteCreateCopy called");
    return NULL;
}

void* SLPaletteCreateDefaultColorPalette(void)
{
    if (verbose) puts("STUB: SLPaletteCreateDefaultColorPalette called");
    return NULL;
}

void* SLPaletteCreateFromPaletteBlendedWithColor(void)
{
    if (verbose) puts("STUB: SLPaletteCreateFromPaletteBlendedWithColor called");
    return NULL;
}

void* SLPaletteCreateWithByteSamples(void)
{
    if (verbose) puts("STUB: SLPaletteCreateWithByteSamples called");
    return NULL;
}

void* SLPaletteCreateWithCapacity(void)
{
    if (verbose) puts("STUB: SLPaletteCreateWithCapacity called");
    return NULL;
}

void* SLPaletteCreateWithDisplay(void)
{
    if (verbose) puts("STUB: SLPaletteCreateWithDisplay called");
    return NULL;
}

void* SLPaletteCreateWithSamples(void)
{
    if (verbose) puts("STUB: SLPaletteCreateWithSamples called");
    return NULL;
}

void* SLPaletteGetColorAtIndex(void)
{
    if (verbose) puts("STUB: SLPaletteGetColorAtIndex called");
    return NULL;
}

void* SLPaletteGetIndexForColor(void)
{
    if (verbose) puts("STUB: SLPaletteGetIndexForColor called");
    return NULL;
}

void* SLPaletteGetNumberOfSamples(void)
{
    if (verbose) puts("STUB: SLPaletteGetNumberOfSamples called");
    return NULL;
}

void* SLPaletteIsEqualToPalette(void)
{
    if (verbose) puts("STUB: SLPaletteIsEqualToPalette called");
    return NULL;
}

void* SLPaletteRelease(void)
{
    if (verbose) puts("STUB: SLPaletteRelease called");
    return NULL;
}

void* SLPaletteSetColorAtIndex(void)
{
    if (verbose) puts("STUB: SLPaletteSetColorAtIndex called");
    return NULL;
}

void* SLPixelAccessCreate(void)
{
    if (verbose) puts("STUB: SLPixelAccessCreate called");
    return NULL;
}

void* SLPixelAccessCreateImageFromRect(void)
{
    if (verbose) puts("STUB: SLPixelAccessCreateImageFromRect called");
    return NULL;
}

void* SLPixelAccessCreateWithWindow(void)
{
    if (verbose) puts("STUB: SLPixelAccessCreateWithWindow called");
    return NULL;
}

void* SLPixelAccessGetAlphaBitsPerComponent(void)
{
    if (verbose) puts("STUB: SLPixelAccessGetAlphaBitsPerComponent called");
    return NULL;
}

void* SLPixelAccessGetAlphaBitsPerPixel(void)
{
    if (verbose) puts("STUB: SLPixelAccessGetAlphaBitsPerPixel called");
    return NULL;
}

void* SLPixelAccessGetAlphaData(void)
{
    if (verbose) puts("STUB: SLPixelAccessGetAlphaData called");
    return NULL;
}

void* SLPixelAccessGetBitmapInfo(void)
{
    if (verbose) puts("STUB: SLPixelAccessGetBitmapInfo called");
    return NULL;
}

void* SLPixelAccessGetBitsPerComponent(void)
{
    if (verbose) puts("STUB: SLPixelAccessGetBitsPerComponent called");
    return NULL;
}

void* SLPixelAccessGetBitsPerPixel(void)
{
    if (verbose) puts("STUB: SLPixelAccessGetBitsPerPixel called");
    return NULL;
}

void* SLPixelAccessGetColorSpace(void)
{
    if (verbose) puts("STUB: SLPixelAccessGetColorSpace called");
    return NULL;
}

void* SLPixelAccessGetData(void)
{
    if (verbose) puts("STUB: SLPixelAccessGetData called");
    return NULL;
}

void* SLPixelAccessGetHeight(void)
{
    if (verbose) puts("STUB: SLPixelAccessGetHeight called");
    return NULL;
}

void* SLPixelAccessGetTransform(void)
{
    if (verbose) puts("STUB: SLPixelAccessGetTransform called");
    return NULL;
}

void* SLPixelAccessGetTypeID(void)
{
    if (verbose) puts("STUB: SLPixelAccessGetTypeID called");
    return NULL;
}

void* SLPixelAccessGetWidth(void)
{
    if (verbose) puts("STUB: SLPixelAccessGetWidth called");
    return NULL;
}

void* SLPixelAccessLock(void)
{
    if (verbose) puts("STUB: SLPixelAccessLock called");
    return NULL;
}

void* SLPixelAccessRect(void)
{
    if (verbose) puts("STUB: SLPixelAccessRect called");
    return NULL;
}

void* SLPixelAccessRelease(void)
{
    if (verbose) puts("STUB: SLPixelAccessRelease called");
    return NULL;
}

void* SLPixelAccessTransformRect(void)
{
    if (verbose) puts("STUB: SLPixelAccessTransformRect called");
    return NULL;
}

void* SLPixelAccessUnlock(void)
{
    if (verbose) puts("STUB: SLPixelAccessUnlock called");
    return NULL;
}

void* SLPostKeyboardEvent(void)
{
    if (verbose) puts("STUB: SLPostKeyboardEvent called");
    return NULL;
}

void* SLPostMouseEvent(void)
{
    if (verbose) puts("STUB: SLPostMouseEvent called");
    return NULL;
}

void* SLPostScrollWheelEvent(void)
{
    if (verbose) puts("STUB: SLPostScrollWheelEvent called");
    return NULL;
}

void* SLPreflightListenEventAccess(void)
{
    if (verbose) puts("STUB: SLPreflightListenEventAccess called");
    return NULL;
}

void* SLPreflightPostEventAccess(void)
{
    if (verbose) puts("STUB: SLPreflightPostEventAccess called");
    return NULL;
}

void* SLPreflightScreenCaptureAccess(void)
{
    if (verbose) puts("STUB: SLPreflightScreenCaptureAccess called");
    return NULL;
}

void* SLRegisterScreenRefreshCallback(void)
{
    if (verbose) puts("STUB: SLRegisterScreenRefreshCallback called");
    return NULL;
}

void* SLReleaseAllDisplays(void)
{
    if (verbose) puts("STUB: SLReleaseAllDisplays called");
    return NULL;
}

void* SLReleaseDisplayFadeReservation(void)
{
    if (verbose) puts("STUB: SLReleaseDisplayFadeReservation called");
    return NULL;
}

void* SLReleaseScreenRefreshRects(void)
{
    if (verbose) puts("STUB: SLReleaseScreenRefreshRects called");
    return NULL;
}

void* SLRequestListenEventAccess(void)
{
    if (verbose) puts("STUB: SLRequestListenEventAccess called");
    return NULL;
}

void* SLRequestPostEventAccess(void)
{
    if (verbose) puts("STUB: SLRequestPostEventAccess called");
    return NULL;
}

void* SLRequestScreenCaptureAccess(void)
{
    if (verbose) puts("STUB: SLRequestScreenCaptureAccess called");
    return NULL;
}

void* SLRestorePermanentDisplayConfiguration(void)
{
    if (verbose) puts("STUB: SLRestorePermanentDisplayConfiguration called");
    return NULL;
}

void* SLSAAHSetAppearanceTheme(void)
{
    if (verbose) puts("STUB: SLSAAHSetAppearanceTheme called");
    return NULL;
}

void* SLSAcceleratorForDisplayAlias(void)
{
    if (verbose) puts("STUB: SLSAcceleratorForDisplayAlias called");
    return NULL;
}

void* SLSAcceleratorForDisplayNumber(void)
{
    if (verbose) puts("STUB: SLSAcceleratorForDisplayNumber called");
    return NULL;
}

void* SLSAccessWindowBackingStore(void)
{
    if (verbose) puts("STUB: SLSAccessWindowBackingStore called");
    return NULL;
}

void* SLSActuateDeviceWithPattern(void)
{
    if (verbose) puts("STUB: SLSActuateDeviceWithPattern called");
    return NULL;
}

void* SLSAddActivationRegion(void)
{
    if (verbose) puts("STUB: SLSAddActivationRegion called");
    return NULL;
}

void* SLSAddButtonRegionInWindow(void)
{
    if (verbose) puts("STUB: SLSAddButtonRegionInWindow called");
    return NULL;
}

void* SLSAddDragRegion(void)
{
    if (verbose) puts("STUB: SLSAddDragRegion called");
    return NULL;
}

void* SLSAddDragRegionInWindow(void)
{
    if (verbose) puts("STUB: SLSAddDragRegionInWindow called");
    return NULL;
}

void* SLSAddStructuralRegion(void)
{
    if (verbose) puts("STUB: SLSAddStructuralRegion called");
    return NULL;
}

void* SLSAddSurface(void)
{
    if (verbose) puts("STUB: SLSAddSurface called");
    return NULL;
}

void* SLSAddTrackingArea(void)
{
    if (verbose) puts("STUB: SLSAddTrackingArea called");
    return NULL;
}

void* SLSAddTrackingAreas(void)
{
    if (verbose) puts("STUB: SLSAddTrackingAreas called");
    return NULL;
}

void* SLSAddTrackingRect(void)
{
    if (verbose) puts("STUB: SLSAddTrackingRect called");
    return NULL;
}

void* SLSAddTrackingRegion(void)
{
    if (verbose) puts("STUB: SLSAddTrackingRegion called");
    return NULL;
}

void* SLSAddWindowFilter(void)
{
    if (verbose) puts("STUB: SLSAddWindowFilter called");
    return NULL;
}

void* SLSAddWindowToWindowMovementGroup(void)
{
    if (verbose) puts("STUB: SLSAddWindowToWindowMovementGroup called");
    return NULL;
}

void* SLSAddWindowToWindowOrderingGroup(void)
{
    if (verbose) puts("STUB: SLSAddWindowToWindowOrderingGroup called");
    return NULL;
}

void* SLSAddWindowsToSpaces(void)
{
    if (verbose) puts("STUB: SLSAddWindowsToSpaces called");
    return NULL;
}

void* SLSAdjustSystemStatusBarWindows(void)
{
    if (verbose) puts("STUB: SLSAdjustSystemStatusBarWindows called");
    return NULL;
}

void* SLSAreEventsCaptured(void)
{
    if (verbose) puts("STUB: SLSAreEventsCaptured called");
    return NULL;
}

void* SLSAreEventsCapturedReturningMask(void)
{
    if (verbose) puts("STUB: SLSAreEventsCapturedReturningMask called");
    return NULL;
}

void* SLSAssociateMouseAndMouseCursorPosition(void)
{
    if (verbose) puts("STUB: SLSAssociateMouseAndMouseCursorPosition called");
    return NULL;
}

void* SLSBeginDisplayConfiguration(void)
{
    if (verbose) puts("STUB: SLSBeginDisplayConfiguration called");
    return NULL;
}

void* SLSBindSurface(void)
{
    if (verbose) puts("STUB: SLSBindSurface called");
    return NULL;
}

void* SLSBlockWindowOrdering(void)
{
    if (verbose) puts("STUB: SLSBlockWindowOrdering called");
    return NULL;
}

void* SLSBluishYellow(void)
{
    if (verbose) puts("STUB: SLSBluishYellow called");
    return NULL;
}

void* SLSBoundsEqualToBounds(void)
{
    if (verbose) puts("STUB: SLSBoundsEqualToBounds called");
    return NULL;
}

void* SLSBoundsIntersection(void)
{
    if (verbose) puts("STUB: SLSBoundsIntersection called");
    return NULL;
}

void* SLSBoundsUnion(void)
{
    if (verbose) puts("STUB: SLSBoundsUnion called");
    return NULL;
}

void* SLSBringAirDisplayOffline(void)
{
    if (verbose) puts("STUB: SLSBringAirDisplayOffline called");
    return NULL;
}

void* SLSBringAirDisplayOnline(void)
{
    if (verbose) puts("STUB: SLSBringAirDisplayOnline called");
    return NULL;
}

void* SLSCanCoalesceEvents(void)
{
    if (verbose) puts("STUB: SLSCanCoalesceEvents called");
    return NULL;
}

void* SLSCancelDisplayConfiguration(void)
{
    if (verbose) puts("STUB: SLSCancelDisplayConfiguration called");
    return NULL;
}

void* SLSCaptureDisplay(void)
{
    if (verbose) puts("STUB: SLSCaptureDisplay called");
    return NULL;
}

void* SLSCaptureWindowsContentsToRect(void)
{
    if (verbose) puts("STUB: SLSCaptureWindowsContentsToRect called");
    return NULL;
}

void* SLSCaptureWindowsContentsToRectWithOptions(void)
{
    if (verbose) puts("STUB: SLSCaptureWindowsContentsToRectWithOptions called");
    return NULL;
}

void* SLSClearActivationRegion(void)
{
    if (verbose) puts("STUB: SLSClearActivationRegion called");
    return NULL;
}

void* SLSClearButtonRegionInWindow(void)
{
    if (verbose) puts("STUB: SLSClearButtonRegionInWindow called");
    return NULL;
}

void* SLSClearDragRegion(void)
{
    if (verbose) puts("STUB: SLSClearDragRegion called");
    return NULL;
}

void* SLSClearEvent(void)
{
    if (verbose) puts("STUB: SLSClearEvent called");
    return NULL;
}

void* SLSClearEventOfLength(void)
{
    if (verbose) puts("STUB: SLSClearEventOfLength called");
    return NULL;
}

void* SLSClearWindowListTags(void)
{
    if (verbose) puts("STUB: SLSClearWindowListTags called");
    return NULL;
}

void* SLSClearWindowMovementGroup(void)
{
    if (verbose) puts("STUB: SLSClearWindowMovementGroup called");
    return NULL;
}

void* SLSClearWindowOrderingGroup(void)
{
    if (verbose) puts("STUB: SLSClearWindowOrderingGroup called");
    return NULL;
}

void* SLSClearWindowTags(void)
{
    if (verbose) puts("STUB: SLSClearWindowTags called");
    return NULL;
}

void* SLSCoalesceEvents(void)
{
    if (verbose) puts("STUB: SLSCoalesceEvents called");
    return NULL;
}

void* SLSCoalesceEventsInMask(void)
{
    if (verbose) puts("STUB: SLSCoalesceEventsInMask called");
    return NULL;
}

void* SLSCompleteDisplayConfiguration(void)
{
    if (verbose) puts("STUB: SLSCompleteDisplayConfiguration called");
    return NULL;
}

void* SLSCompleteDisplayConfigurationWithOption(void)
{
    if (verbose) puts("STUB: SLSCompleteDisplayConfigurationWithOption called");
    return NULL;
}

void* SLSCompleteWindowRightsGrant(void)
{
    if (verbose) puts("STUB: SLSCompleteWindowRightsGrant called");
    return NULL;
}

void* SLSConfigureDisplayEnabled(void)
{
    if (verbose) puts("STUB: SLSConfigureDisplayEnabled called");
    return NULL;
}

void* SLSConfigureDisplayFadeEffect(void)
{
    if (verbose) puts("STUB: SLSConfigureDisplayFadeEffect called");
    return NULL;
}

void* SLSConfigureDisplayMirrorOfDisplay(void)
{
    if (verbose) puts("STUB: SLSConfigureDisplayMirrorOfDisplay called");
    return NULL;
}

void* SLSConfigureDisplayMirrored(void)
{
    if (verbose) puts("STUB: SLSConfigureDisplayMirrored called");
    return NULL;
}

void* SLSConfigureDisplayMode(void)
{
    if (verbose) puts("STUB: SLSConfigureDisplayMode called");
    return NULL;
}

void* SLSConfigureDisplayOrigin(void)
{
    if (verbose) puts("STUB: SLSConfigureDisplayOrigin called");
    return NULL;
}

void* SLSConfigureDisplayResolution(void)
{
    if (verbose) puts("STUB: SLSConfigureDisplayResolution called");
    return NULL;
}

void* SLSConfigureDisplayStereoOperation(void)
{
    if (verbose) puts("STUB: SLSConfigureDisplayStereoOperation called");
    return NULL;
}

void* SLSConnectionDispatchNotificationsToMainQueueIfNotMainThread(void)
{
    if (verbose) puts("STUB: SLSConnectionDispatchNotificationsToMainQueueIfNotMainThread called");
    return NULL;
}

void* SLSConnectionGetHeldWindowRights(void)
{
    if (verbose) puts("STUB: SLSConnectionGetHeldWindowRights called");
    return NULL;
}

void* SLSConnectionGetPID(void)
{
    if (verbose) puts("STUB: SLSConnectionGetPID called");
    return NULL;
}

void* SLSConnectionGetPIDAndVersion(void)
{
    if (verbose) puts("STUB: SLSConnectionGetPIDAndVersion called");
    return NULL;
}

void* SLSConnectionRelinquishWindowRights(void)
{
    if (verbose) puts("STUB: SLSConnectionRelinquishWindowRights called");
    return NULL;
}

void* SLSConnectionSetSpinning(void)
{
    if (verbose) puts("STUB: SLSConnectionSetSpinning called");
    return NULL;
}

void* SLSCopyActiveMenuBarDisplayIdentifier(void)
{
    if (verbose) puts("STUB: SLSCopyActiveMenuBarDisplayIdentifier called");
    return NULL;
}

void* SLSCopyAllSessionPropertiesTemporaryBridge(void)
{
    if (verbose) puts("STUB: SLSCopyAllSessionPropertiesTemporaryBridge called");
    return NULL;
}

void* SLSCopyAssociatedWindows(void)
{
    if (verbose) puts("STUB: SLSCopyAssociatedWindows called");
    return NULL;
}

void* SLSCopyAvailableHMDList(void)
{
    if (verbose) puts("STUB: SLSCopyAvailableHMDList called");
    return NULL;
}

void* SLSCopyBestManagedDisplayForPoint(void)
{
    if (verbose) puts("STUB: SLSCopyBestManagedDisplayForPoint called");
    return NULL;
}

void* SLSCopyBestManagedDisplayForRect(void)
{
    if (verbose) puts("STUB: SLSCopyBestManagedDisplayForRect called");
    return NULL;
}

void* SLSCopyConnectionProperty(void)
{
    if (verbose) puts("STUB: SLSCopyConnectionProperty called");
    return NULL;
}

void* SLSCopyCoordinatedDistributedNotificationContinuationBlock(void)
{
    if (verbose) puts("STUB: SLSCopyCoordinatedDistributedNotificationContinuationBlock called");
    return NULL;
}

void* SLSCopyCurrentSessionDictionary(void)
{
    if (verbose) puts("STUB: SLSCopyCurrentSessionDictionary called");
    return NULL;
}

void* SLSCopyDevicesDictionary(void)
{
    if (verbose) puts("STUB: SLSCopyDevicesDictionary called");
    return NULL;
}

void* SLSCopyDisplayColorSpace(void)
{
    if (verbose) puts("STUB: SLSCopyDisplayColorSpace called");
    return NULL;
}

void* SLSCopyDisplayInfoDictionary(void)
{
    if (verbose) puts("STUB: SLSCopyDisplayInfoDictionary called");
    return NULL;
}

void* SLSCopyDisplayModePixelEncoding(void)
{
    if (verbose) puts("STUB: SLSCopyDisplayModePixelEncoding called");
    return NULL;
}

void* SLSCopyDisplayRegion(void)
{
    if (verbose) puts("STUB: SLSCopyDisplayRegion called");
    return NULL;
}

void* SLSCopyDisplayUUID(void)
{
    if (verbose) puts("STUB: SLSCopyDisplayUUID called");
    return NULL;
}

void* SLSCopyManagedDisplayForSpace(void)
{
    if (verbose) puts("STUB: SLSCopyManagedDisplayForSpace called");
    return NULL;
}

void* SLSCopyManagedDisplayForWindow(void)
{
    if (verbose) puts("STUB: SLSCopyManagedDisplayForWindow called");
    return NULL;
}

void* SLSCopyManagedDisplaySpaces(void)
{
    if (verbose) puts("STUB: SLSCopyManagedDisplaySpaces called");
    return NULL;
}

void* SLSCopyManagedDisplays(void)
{
    if (verbose) puts("STUB: SLSCopyManagedDisplays called");
    return NULL;
}

void* SLSCopyPackagesPersistenceDictionary(void)
{
    if (verbose) puts("STUB: SLSCopyPackagesPersistenceDictionary called");
    return NULL;
}

void* SLSCopyRegisteredCursorImages(void)
{
    if (verbose) puts("STUB: SLSCopyRegisteredCursorImages called");
    return NULL;
}

void* SLSCopySessionList(void)
{
    if (verbose) puts("STUB: SLSCopySessionList called");
    return NULL;
}

void* SLSCopySessionPropertiesTemporaryBridge(void)
{
    if (verbose) puts("STUB: SLSCopySessionPropertiesTemporaryBridge called");
    return NULL;
}

void* SLSCopySpaces(void)
{
    if (verbose) puts("STUB: SLSCopySpaces called");
    return NULL;
}

void* SLSCopySpacesForWindows(void)
{
    if (verbose) puts("STUB: SLSCopySpacesForWindows called");
    return NULL;
}

void* SLSCopySpecifiedWindowShape(void)
{
    if (verbose) puts("STUB: SLSCopySpecifiedWindowShape called");
    return NULL;
}

void* SLSCopySurfaceColorSpace(void)
{
    if (verbose) puts("STUB: SLSCopySurfaceColorSpace called");
    return NULL;
}

void* SLSCopyWindowAlphaShape(void)
{
    if (verbose) puts("STUB: SLSCopyWindowAlphaShape called");
    return NULL;
}

void* SLSCopyWindowClipShape(void)
{
    if (verbose) puts("STUB: SLSCopyWindowClipShape called");
    return NULL;
}

void* SLSCopyWindowColorSpace(void)
{
    if (verbose) puts("STUB: SLSCopyWindowColorSpace called");
    return NULL;
}

void* SLSCopyWindowGroup(void)
{
    if (verbose) puts("STUB: SLSCopyWindowGroup called");
    return NULL;
}

void* SLSCopyWindowProperty(void)
{
    if (verbose) puts("STUB: SLSCopyWindowProperty called");
    return NULL;
}

void* SLSCopyWindowRoutingRecordsForScreenLocation(void)
{
    if (verbose) puts("STUB: SLSCopyWindowRoutingRecordsForScreenLocation called");
    return NULL;
}

void* SLSCopyWindowShape(void)
{
    if (verbose) puts("STUB: SLSCopyWindowShape called");
    return NULL;
}

void* SLSCopyWindowsWithOptionsAndTags(void)
{
    if (verbose) puts("STUB: SLSCopyWindowsWithOptionsAndTags called");
    return NULL;
}

void* SLSCopyWindowsWithOptionsAndTagsAndSpaceOptions(void)
{
    if (verbose) puts("STUB: SLSCopyWindowsWithOptionsAndTagsAndSpaceOptions called");
    return NULL;
}

void* SLSCosmeticShieldCursor(void)
{
    if (verbose) puts("STUB: SLSCosmeticShieldCursor called");
    return NULL;
}

void* SLSCreateCinematicBootWindowArray(void)
{
    if (verbose) puts("STUB: SLSCreateCinematicBootWindowArray called");
    return NULL;
}

void* SLSCreateDefaultMetalDevice(void)
{
    if (verbose) puts("STUB: SLSCreateDefaultMetalDevice called");
    return NULL;
}

void* SLSCreateGenieWindowAnimation(void)
{
    if (verbose) puts("STUB: SLSCreateGenieWindowAnimation called");
    return NULL;
}

void* SLSCreateKeyboardEvent(void)
{
    if (verbose) puts("STUB: SLSCreateKeyboardEvent called");
    return NULL;
}

void* SLSCreateKeyboardEventOfLength(void)
{
    if (verbose) puts("STUB: SLSCreateKeyboardEventOfLength called");
    return NULL;
}

void* SLSCreateLayerContext(void)
{
    if (verbose) puts("STUB: SLSCreateLayerContext called");
    return NULL;
}

void* SLSCreateLoginSession(void)
{
    if (verbose) puts("STUB: SLSCreateLoginSession called");
    return NULL;
}

void* SLSCreateLoginSessionWithDataAndVisibility(void)
{
    if (verbose) puts("STUB: SLSCreateLoginSessionWithDataAndVisibility called");
    return NULL;
}

void* SLSCreateMetalSheetWindowAnimationWithParent(void)
{
    if (verbose) puts("STUB: SLSCreateMetalSheetWindowAnimationWithParent called");
    return NULL;
}

void* SLSCreateMetalSheetWindowAnimationWithParentAndShift(void)
{
    if (verbose) puts("STUB: SLSCreateMetalSheetWindowAnimationWithParentAndShift called");
    return NULL;
}

void* SLSCreateRegisteredCursorImage(void)
{
    if (verbose) puts("STUB: SLSCreateRegisteredCursorImage called");
    return NULL;
}

void* SLSCreateSession(void)
{
    if (verbose) puts("STUB: SLSCreateSession called");
    return NULL;
}

void* SLSCreateSessionTemporaryBridge(void)
{
    if (verbose) puts("STUB: SLSCreateSessionTemporaryBridge called");
    return NULL;
}

void* SLSCreateSessionWithDataAndOptions(void)
{
    if (verbose) puts("STUB: SLSCreateSessionWithDataAndOptions called");
    return NULL;
}

void* SLSCreateSessionWithDataAndVisibility(void)
{
    if (verbose) puts("STUB: SLSCreateSessionWithDataAndVisibility called");
    return NULL;
}

void* SLSCreateShadowCacheDebugInfo(void)
{
    if (verbose) puts("STUB: SLSCreateShadowCacheDebugInfo called");
    return NULL;
}

void* SLSCreateShadowGroup(void)
{
    if (verbose) puts("STUB: SLSCreateShadowGroup called");
    return NULL;
}

void* SLSCreateSheetWindowAnimation(void)
{
    if (verbose) puts("STUB: SLSCreateSheetWindowAnimation called");
    return NULL;
}

void* SLSCreateSheetWindowAnimationWithParent(void)
{
    if (verbose) puts("STUB: SLSCreateSheetWindowAnimationWithParent called");
    return NULL;
}

void* SLSCreateWindowDebugInfo(void)
{
    if (verbose) puts("STUB: SLSCreateWindowDebugInfo called");
    return NULL;
}

void* SLSCurrentCursorSeed(void)
{
    if (verbose) puts("STUB: SLSCurrentCursorSeed called");
    return NULL;
}

void* SLSCurrentEventFlags(void)
{
    if (verbose) puts("STUB: SLSCurrentEventFlags called");
    return NULL;
}

void* SLSCurrentEventMask(void)
{
    if (verbose) puts("STUB: SLSCurrentEventMask called");
    return NULL;
}

void* SLSCurrentEventTimestamp(void)
{
    if (verbose) puts("STUB: SLSCurrentEventTimestamp called");
    return NULL;
}

void* SLSCurrentInputPointerPosition(void)
{
    if (verbose) puts("STUB: SLSCurrentInputPointerPosition called");
    return NULL;
}

void* SLSCurrentKeyboardEventSeed(void)
{
    if (verbose) puts("STUB: SLSCurrentKeyboardEventSeed called");
    return NULL;
}

void* SLSCurrentRealTime(void)
{
    if (verbose) puts("STUB: SLSCurrentRealTime called");
    return NULL;
}

void* SLSCursorNameForSystemCursor(void)
{
    if (verbose) puts("STUB: SLSCursorNameForSystemCursor called");
    return NULL;
}

void* SLSCycleWindows(void)
{
    if (verbose) puts("STUB: SLSCycleWindows called");
    return NULL;
}

void* SLSDFRAgentRegister(void)
{
    if (verbose) puts("STUB: SLSDFRAgentRegister called");
    return NULL;
}

void* SLSDFRCopyProperties(void)
{
    if (verbose) puts("STUB: SLSDFRCopyProperties called");
    return NULL;
}

void* SLSDFRDisplayStreamCreate(void)
{
    if (verbose) puts("STUB: SLSDFRDisplayStreamCreate called");
    return NULL;
}

void* SLSDFRSetDimmingStep(void)
{
    if (verbose) puts("STUB: SLSDFRSetDimmingStep called");
    return NULL;
}

void* SLSDFRSetPreferences(void)
{
    if (verbose) puts("STUB: SLSDFRSetPreferences called");
    return NULL;
}

void* SLSDeactivateCurrContext(void)
{
    if (verbose) puts("STUB: SLSDeactivateCurrContext called");
    return NULL;
}

void* SLSDefaultConnectionForThread(void)
{
    if (verbose) puts("STUB: SLSDefaultConnectionForThread called");
    return NULL;
}

void* SLSDefaultKeyboardID(void)
{
    if (verbose) puts("STUB: SLSDefaultKeyboardID called");
    return NULL;
}

void* SLSDeferWindowMoveEvents(void)
{
    if (verbose) puts("STUB: SLSDeferWindowMoveEvents called");
    return NULL;
}

void* SLSDeferredUpdates(void)
{
    if (verbose) puts("STUB: SLSDeferredUpdates called");
    return NULL;
}

void* SLSDetectDisplays(void)
{
    if (verbose) puts("STUB: SLSDetectDisplays called");
    return NULL;
}

void* SLSDeviceCopyColorSpace(void)
{
    if (verbose) puts("STUB: SLSDeviceCopyColorSpace called");
    return NULL;
}

void* SLSDeviceCreate(void)
{
    if (verbose) puts("STUB: SLSDeviceCreate called");
    return NULL;
}

void* SLSDeviceDestroy(void)
{
    if (verbose) puts("STUB: SLSDeviceDestroy called");
    return NULL;
}

void* SLSDeviceFinish(void)
{
    if (verbose) puts("STUB: SLSDeviceFinish called");
    return NULL;
}

void* SLSDeviceGetGeometry(void)
{
    if (verbose) puts("STUB: SLSDeviceGetGeometry called");
    return NULL;
}

void* SLSDeviceGetProfile(void)
{
    if (verbose) puts("STUB: SLSDeviceGetProfile called");
    return NULL;
}

void* SLSDeviceGetResolution(void)
{
    if (verbose) puts("STUB: SLSDeviceGetResolution called");
    return NULL;
}

void* SLSDeviceInvalidate(void)
{
    if (verbose) puts("STUB: SLSDeviceInvalidate called");
    return NULL;
}

void* SLSDeviceLock(void)
{
    if (verbose) puts("STUB: SLSDeviceLock called");
    return NULL;
}

void* SLSDeviceScroll(void)
{
    if (verbose) puts("STUB: SLSDeviceScroll called");
    return NULL;
}

void* SLSDeviceSynchronize(void)
{
    if (verbose) puts("STUB: SLSDeviceSynchronize called");
    return NULL;
}

void* SLSDeviceUnlock(void)
{
    if (verbose) puts("STUB: SLSDeviceUnlock called");
    return NULL;
}

void* SLSDisableCursorObscure(void)
{
    if (verbose) puts("STUB: SLSDisableCursorObscure called");
    return NULL;
}

void* SLSDisableDisableUpdate(void)
{
    if (verbose) puts("STUB: SLSDisableDisableUpdate called");
    return NULL;
}

void* SLSDisableUpdate(void)
{
    if (verbose) puts("STUB: SLSDisableUpdate called");
    return NULL;
}

void* SLSDisableUpdateToken(void)
{
    if (verbose) puts("STUB: SLSDisableUpdateToken called");
    return NULL;
}

void* SLSDisplayChangedSeed(void)
{
    if (verbose) puts("STUB: SLSDisplayChangedSeed called");
    return NULL;
}

void* SLSDisplayConfigurationOrigin(void)
{
    if (verbose) puts("STUB: SLSDisplayConfigurationOrigin called");
    return NULL;
}

void* SLSDisplayCopyCurrentMetalDevice(void)
{
    if (verbose) puts("STUB: SLSDisplayCopyCurrentMetalDevice called");
    return NULL;
}

void* SLSDisplayGetRefreshTiming(void)
{
    if (verbose) puts("STUB: SLSDisplayGetRefreshTiming called");
    return NULL;
}

void* SLSDisplayLoadUserPreferences(void)
{
    if (verbose) puts("STUB: SLSDisplayLoadUserPreferences called");
    return NULL;
}

void* SLSDisplayMappingChangedSeed(void)
{
    if (verbose) puts("STUB: SLSDisplayMappingChangedSeed called");
    return NULL;
}

void* SLSDisplayMuxAcquired(void)
{
    if (verbose) puts("STUB: SLSDisplayMuxAcquired called");
    return NULL;
}

void* SLSDisplayMuxReleased(void)
{
    if (verbose) puts("STUB: SLSDisplayMuxReleased called");
    return NULL;
}

void* SLSDisplayRegisterReconfigurationCallbackWithUUID(void)
{
    if (verbose) puts("STUB: SLSDisplayRegisterReconfigurationCallbackWithUUID called");
    return NULL;
}

void* SLSDisplayStatusQuery(void)
{
    if (verbose) puts("STUB: SLSDisplayStatusQuery called");
    return NULL;
}

void* SLSDisplayStreamCreateCustom(void)
{
    if (verbose) puts("STUB: SLSDisplayStreamCreateCustom called");
    return NULL;
}

void* SLSDisplayStreamSidecarTouchBar(void)
{
    if (verbose) puts("STUB: SLSDisplayStreamSidecarTouchBar called");
    return NULL;
}

void* SLSDisplayStreamUpdateCreate(void)
{
    if (verbose) puts("STUB: SLSDisplayStreamUpdateCreate called");
    return NULL;
}

void* SLSDisplaySystemStateSeed(void)
{
    if (verbose) puts("STUB: SLSDisplaySystemStateSeed called");
    return NULL;
}

void* SLSDoDeferredOrdering(void)
{
    if (verbose) puts("STUB: SLSDoDeferredOrdering called");
    return NULL;
}

void* SLSDragWindowRelativeToMouse(void)
{
    if (verbose) puts("STUB: SLSDragWindowRelativeToMouse called");
    return NULL;
}

void* SLSEnableAmbientDisplay(void)
{
    if (verbose) puts("STUB: SLSEnableAmbientDisplay called");
    return NULL;
}

void* SLSEnableDeferredUpdateNotifications(void)
{
    if (verbose) puts("STUB: SLSEnableDeferredUpdateNotifications called");
    return NULL;
}

void* SLSEnableHDR(void)
{
    if (verbose) puts("STUB: SLSEnableHDR called");
    return NULL;
}

void* SLSEnsureSpaceSwitchToActiveProcess(void)
{
    if (verbose) puts("STUB: SLSEnsureSpaceSwitchToActiveProcess called");
    return NULL;
}

void* SLSEventAppUnresponsiveStatus(void)
{
    if (verbose) puts("STUB: SLSEventAppUnresponsiveStatus called");
    return NULL;
}

void* SLSEventCounter(void)
{
    if (verbose) puts("STUB: SLSEventCounter called");
    return NULL;
}

void* SLSEventFlagForVirtualKeyCode(void)
{
    if (verbose) puts("STUB: SLSEventFlagForVirtualKeyCode called");
    return NULL;
}

void* SLSEventIsAppUnresponsive(void)
{
    if (verbose) puts("STUB: SLSEventIsAppUnresponsive called");
    return NULL;
}

void* SLSEventPostEventToPSN(void)
{
    if (verbose) puts("STUB: SLSEventPostEventToPSN called");
    return NULL;
}

void* SLSEventRecordGetContextID(void)
{
    if (verbose) puts("STUB: SLSEventRecordGetContextID called");
    return NULL;
}

void* SLSEventRecordGetContextOrigin(void)
{
    if (verbose) puts("STUB: SLSEventRecordGetContextOrigin called");
    return NULL;
}

void* SLSEventRecordGetDFRTouchTimestamp(void)
{
    if (verbose) puts("STUB: SLSEventRecordGetDFRTouchTimestamp called");
    return NULL;
}

void* SLSEventRecordGetStagePressure(void)
{
    if (verbose) puts("STUB: SLSEventRecordGetStagePressure called");
    return NULL;
}

void* SLSEventRecordGetTransitionProgress(void)
{
    if (verbose) puts("STUB: SLSEventRecordGetTransitionProgress called");
    return NULL;
}

void* SLSEventRecordGetUnacceleratedPointerMoveX(void)
{
    if (verbose) puts("STUB: SLSEventRecordGetUnacceleratedPointerMoveX called");
    return NULL;
}

void* SLSEventRecordGetUnacceleratedPointerMoveY(void)
{
    if (verbose) puts("STUB: SLSEventRecordGetUnacceleratedPointerMoveY called");
    return NULL;
}

void* SLSEventRecordGetUnmodifiedTimestamp(void)
{
    if (verbose) puts("STUB: SLSEventRecordGetUnmodifiedTimestamp called");
    return NULL;
}

void* SLSEventRecordLength(void)
{
    if (verbose) puts("STUB: SLSEventRecordLength called");
    return NULL;
}

void* SLSEventRecordSetContextID(void)
{
    if (verbose) puts("STUB: SLSEventRecordSetContextID called");
    return NULL;
}

void* SLSEventRecordSetContextOrigin(void)
{
    if (verbose) puts("STUB: SLSEventRecordSetContextOrigin called");
    return NULL;
}

void* SLSEventRecordSetDFRTouchTimestamp(void)
{
    if (verbose) puts("STUB: SLSEventRecordSetDFRTouchTimestamp called");
    return NULL;
}

void* SLSEventRecordSetUnacceleratedPointerMoveX(void)
{
    if (verbose) puts("STUB: SLSEventRecordSetUnacceleratedPointerMoveX called");
    return NULL;
}

void* SLSEventRecordSetUnacceleratedPointerMoveY(void)
{
    if (verbose) puts("STUB: SLSEventRecordSetUnacceleratedPointerMoveY called");
    return NULL;
}

void* SLSEventRecordSetUnmodifiedTimestamp(void)
{
    if (verbose) puts("STUB: SLSEventRecordSetUnmodifiedTimestamp called");
    return NULL;
}

void* SLSEventRecordTimeFromRealTimeStamp(void)
{
    if (verbose) puts("STUB: SLSEventRecordTimeFromRealTimeStamp called");
    return NULL;
}

void* SLSEventSetAppIsUnresponsiveNotificationTimeout(void)
{
    if (verbose) puts("STUB: SLSEventSetAppIsUnresponsiveNotificationTimeout called");
    return NULL;
}

void* SLSEventTapCreate(void)
{
    if (verbose) puts("STUB: SLSEventTapCreate called");
    return NULL;
}

void* SLSExtractEventRecordsFromMessage(void)
{
    if (verbose) puts("STUB: SLSExtractEventRecordsFromMessage called");
    return NULL;
}

void* SLSFindSessionAuditSessionID(void)
{
    if (verbose) puts("STUB: SLSFindSessionAuditSessionID called");
    return NULL;
}

void* SLSFindWindow(void)
{
    if (verbose) puts("STUB: SLSFindWindow called");
    return NULL;
}

void* SLSFindWindowAndOwner(void)
{
    if (verbose) puts("STUB: SLSFindWindowAndOwner called");
    return NULL;
}

void* SLSFindWindowByGeometry(void)
{
    if (verbose) puts("STUB: SLSFindWindowByGeometry called");
    return NULL;
}

void* SLSFlushSurface(void)
{
    if (verbose) puts("STUB: SLSFlushSurface called");
    return NULL;
}

void* SLSFlushSurfaceWithOptions(void)
{
    if (verbose) puts("STUB: SLSFlushSurfaceWithOptions called");
    return NULL;
}

void* SLSFlushSurfaceWithOptionsAndIndex(void)
{
    if (verbose) puts("STUB: SLSFlushSurfaceWithOptionsAndIndex called");
    return NULL;
}

void* SLSFlushWindow(void)
{
    if (verbose) puts("STUB: SLSFlushWindow called");
    return NULL;
}

void* SLSFlushWindowContentRegion(void)
{
    if (verbose) puts("STUB: SLSFlushWindowContentRegion called");
    return NULL;
}

void* SLSForceShowCursor(void)
{
    if (verbose) puts("STUB: SLSForceShowCursor called");
    return NULL;
}

void* SLSForceWaitCursorActive(void)
{
    if (verbose) puts("STUB: SLSForceWaitCursorActive called");
    return NULL;
}

void* SLSGLCompositorSettings(void)
{
    if (verbose) puts("STUB: SLSGLCompositorSettings called");
    return NULL;
}

void* SLSGeometrySeed(void)
{
    if (verbose) puts("STUB: SLSGeometrySeed called");
    return NULL;
}

void* SLSGetActivationRegionLegacy(void)
{
    if (verbose) puts("STUB: SLSGetActivationRegionLegacy called");
    return NULL;
}

void* SLSGetActiveDisplayList(void)
{
    if (verbose) puts("STUB: SLSGetActiveDisplayList called");
    return NULL;
}

void* SLSGetActiveMenuBarDrawingStyle(void)
{
    if (verbose) puts("STUB: SLSGetActiveMenuBarDrawingStyle called");
    return NULL;
}

void* SLSGetActiveSpace(void)
{
    if (verbose) puts("STUB: SLSGetActiveSpace called");
    return NULL;
}

void* SLSGetAppearanceThemeLegacy(void)
{
    if (verbose) puts("STUB: SLSGetAppearanceThemeLegacy called");
    return NULL;
}

void* SLSGetAppearanceThemeSwitchesAutomatically(void)
{
    if (verbose) puts("STUB: SLSGetAppearanceThemeSwitchesAutomatically called");
    return NULL;
}

void* SLSGetBackgroundEventMask(void)
{
    if (verbose) puts("STUB: SLSGetBackgroundEventMask called");
    return NULL;
}

void* SLSGetButtonRegionLegacy(void)
{
    if (verbose) puts("STUB: SLSGetButtonRegionLegacy called");
    return NULL;
}

void* SLSGetCAMetalDevice(void)
{
    if (verbose) puts("STUB: SLSGetCAMetalDevice called");
    return NULL;
}

void* SLSGetCatenatedWindowTransform(void)
{
    if (verbose) puts("STUB: SLSGetCatenatedWindowTransform called");
    return NULL;
}

void* SLSGetCoalesceEventsMask(void)
{
    if (verbose) puts("STUB: SLSGetCoalesceEventsMask called");
    return NULL;
}

void* SLSGetConnectionIDForPSN(void)
{
    if (verbose) puts("STUB: SLSGetConnectionIDForPSN called");
    return NULL;
}

void* SLSGetConnectionPSN(void)
{
    if (verbose) puts("STUB: SLSGetConnectionPSN called");
    return NULL;
}

void* SLSGetCurrentCursorLocation(void)
{
    if (verbose) puts("STUB: SLSGetCurrentCursorLocation called");
    return NULL;
}

void* SLSGetCurrentDisplayMode(void)
{
    if (verbose) puts("STUB: SLSGetCurrentDisplayMode called");
    return NULL;
}

void* SLSGetCurrentMouseButtonState(void)
{
    if (verbose) puts("STUB: SLSGetCurrentMouseButtonState called");
    return NULL;
}

void* SLSGetCursorData(void)
{
    if (verbose) puts("STUB: SLSGetCursorData called");
    return NULL;
}

void* SLSGetCursorDataSize(void)
{
    if (verbose) puts("STUB: SLSGetCursorDataSize called");
    return NULL;
}

void* SLSGetCursorScale(void)
{
    if (verbose) puts("STUB: SLSGetCursorScale called");
    return NULL;
}

void* SLSGetDebugOptions(void)
{
    if (verbose) puts("STUB: SLSGetDebugOptions called");
    return NULL;
}

void* SLSGetDenyWindowServerConnections(void)
{
    if (verbose) puts("STUB: SLSGetDenyWindowServerConnections called");
    return NULL;
}

void* SLSGetDisplayAddress(void)
{
    if (verbose) puts("STUB: SLSGetDisplayAddress called");
    return NULL;
}

void* SLSGetDisplayAliasList(void)
{
    if (verbose) puts("STUB: SLSGetDisplayAliasList called");
    return NULL;
}

void* SLSGetDisplayBaseAddress(void)
{
    if (verbose) puts("STUB: SLSGetDisplayBaseAddress called");
    return NULL;
}

void* SLSGetDisplayBounds(void)
{
    if (verbose) puts("STUB: SLSGetDisplayBounds called");
    return NULL;
}

void* SLSGetDisplayBrightness(void)
{
    if (verbose) puts("STUB: SLSGetDisplayBrightness called");
    return NULL;
}

void* SLSGetDisplayDepth(void)
{
    if (verbose) puts("STUB: SLSGetDisplayDepth called");
    return NULL;
}

void* SLSGetDisplayForUUID(void)
{
    if (verbose) puts("STUB: SLSGetDisplayForUUID called");
    return NULL;
}

void* SLSGetDisplayHardwareBounds(void)
{
    if (verbose) puts("STUB: SLSGetDisplayHardwareBounds called");
    return NULL;
}

void* SLSGetDisplayList(void)
{
    if (verbose) puts("STUB: SLSGetDisplayList called");
    return NULL;
}

void* SLSGetDisplayMaxOpenGLSurfaceSize(void)
{
    if (verbose) puts("STUB: SLSGetDisplayMaxOpenGLSurfaceSize called");
    return NULL;
}

void* SLSGetDisplayMaximumHDRValue(void)
{
    if (verbose) puts("STUB: SLSGetDisplayMaximumHDRValue called");
    return NULL;
}

void* SLSGetDisplayModeDescription(void)
{
    if (verbose) puts("STUB: SLSGetDisplayModeDescription called");
    return NULL;
}

void* SLSGetDisplayModeDescriptionOfLength(void)
{
    if (verbose) puts("STUB: SLSGetDisplayModeDescriptionOfLength called");
    return NULL;
}

void* SLSGetDisplayModeDescriptionsOfLength(void)
{
    if (verbose) puts("STUB: SLSGetDisplayModeDescriptionsOfLength called");
    return NULL;
}

void* SLSGetDisplayOpenGLDisplayAliasMask(void)
{
    if (verbose) puts("STUB: SLSGetDisplayOpenGLDisplayAliasMask called");
    return NULL;
}

void* SLSGetDisplayOpenGLDisplayMask(void)
{
    if (verbose) puts("STUB: SLSGetDisplayOpenGLDisplayMask called");
    return NULL;
}

void* SLSGetDisplayPixelEncodingOfLength(void)
{
    if (verbose) puts("STUB: SLSGetDisplayPixelEncodingOfLength called");
    return NULL;
}

void* SLSGetDisplayPixelFormat(void)
{
    if (verbose) puts("STUB: SLSGetDisplayPixelFormat called");
    return NULL;
}

void* SLSGetDisplayReconfigureTimeWhenWindowsLastMoved(void)
{
    if (verbose) puts("STUB: SLSGetDisplayReconfigureTimeWhenWindowsLastMoved called");
    return NULL;
}

void* SLSGetDisplayRowBytes(void)
{
    if (verbose) puts("STUB: SLSGetDisplayRowBytes called");
    return NULL;
}

void* SLSGetDisplayTransfer(void)
{
    if (verbose) puts("STUB: SLSGetDisplayTransfer called");
    return NULL;
}

void* SLSGetDisplayTransferByFormula(void)
{
    if (verbose) puts("STUB: SLSGetDisplayTransferByFormula called");
    return NULL;
}

void* SLSGetDisplayTransferByTable(void)
{
    if (verbose) puts("STUB: SLSGetDisplayTransferByTable called");
    return NULL;
}

void* SLSGetDisplaysWithOpenGLDisplayMask(void)
{
    if (verbose) puts("STUB: SLSGetDisplaysWithOpenGLDisplayMask called");
    return NULL;
}

void* SLSGetDisplaysWithPoint(void)
{
    if (verbose) puts("STUB: SLSGetDisplaysWithPoint called");
    return NULL;
}

void* SLSGetDisplaysWithRect(void)
{
    if (verbose) puts("STUB: SLSGetDisplaysWithRect called");
    return NULL;
}

void* SLSGetDisplaysWithUUID(void)
{
    if (verbose) puts("STUB: SLSGetDisplaysWithUUID called");
    return NULL;
}

void* SLSGetDockRectWithOrientation(void)
{
    if (verbose) puts("STUB: SLSGetDockRectWithOrientation called");
    return NULL;
}

void* SLSGetDockRectWithReason(void)
{
    if (verbose) puts("STUB: SLSGetDockRectWithReason called");
    return NULL;
}

void* SLSGetDragRegionLegacy(void)
{
    if (verbose) puts("STUB: SLSGetDragRegionLegacy called");
    return NULL;
}

void* SLSGetEligiblePerAppDisplayList(void)
{
    if (verbose) puts("STUB: SLSGetEligiblePerAppDisplayList called");
    return NULL;
}

void* SLSGetEventPort(void)
{
    if (verbose) puts("STUB: SLSGetEventPort called");
    return NULL;
}

void* SLSGetEventTapList(void)
{
    if (verbose) puts("STUB: SLSGetEventTapList called");
    return NULL;
}

void* SLSGetEventTapTimeout(void)
{
    if (verbose) puts("STUB: SLSGetEventTapTimeout called");
    return NULL;
}

void* SLSGetGestureEventSubmask(void)
{
    if (verbose) puts("STUB: SLSGetGestureEventSubmask called");
    return NULL;
}

void* SLSGetGlobalCursorData(void)
{
    if (verbose) puts("STUB: SLSGetGlobalCursorData called");
    return NULL;
}

void* SLSGetGlobalCursorDataSize(void)
{
    if (verbose) puts("STUB: SLSGetGlobalCursorDataSize called");
    return NULL;
}

void* SLSGetGlobalHotKeyOperatingMode(void)
{
    if (verbose) puts("STUB: SLSGetGlobalHotKeyOperatingMode called");
    return NULL;
}

void* SLSGetHIDEventRecord(void)
{
    if (verbose) puts("STUB: SLSGetHIDEventRecord called");
    return NULL;
}

void* SLSGetHotButton(void)
{
    if (verbose) puts("STUB: SLSGetHotButton called");
    return NULL;
}

void* SLSGetHotKey(void)
{
    if (verbose) puts("STUB: SLSGetHotKey called");
    return NULL;
}

void* SLSGetHotKeyRepresentation(void)
{
    if (verbose) puts("STUB: SLSGetHotKeyRepresentation called");
    return NULL;
}

void* SLSGetHotKeyType(void)
{
    if (verbose) puts("STUB: SLSGetHotKeyType called");
    return NULL;
}

void* SLSGetHotModifier(void)
{
    if (verbose) puts("STUB: SLSGetHotModifier called");
    return NULL;
}

void* SLSGetInkingState(void)
{
    if (verbose) puts("STUB: SLSGetInkingState called");
    return NULL;
}

void* SLSGetKeys(void)
{
    if (verbose) puts("STUB: SLSGetKeys called");
    return NULL;
}

void* SLSGetLoginName(void)
{
    if (verbose) puts("STUB: SLSGetLoginName called");
    return NULL;
}

void* SLSGetMagicZoomState(void)
{
    if (verbose) puts("STUB: SLSGetMagicZoomState called");
    return NULL;
}

void* SLSGetMagicZoomWindowID(void)
{
    if (verbose) puts("STUB: SLSGetMagicZoomWindowID called");
    return NULL;
}

void* SLSGetMenuBar(void)
{
    if (verbose) puts("STUB: SLSGetMenuBar called");
    return NULL;
}

void* SLSGetMenuBarAutohideEnabled(void)
{
    if (verbose) puts("STUB: SLSGetMenuBarAutohideEnabled called");
    return NULL;
}

void* SLSGetMenuBarDrawingStyle(void)
{
    if (verbose) puts("STUB: SLSGetMenuBarDrawingStyle called");
    return NULL;
}

void* SLSGetMenuBarUsesRTLLayoutOnDisplayLegacy(void)
{
    if (verbose) puts("STUB: SLSGetMenuBarUsesRTLLayoutOnDisplayLegacy called");
    return NULL;
}

void* SLSGetMenuBarVisibilityOverrideOnDisplay(void)
{
    if (verbose) puts("STUB: SLSGetMenuBarVisibilityOverrideOnDisplay called");
    return NULL;
}

void* SLSGetNextEventRecord(void)
{
    if (verbose) puts("STUB: SLSGetNextEventRecord called");
    return NULL;
}

void* SLSGetNextEventRecordFromMessage(void)
{
    if (verbose) puts("STUB: SLSGetNextEventRecordFromMessage called");
    return NULL;
}

void* SLSGetNumberOfDisplayModes(void)
{
    if (verbose) puts("STUB: SLSGetNumberOfDisplayModes called");
    return NULL;
}

void* SLSGetNumberOfDisplays(void)
{
    if (verbose) puts("STUB: SLSGetNumberOfDisplays called");
    return NULL;
}

void* SLSGetOnScreenWindowCount(void)
{
    if (verbose) puts("STUB: SLSGetOnScreenWindowCount called");
    return NULL;
}

void* SLSGetOnScreenWindowList(void)
{
    if (verbose) puts("STUB: SLSGetOnScreenWindowList called");
    return NULL;
}

void* SLSGetOnlineDisplayList(void)
{
    if (verbose) puts("STUB: SLSGetOnlineDisplayList called");
    return NULL;
}

void* SLSGetOnscreenWindowBounds(void)
{
    if (verbose) puts("STUB: SLSGetOnscreenWindowBounds called");
    return NULL;
}

void* SLSGetParentWindowList(void)
{
    if (verbose) puts("STUB: SLSGetParentWindowList called");
    return NULL;
}

void* SLSGetPerformanceData(void)
{
    if (verbose) puts("STUB: SLSGetPerformanceData called");
    return NULL;
}

void* SLSGetPerformanceTotalUpdateCount(void)
{
    if (verbose) puts("STUB: SLSGetPerformanceTotalUpdateCount called");
    return NULL;
}

void* SLSGetPerformanceTotalUpdateHistogram(void)
{
    if (verbose) puts("STUB: SLSGetPerformanceTotalUpdateHistogram called");
    return NULL;
}

void* SLSGetPerformanceUpdateCountPerDisplay(void)
{
    if (verbose) puts("STUB: SLSGetPerformanceUpdateCountPerDisplay called");
    return NULL;
}

void* SLSGetPotentiallyActiveDisplayList(void)
{
    if (verbose) puts("STUB: SLSGetPotentiallyActiveDisplayList called");
    return NULL;
}

void* SLSGetProcessMenuBarWindowList(void)
{
    if (verbose) puts("STUB: SLSGetProcessMenuBarWindowList called");
    return NULL;
}

void* SLSGetProcessValidity(void)
{
    if (verbose) puts("STUB: SLSGetProcessValidity called");
    return NULL;
}

void* SLSGetRealtimeDisplayInfoShmem(void)
{
    if (verbose) puts("STUB: SLSGetRealtimeDisplayInfoShmem called");
    return NULL;
}

void* SLSGetRegisteredCursorData(void)
{
    if (verbose) puts("STUB: SLSGetRegisteredCursorData called");
    return NULL;
}

void* SLSGetRegisteredCursorData2(void)
{
    if (verbose) puts("STUB: SLSGetRegisteredCursorData2 called");
    return NULL;
}

void* SLSGetRegisteredCursorDataSize(void)
{
    if (verbose) puts("STUB: SLSGetRegisteredCursorDataSize called");
    return NULL;
}

void* SLSGetRevealedMenuBarBounds(void)
{
    if (verbose) puts("STUB: SLSGetRevealedMenuBarBounds called");
    return NULL;
}

void* SLSGetScreenRectForWindow(void)
{
    if (verbose) puts("STUB: SLSGetScreenRectForWindow called");
    return NULL;
}

void* SLSGetSharedWindow(void)
{
    if (verbose) puts("STUB: SLSGetSharedWindow called");
    return NULL;
}

void* SLSGetSizeOfDisplayTransfer(void)
{
    if (verbose) puts("STUB: SLSGetSizeOfDisplayTransfer called");
    return NULL;
}

void* SLSGetSpaceManagementMode(void)
{
    if (verbose) puts("STUB: SLSGetSpaceManagementMode called");
    return NULL;
}

void* SLSGetSpaceMenuBarReveal(void)
{
    if (verbose) puts("STUB: SLSGetSpaceMenuBarReveal called");
    return NULL;
}

void* SLSGetSpacePermittedResizeDirections(void)
{
    if (verbose) puts("STUB: SLSGetSpacePermittedResizeDirections called");
    return NULL;
}

void* SLSGetSpecialCommandRegionLegacy(void)
{
    if (verbose) puts("STUB: SLSGetSpecialCommandRegionLegacy called");
    return NULL;
}

void* SLSGetSurfaceBinding(void)
{
    if (verbose) puts("STUB: SLSGetSurfaceBinding called");
    return NULL;
}

void* SLSGetSurfaceBounds(void)
{
    if (verbose) puts("STUB: SLSGetSurfaceBounds called");
    return NULL;
}

void* SLSGetSurfaceCount(void)
{
    if (verbose) puts("STUB: SLSGetSurfaceCount called");
    return NULL;
}

void* SLSGetSurfaceFrontBufferID(void)
{
    if (verbose) puts("STUB: SLSGetSurfaceFrontBufferID called");
    return NULL;
}

void* SLSGetSurfaceList(void)
{
    if (verbose) puts("STUB: SLSGetSurfaceList called");
    return NULL;
}

void* SLSGetSurfaceOpacity(void)
{
    if (verbose) puts("STUB: SLSGetSurfaceOpacity called");
    return NULL;
}

void* SLSGetSurfaceResolution(void)
{
    if (verbose) puts("STUB: SLSGetSurfaceResolution called");
    return NULL;
}

void* SLSGetSymbolicHotKeyButtonValue(void)
{
    if (verbose) puts("STUB: SLSGetSymbolicHotKeyButtonValue called");
    return NULL;
}

void* SLSGetSymbolicHotKeyRepresentation(void)
{
    if (verbose) puts("STUB: SLSGetSymbolicHotKeyRepresentation called");
    return NULL;
}

void* SLSGetSymbolicHotKeyRepresentationList(void)
{
    if (verbose) puts("STUB: SLSGetSymbolicHotKeyRepresentationList called");
    return NULL;
}

void* SLSGetSymbolicHotKeyValue(void)
{
    if (verbose) puts("STUB: SLSGetSymbolicHotKeyValue called");
    return NULL;
}

void* SLSGetSymbolicHotKeyValuesAndStates(void)
{
    if (verbose) puts("STUB: SLSGetSymbolicHotKeyValuesAndStates called");
    return NULL;
}

void* SLSGetSymbolicHotModifierValue(void)
{
    if (verbose) puts("STUB: SLSGetSymbolicHotModifierValue called");
    return NULL;
}

void* SLSGetSystemBackgroundWindow(void)
{
    if (verbose) puts("STUB: SLSGetSystemBackgroundWindow called");
    return NULL;
}

void* SLSGetSystemDefinedCursorData(void)
{
    if (verbose) puts("STUB: SLSGetSystemDefinedCursorData called");
    return NULL;
}

void* SLSGetSystemDefinedCursorDataSize(void)
{
    if (verbose) puts("STUB: SLSGetSystemDefinedCursorDataSize called");
    return NULL;
}

void* SLSGetSystemUIHasRTLDirectionLegacy(void)
{
    if (verbose) puts("STUB: SLSGetSystemUIHasRTLDirectionLegacy called");
    return NULL;
}

void* SLSGetSystemWindow(void)
{
    if (verbose) puts("STUB: SLSGetSystemWindow called");
    return NULL;
}

void* SLSGetTileSpaceDividerDirections(void)
{
    if (verbose) puts("STUB: SLSGetTileSpaceDividerDirections called");
    return NULL;
}

void* SLSGetUnflippedEventLocation(void)
{
    if (verbose) puts("STUB: SLSGetUnflippedEventLocation called");
    return NULL;
}

void* SLSGetUnflippedWindowEventLocation(void)
{
    if (verbose) puts("STUB: SLSGetUnflippedWindowEventLocation called");
    return NULL;
}

void* SLSGetUpdateClientPipe(void)
{
    if (verbose) puts("STUB: SLSGetUpdateClientPipe called");
    return NULL;
}

void* SLSGetWindowAccelerationCapability(void)
{
    if (verbose) puts("STUB: SLSGetWindowAccelerationCapability called");
    return NULL;
}

void* SLSGetWindowAlpha(void)
{
    if (verbose) puts("STUB: SLSGetWindowAlpha called");
    return NULL;
}

void* SLSGetWindowAutofill(void)
{
    if (verbose) puts("STUB: SLSGetWindowAutofill called");
    return NULL;
}

void* SLSGetWindowBounds(void)
{
    if (verbose) puts("STUB: SLSGetWindowBounds called");
    return NULL;
}

void* SLSGetWindowCount(void)
{
    if (verbose) puts("STUB: SLSGetWindowCount called");
    return NULL;
}

void* SLSGetWindowDepth(void)
{
    if (verbose) puts("STUB: SLSGetWindowDepth called");
    return NULL;
}

void* SLSGetWindowEventMask(void)
{
    if (verbose) puts("STUB: SLSGetWindowEventMask called");
    return NULL;
}

void* SLSGetWindowEventReceiver(void)
{
    if (verbose) puts("STUB: SLSGetWindowEventReceiver called");
    return NULL;
}

void* SLSGetWindowFlushSeed(void)
{
    if (verbose) puts("STUB: SLSGetWindowFlushSeed called");
    return NULL;
}

void* SLSGetWindowFrameBounds(void)
{
    if (verbose) puts("STUB: SLSGetWindowFrameBounds called");
    return NULL;
}

void* SLSGetWindowGeometry(void)
{
    if (verbose) puts("STUB: SLSGetWindowGeometry called");
    return NULL;
}

void* SLSGetWindowLayerContext(void)
{
    if (verbose) puts("STUB: SLSGetWindowLayerContext called");
    return NULL;
}

void* SLSGetWindowLevel(void)
{
    if (verbose) puts("STUB: SLSGetWindowLevel called");
    return NULL;
}

void* SLSGetWindowList(void)
{
    if (verbose) puts("STUB: SLSGetWindowList called");
    return NULL;
}

void* SLSGetWindowListWithTags(void)
{
    if (verbose) puts("STUB: SLSGetWindowListWithTags called");
    return NULL;
}

void* SLSGetWindowMouseLocation(void)
{
    if (verbose) puts("STUB: SLSGetWindowMouseLocation called");
    return NULL;
}

void* SLSGetWindowOpacity(void)
{
    if (verbose) puts("STUB: SLSGetWindowOpacity called");
    return NULL;
}

void* SLSGetWindowOwner(void)
{
    if (verbose) puts("STUB: SLSGetWindowOwner called");
    return NULL;
}

void* SLSGetWindowPresenter(void)
{
    if (verbose) puts("STUB: SLSGetWindowPresenter called");
    return NULL;
}

void* SLSGetWindowResizeState(void)
{
    if (verbose) puts("STUB: SLSGetWindowResizeState called");
    return NULL;
}

void* SLSGetWindowResolution(void)
{
    if (verbose) puts("STUB: SLSGetWindowResolution called");
    return NULL;
}

void* SLSGetWindowShadowAndRimParameters(void)
{
    if (verbose) puts("STUB: SLSGetWindowShadowAndRimParameters called");
    return NULL;
}

void* SLSGetWindowSharingState(void)
{
    if (verbose) puts("STUB: SLSGetWindowSharingState called");
    return NULL;
}

void* SLSGetWindowSize(void)
{
    if (verbose) puts("STUB: SLSGetWindowSize called");
    return NULL;
}

void* SLSGetWindowSubLevel(void)
{
    if (verbose) puts("STUB: SLSGetWindowSubLevel called");
    return NULL;
}

void* SLSGetWindowTags(void)
{
    if (verbose) puts("STUB: SLSGetWindowTags called");
    return NULL;
}

void* SLSGetWindowTransform(void)
{
    if (verbose) puts("STUB: SLSGetWindowTransform called");
    return NULL;
}

void* SLSGetWindowTransformAtPlacement(void)
{
    if (verbose) puts("STUB: SLSGetWindowTransformAtPlacement called");
    return NULL;
}

void* SLSGetWindowType(void)
{
    if (verbose) puts("STUB: SLSGetWindowType called");
    return NULL;
}

void* SLSGetWindowWorkspace(void)
{
    if (verbose) puts("STUB: SLSGetWindowWorkspace called");
    return NULL;
}

void* SLSGetWindowWorkspaceIgnoringVisibility(void)
{
    if (verbose) puts("STUB: SLSGetWindowWorkspaceIgnoringVisibility called");
    return NULL;
}

void* SLSGetWorkspace(void)
{
    if (verbose) puts("STUB: SLSGetWorkspace called");
    return NULL;
}

void* SLSGetWorkspaceWindowCountWithOptionsAndTags(void)
{
    if (verbose) puts("STUB: SLSGetWorkspaceWindowCountWithOptionsAndTags called");
    return NULL;
}

void* SLSGetWorkspaceWindowGroup(void)
{
    if (verbose) puts("STUB: SLSGetWorkspaceWindowGroup called");
    return NULL;
}

void* SLSGetWorkspaceWindowListWithOptionsAndTags(void)
{
    if (verbose) puts("STUB: SLSGetWorkspaceWindowListWithOptionsAndTags called");
    return NULL;
}

void* SLSGetZoomDisplay(void)
{
    if (verbose) puts("STUB: SLSGetZoomDisplay called");
    return NULL;
}

void* SLSGetZoomParameters(void)
{
    if (verbose) puts("STUB: SLSGetZoomParameters called");
    return NULL;
}

void* SLSGrantWindowRightsReservingWithTimeoutAndToken(void)
{
    if (verbose) puts("STUB: SLSGrantWindowRightsReservingWithTimeoutAndToken called");
    return NULL;
}

void* SLSHIDEventRecordReplay(void)
{
    if (verbose) puts("STUB: SLSHIDEventRecordReplay called");
    return NULL;
}

void* SLSHMDAcquire(void)
{
    if (verbose) puts("STUB: SLSHMDAcquire called");
    return NULL;
}

void* SLSHMDAddNotificationHandler(void)
{
    if (verbose) puts("STUB: SLSHMDAddNotificationHandler called");
    return NULL;
}

void* SLSHMDConfigureDisplayQueue(void)
{
    if (verbose) puts("STUB: SLSHMDConfigureDisplayQueue called");
    return NULL;
}

void* SLSHMDCopyAudioSelector(void)
{
    if (verbose) puts("STUB: SLSHMDCopyAudioSelector called");
    return NULL;
}

void* SLSHMDCopyCapabilities(void)
{
    if (verbose) puts("STUB: SLSHMDCopyCapabilities called");
    return NULL;
}

void* SLSHMDCopyDisplayInfoDictionary(void)
{
    if (verbose) puts("STUB: SLSHMDCopyDisplayInfoDictionary called");
    return NULL;
}

void* SLSHMDCopyDisplayQueueConfiguration(void)
{
    if (verbose) puts("STUB: SLSHMDCopyDisplayQueueConfiguration called");
    return NULL;
}

void* SLSHMDCopyMetalDevice(void)
{
    if (verbose) puts("STUB: SLSHMDCopyMetalDevice called");
    return NULL;
}

void* SLSHMDDrawableGetIOSurface(void)
{
    if (verbose) puts("STUB: SLSHMDDrawableGetIOSurface called");
    return NULL;
}

void* SLSHMDDrawablePresentWithTime(void)
{
    if (verbose) puts("STUB: SLSHMDDrawablePresentWithTime called");
    return NULL;
}

void* SLSHMDGetAcceleratorPort(void)
{
    if (verbose) puts("STUB: SLSHMDGetAcceleratorPort called");
    return NULL;
}

void* SLSHMDGetDrawable(void)
{
    if (verbose) puts("STUB: SLSHMDGetDrawable called");
    return NULL;
}

void* SLSHMDGetRefreshTiming(void)
{
    if (verbose) puts("STUB: SLSHMDGetRefreshTiming called");
    return NULL;
}

void* SLSHMDGetTotalUpdateCount(void)
{
    if (verbose) puts("STUB: SLSHMDGetTotalUpdateCount called");
    return NULL;
}

void* SLSHMDIsAvailable(void)
{
    if (verbose) puts("STUB: SLSHMDIsAvailable called");
    return NULL;
}

void* SLSHMDIsAwake(void)
{
    if (verbose) puts("STUB: SLSHMDIsAwake called");
    return NULL;
}

void* SLSHMDRelinquish(void)
{
    if (verbose) puts("STUB: SLSHMDRelinquish called");
    return NULL;
}

void* SLSHMDRemoveNotificationHandler(void)
{
    if (verbose) puts("STUB: SLSHMDRemoveNotificationHandler called");
    return NULL;
}

void* SLSHWCaptureDesktop(void)
{
    if (verbose) puts("STUB: SLSHWCaptureDesktop called");
    return NULL;
}

void* SLSHWCaptureDisplayIntoIOSurface(void)
{
    if (verbose) puts("STUB: SLSHWCaptureDisplayIntoIOSurface called");
    return NULL;
}

void* SLSHWCaptureDisplayIntoIOSurfaceWithOptions(void)
{
    if (verbose) puts("STUB: SLSHWCaptureDisplayIntoIOSurfaceWithOptions called");
    return NULL;
}

void* SLSHWCaptureProcessWindowsInSpaceIncludeDesktop(void)
{
    if (verbose) puts("STUB: SLSHWCaptureProcessWindowsInSpaceIncludeDesktop called");
    return NULL;
}

void* SLSHWCaptureSpace(void)
{
    if (verbose) puts("STUB: SLSHWCaptureSpace called");
    return NULL;
}

void* SLSHWCaptureStreamCreateWithWindow(void)
{
    if (verbose) puts("STUB: SLSHWCaptureStreamCreateWithWindow called");
    return NULL;
}

void* SLSHWCaptureWindowList(void)
{
    if (verbose) puts("STUB: SLSHWCaptureWindowList called");
    return NULL;
}

void* SLSHWCaptureWindowListInRect(void)
{
    if (verbose) puts("STUB: SLSHWCaptureWindowListInRect called");
    return NULL;
}

void* SLSHardwareCursorActive(void)
{
    if (verbose) puts("STUB: SLSHardwareCursorActive called");
    return NULL;
}

void* SLSHideCursor(void)
{
    if (verbose) puts("STUB: SLSHideCursor called");
    return NULL;
}

void* SLSHideSpaces(void)
{
    if (verbose) puts("STUB: SLSHideSpaces called");
    return NULL;
}

void* SLSInhibitLocalEvents(void)
{
    if (verbose) puts("STUB: SLSInhibitLocalEvents called");
    return NULL;
}

void* SLSInitCursor(void)
{
    if (verbose) puts("STUB: SLSInitCursor called");
    return NULL;
}

void* SLSInitialEnableOfDisplayUpdates(void)
{
    if (verbose) puts("STUB: SLSInitialEnableOfDisplayUpdates called");
    return NULL;
}

void* SLSInitialize(void)
{
    if (verbose) puts("STUB: SLSInitialize called");
    return NULL;
}

void* SLSInitializeGamma(void)
{
    if (verbose) puts("STUB: SLSInitializeGamma called");
    return NULL;
}

void* SLSInputButtonState(void)
{
    if (verbose) puts("STUB: SLSInputButtonState called");
    return NULL;
}

void* SLSInputKeyState(void)
{
    if (verbose) puts("STUB: SLSInputKeyState called");
    return NULL;
}

void* SLSInputModifierKeyState(void)
{
    if (verbose) puts("STUB: SLSInputModifierKeyState called");
    return NULL;
}

void* SLSInstallRemoteContextNotificationHandler(void)
{
    if (verbose) puts("STUB: SLSInstallRemoteContextNotificationHandler called");
    return NULL;
}

void* SLSInterruptMenuBarReveal(void)
{
    if (verbose) puts("STUB: SLSInterruptMenuBarReveal called");
    return NULL;
}

void* SLSInvalidateGroupedWindowShadow(void)
{
    if (verbose) puts("STUB: SLSInvalidateGroupedWindowShadow called");
    return NULL;
}

void* SLSInvalidateWindowShadow(void)
{
    if (verbose) puts("STUB: SLSInvalidateWindowShadow called");
    return NULL;
}

void* SLSInvokeTransition(void)
{
    if (verbose) puts("STUB: SLSInvokeTransition called");
    return NULL;
}

void* SLSIsAccessibilityDisplay(void)
{
    if (verbose) puts("STUB: SLSIsAccessibilityDisplay called");
    return NULL;
}

void* SLSIsAmbientDisplayEnabled(void)
{
    if (verbose) puts("STUB: SLSIsAmbientDisplayEnabled called");
    return NULL;
}

void* SLSIsDisplayChangedSeedValid(void)
{
    if (verbose) puts("STUB: SLSIsDisplayChangedSeedValid called");
    return NULL;
}

void* SLSIsHDREnabled(void)
{
    if (verbose) puts("STUB: SLSIsHDREnabled called");
    return NULL;
}

void* SLSIsHDRSupported(void)
{
    if (verbose) puts("STUB: SLSIsHDRSupported called");
    return NULL;
}

void* SLSIsHotKeyEnabled(void)
{
    if (verbose) puts("STUB: SLSIsHotKeyEnabled called");
    return NULL;
}

void* SLSIsMenuBarVisibleOnSpace(void)
{
    if (verbose) puts("STUB: SLSIsMenuBarVisibleOnSpace called");
    return NULL;
}

void* SLSIsScreenWatcherPresent(void)
{
    if (verbose) puts("STUB: SLSIsScreenWatcherPresent called");
    return NULL;
}

void* SLSIsSecureEventInputSet(void)
{
    if (verbose) puts("STUB: SLSIsSecureEventInputSet called");
    return NULL;
}

void* SLSIsSymbolicHotKeyEnabled(void)
{
    if (verbose) puts("STUB: SLSIsSymbolicHotKeyEnabled called");
    return NULL;
}

void* SLSIsSymbolicHotKeyEnabledForConnection(void)
{
    if (verbose) puts("STUB: SLSIsSymbolicHotKeyEnabledForConnection called");
    return NULL;
}

void* SLSIsZoomDisplay(void)
{
    if (verbose) puts("STUB: SLSIsZoomDisplay called");
    return NULL;
}

void* SLSIsZoomed(void)
{
    if (verbose) puts("STUB: SLSIsZoomed called");
    return NULL;
}

void* SLSKeyEventUpdateCharacterTranslation(void)
{
    if (verbose) puts("STUB: SLSKeyEventUpdateCharacterTranslation called");
    return NULL;
}

void* SLSKitVersion(void)
{
    if (verbose) puts("STUB: SLSKitVersion called");
    return NULL;
}

void* SLSLockWindowBackingStore(void)
{
    if (verbose) puts("STUB: SLSLockWindowBackingStore called");
    return NULL;
}

void* SLSLockWindowBits(void)
{
    if (verbose) puts("STUB: SLSLockWindowBits called");
    return NULL;
}

void* SLSLockWindowRectBits(void)
{
    if (verbose) puts("STUB: SLSLockWindowRectBits called");
    return NULL;
}

void* SLSLockWindowVisibleRegion(void)
{
    if (verbose) puts("STUB: SLSLockWindowVisibleRegion called");
    return NULL;
}

void* SLSMachineSupportsLowPowerVideo(void)
{
    if (verbose) puts("STUB: SLSMachineSupportsLowPowerVideo called");
    return NULL;
}

void* SLSMainConnectionID(void)
{
    if (verbose) puts("STUB: SLSMainConnectionID called");
    return NULL;
}

void* SLSMainDisplayBoundsIgnoringCapture(void)
{
    if (verbose) puts("STUB: SLSMainDisplayBoundsIgnoringCapture called");
    return NULL;
}

void* SLSMainDisplayID(void)
{
    if (verbose) puts("STUB: SLSMainDisplayID called");
    return NULL;
}

void* SLSManagedDisplayCurrentSpaceAllowsWindow(void)
{
    if (verbose) puts("STUB: SLSManagedDisplayCurrentSpaceAllowsWindow called");
    return NULL;
}

void* SLSManagedDisplayGetCurrentSpace(void)
{
    if (verbose) puts("STUB: SLSManagedDisplayGetCurrentSpace called");
    return NULL;
}

void* SLSManagedDisplayIsAnimating(void)
{
    if (verbose) puts("STUB: SLSManagedDisplayIsAnimating called");
    return NULL;
}

void* SLSManagedDisplayIteratorAdvance(void)
{
    if (verbose) puts("STUB: SLSManagedDisplayIteratorAdvance called");
    return NULL;
}

void* SLSManagedDisplayIteratorCopyManagedSpaces(void)
{
    if (verbose) puts("STUB: SLSManagedDisplayIteratorCopyManagedSpaces called");
    return NULL;
}

void* SLSManagedDisplayIteratorCopyRoleWindows(void)
{
    if (verbose) puts("STUB: SLSManagedDisplayIteratorCopyRoleWindows called");
    return NULL;
}

void* SLSManagedDisplayIteratorGetAttributes(void)
{
    if (verbose) puts("STUB: SLSManagedDisplayIteratorGetAttributes called");
    return NULL;
}

void* SLSManagedDisplayIteratorGetBounds(void)
{
    if (verbose) puts("STUB: SLSManagedDisplayIteratorGetBounds called");
    return NULL;
}

void* SLSManagedDisplayIteratorGetCount(void)
{
    if (verbose) puts("STUB: SLSManagedDisplayIteratorGetCount called");
    return NULL;
}

void* SLSManagedDisplayIteratorGetTypeID(void)
{
    if (verbose) puts("STUB: SLSManagedDisplayIteratorGetTypeID called");
    return NULL;
}

void* SLSManagedDisplayIteratorGetUUIDBytes(void)
{
    if (verbose) puts("STUB: SLSManagedDisplayIteratorGetUUIDBytes called");
    return NULL;
}

void* SLSManagedDisplaySetCurrentSpace(void)
{
    if (verbose) puts("STUB: SLSManagedDisplaySetCurrentSpace called");
    return NULL;
}

void* SLSManagedDisplaySetIsAnimating(void)
{
    if (verbose) puts("STUB: SLSManagedDisplaySetIsAnimating called");
    return NULL;
}

void* SLSManagedDisplaySetRoleWindow(void)
{
    if (verbose) puts("STUB: SLSManagedDisplaySetRoleWindow called");
    return NULL;
}

void* SLSManagedDisplaysCopyRoleWindows(void)
{
    if (verbose) puts("STUB: SLSManagedDisplaysCopyRoleWindows called");
    return NULL;
}

void* SLSManagedDisplaysGetRoleWindows(void)
{
    if (verbose) puts("STUB: SLSManagedDisplaysGetRoleWindows called");
    return NULL;
}

void* SLSMenuBarExists(void)
{
    if (verbose) puts("STUB: SLSMenuBarExists called");
    return NULL;
}

void* SLSMenuBarHideShowRecordGetAnimationDuration(void)
{
    if (verbose) puts("STUB: SLSMenuBarHideShowRecordGetAnimationDuration called");
    return NULL;
}

void* SLSMenuBarHideShowRecordGetAnimationStartTime(void)
{
    if (verbose) puts("STUB: SLSMenuBarHideShowRecordGetAnimationStartTime called");
    return NULL;
}

void* SLSMenuBarHideShowRecordGetSpaceID(void)
{
    if (verbose) puts("STUB: SLSMenuBarHideShowRecordGetSpaceID called");
    return NULL;
}

void* SLSMoveManagedSpaceToDisplayIndex(void)
{
    if (verbose) puts("STUB: SLSMoveManagedSpaceToDisplayIndex called");
    return NULL;
}

void* SLSMoveSessionToConsoleTemporaryBridge(void)
{
    if (verbose) puts("STUB: SLSMoveSessionToConsoleTemporaryBridge called");
    return NULL;
}

void* SLSMoveSurface(void)
{
    if (verbose) puts("STUB: SLSMoveSurface called");
    return NULL;
}

void* SLSMoveTrackingArea(void)
{
    if (verbose) puts("STUB: SLSMoveTrackingArea called");
    return NULL;
}

void* SLSMoveWindow(void)
{
    if (verbose) puts("STUB: SLSMoveWindow called");
    return NULL;
}

void* SLSMoveWindowList(void)
{
    if (verbose) puts("STUB: SLSMoveWindowList called");
    return NULL;
}

void* SLSMoveWindowListOnMatchingDisplayChangedSeed(void)
{
    if (verbose) puts("STUB: SLSMoveWindowListOnMatchingDisplayChangedSeed called");
    return NULL;
}

void* SLSMoveWindowOnMatchingDisplayChangedSeed(void)
{
    if (verbose) puts("STUB: SLSMoveWindowOnMatchingDisplayChangedSeed called");
    return NULL;
}

void* SLSMoveWindowWithGroup(void)
{
    if (verbose) puts("STUB: SLSMoveWindowWithGroup called");
    return NULL;
}

void* SLSMoveWindowsToManagedSpace(void)
{
    if (verbose) puts("STUB: SLSMoveWindowsToManagedSpace called");
    return NULL;
}

void* SLSMoveWorkspaceWindowList(void)
{
    if (verbose) puts("STUB: SLSMoveWorkspaceWindowList called");
    return NULL;
}

void* SLSMoveWorkspaceWindowListWithOptions(void)
{
    if (verbose) puts("STUB: SLSMoveWorkspaceWindowListWithOptions called");
    return NULL;
}

void* SLSNewCIFilter(void)
{
    if (verbose) puts("STUB: SLSNewCIFilter called");
    return NULL;
}

void* SLSNewCIFilterByName(void)
{
    if (verbose) puts("STUB: SLSNewCIFilterByName called");
    return NULL;
}

void* SLSNewConnection(void)
{
    if (verbose) puts("STUB: SLSNewConnection called");
    return NULL;
}

void* SLSNewTransition(void)
{
    if (verbose) puts("STUB: SLSNewTransition called");
    return NULL;
}

void* SLSNewWindow(void)
{
    if (verbose) puts("STUB: SLSNewWindow called");
    return NULL;
}

void* SLSNewWindowWithOpaqueShape(void)
{
    if (verbose) puts("STUB: SLSNewWindowWithOpaqueShape called");
    return NULL;
}

void* SLSObscureCursor(void)
{
    if (verbose) puts("STUB: SLSObscureCursor called");
    return NULL;
}

void* SLSOrderFrontConditionally(void)
{
    if (verbose) puts("STUB: SLSOrderFrontConditionally called");
    return NULL;
}

void* SLSOrderPanelIfActive(void)
{
    if (verbose) puts("STUB: SLSOrderPanelIfActive called");
    return NULL;
}

void* SLSOrderSurface(void)
{
    if (verbose) puts("STUB: SLSOrderSurface called");
    return NULL;
}

void* SLSOrderWindow(void)
{
    if (verbose) puts("STUB: SLSOrderWindow called");
    return NULL;
}

void* SLSOrderWindowList(void)
{
    if (verbose) puts("STUB: SLSOrderWindowList called");
    return NULL;
}

void* SLSOrderWindowListWithGroups(void)
{
    if (verbose) puts("STUB: SLSOrderWindowListWithGroups called");
    return NULL;
}

void* SLSOrderWindowListWithOperation(void)
{
    if (verbose) puts("STUB: SLSOrderWindowListWithOperation called");
    return NULL;
}

void* SLSOrderWindowWithGroup(void)
{
    if (verbose) puts("STUB: SLSOrderWindowWithGroup called");
    return NULL;
}

void* SLSOverrideAppSleepNotifications(void)
{
    if (verbose) puts("STUB: SLSOverrideAppSleepNotifications called");
    return NULL;
}

void* SLSPackagesAddWindowToDraggingSpace(void)
{
    if (verbose) puts("STUB: SLSPackagesAddWindowToDraggingSpace called");
    return NULL;
}

void* SLSPackagesAssignDraggedWindowToDestinationSpace(void)
{
    if (verbose) puts("STUB: SLSPackagesAssignDraggedWindowToDestinationSpace called");
    return NULL;
}

void* SLSPackagesCopyWorkspaceIdentifierForWorkspace(void)
{
    if (verbose) puts("STUB: SLSPackagesCopyWorkspaceIdentifierForWorkspace called");
    return NULL;
}

void* SLSPackagesDisableFindCursor(void)
{
    if (verbose) puts("STUB: SLSPackagesDisableFindCursor called");
    return NULL;
}

void* SLSPackagesDragWindowRelativeToMouse(void)
{
    if (verbose) puts("STUB: SLSPackagesDragWindowRelativeToMouse called");
    return NULL;
}

void* SLSPackagesEnableConnectionOcclusionNotifications(void)
{
    if (verbose) puts("STUB: SLSPackagesEnableConnectionOcclusionNotifications called");
    return NULL;
}

void* SLSPackagesEnableConnectionWindowModificationNotifications(void)
{
    if (verbose) puts("STUB: SLSPackagesEnableConnectionWindowModificationNotifications called");
    return NULL;
}

void* SLSPackagesEnableWindowOcclusionNotifications(void)
{
    if (verbose) puts("STUB: SLSPackagesEnableWindowOcclusionNotifications called");
    return NULL;
}

void* SLSPackagesFindCursorIsDisabled(void)
{
    if (verbose) puts("STUB: SLSPackagesFindCursorIsDisabled called");
    return NULL;
}

void* SLSPackagesGetWindowConstraints(void)
{
    if (verbose) puts("STUB: SLSPackagesGetWindowConstraints called");
    return NULL;
}

void* SLSPackagesGetWorkspaceForWorkspaceIdentifier(void)
{
    if (verbose) puts("STUB: SLSPackagesGetWorkspaceForWorkspaceIdentifier called");
    return NULL;
}

void* SLSPackagesInitializeSessionPreferences(void)
{
    if (verbose) puts("STUB: SLSPackagesInitializeSessionPreferences called");
    return NULL;
}

void* SLSPackagesRemoveWindowDragTransform(void)
{
    if (verbose) puts("STUB: SLSPackagesRemoveWindowDragTransform called");
    return NULL;
}

void* SLSPackagesRemoveWindowFromDraggingSpace(void)
{
    if (verbose) puts("STUB: SLSPackagesRemoveWindowFromDraggingSpace called");
    return NULL;
}

void* SLSPackagesSetFindCursorPrefs(void)
{
    if (verbose) puts("STUB: SLSPackagesSetFindCursorPrefs called");
    return NULL;
}

void* SLSPackagesSetWindowConstraints(void)
{
    if (verbose) puts("STUB: SLSPackagesSetWindowConstraints called");
    return NULL;
}

void* SLSPackagesSetWindowDragTransform(void)
{
    if (verbose) puts("STUB: SLSPackagesSetWindowDragTransform called");
    return NULL;
}

void* SLSPersistenceAnnounceAllAppsReady(void)
{
    if (verbose) puts("STUB: SLSPersistenceAnnounceAllAppsReady called");
    return NULL;
}

void* SLSPersistenceRegisterAsReady(void)
{
    if (verbose) puts("STUB: SLSPersistenceRegisterAsReady called");
    return NULL;
}

void* SLSPersistenceSaveSpaceConfiguration(void)
{
    if (verbose) puts("STUB: SLSPersistenceSaveSpaceConfiguration called");
    return NULL;
}

void* SLSPersistenceWillLaunchASNsWithCompletionHandler(void)
{
    if (verbose) puts("STUB: SLSPersistenceWillLaunchASNsWithCompletionHandler called");
    return NULL;
}

void* SLSPersistenceWillTerminateSession(void)
{
    if (verbose) puts("STUB: SLSPersistenceWillTerminateSession called");
    return NULL;
}

void* SLSPostBroadcastNotification(void)
{
    if (verbose) puts("STUB: SLSPostBroadcastNotification called");
    return NULL;
}

void* SLSPostCoordinatedDistributedNotification(void)
{
    if (verbose) puts("STUB: SLSPostCoordinatedDistributedNotification called");
    return NULL;
}

void* SLSPostEvent(void)
{
    if (verbose) puts("STUB: SLSPostEvent called");
    return NULL;
}

void* SLSPostEventRecord(void)
{
    if (verbose) puts("STUB: SLSPostEventRecord called");
    return NULL;
}

void* SLSPostKeyboardEvent(void)
{
    if (verbose) puts("STUB: SLSPostKeyboardEvent called");
    return NULL;
}

void* SLSPostMouseEvent(void)
{
    if (verbose) puts("STUB: SLSPostMouseEvent called");
    return NULL;
}

void* SLSPostMouseEvents(void)
{
    if (verbose) puts("STUB: SLSPostMouseEvents called");
    return NULL;
}

void* SLSPostNotificationToConnection(void)
{
    if (verbose) puts("STUB: SLSPostNotificationToConnection called");
    return NULL;
}

void* SLSPreflightListenEventAccess(void)
{
    if (verbose) puts("STUB: SLSPreflightListenEventAccess called");
    return NULL;
}

void* SLSPreflightPostEventAccess(void)
{
    if (verbose) puts("STUB: SLSPreflightPostEventAccess called");
    return NULL;
}

void* SLSPreflightScreenCaptureAccess(void)
{
    if (verbose) puts("STUB: SLSPreflightScreenCaptureAccess called");
    return NULL;
}

void* SLSProcessAssignToAllSpaces(void)
{
    if (verbose) puts("STUB: SLSProcessAssignToAllSpaces called");
    return NULL;
}

void* SLSProcessAssignToSpace(void)
{
    if (verbose) puts("STUB: SLSProcessAssignToSpace called");
    return NULL;
}

void* SLSRealTimeStampFromEventRecordTime(void)
{
    if (verbose) puts("STUB: SLSRealTimeStampFromEventRecordTime called");
    return NULL;
}

void* SLSReassociateWindowsSpacesByGeometry(void)
{
    if (verbose) puts("STUB: SLSReassociateWindowsSpacesByGeometry called");
    return NULL;
}

void* SLSReenableCursorObscure(void)
{
    if (verbose) puts("STUB: SLSReenableCursorObscure called");
    return NULL;
}

void* SLSReenableDisableUpdate(void)
{
    if (verbose) puts("STUB: SLSReenableDisableUpdate called");
    return NULL;
}

void* SLSReenableUpdate(void)
{
    if (verbose) puts("STUB: SLSReenableUpdate called");
    return NULL;
}

void* SLSReenableUpdateToken(void)
{
    if (verbose) puts("STUB: SLSReenableUpdateToken called");
    return NULL;
}

void* SLSReenableUpdateTokenWithSeed(void)
{
    if (verbose) puts("STUB: SLSReenableUpdateTokenWithSeed called");
    return NULL;
}

void* SLSRegisterColorSpaceFromICCData(void)
{
    if (verbose) puts("STUB: SLSRegisterColorSpaceFromICCData called");
    return NULL;
}

void* SLSRegisterConnectionNotifyProc(void)
{
    if (verbose) puts("STUB: SLSRegisterConnectionNotifyProc called");
    return NULL;
}

void* SLSRegisterCursorWithData(void)
{
    if (verbose) puts("STUB: SLSRegisterCursorWithData called");
    return NULL;
}

void* SLSRegisterCursorWithImage(void)
{
    if (verbose) puts("STUB: SLSRegisterCursorWithImage called");
    return NULL;
}

void* SLSRegisterCursorWithImages(void)
{
    if (verbose) puts("STUB: SLSRegisterCursorWithImages called");
    return NULL;
}

void* SLSRegisterNotifyProc(void)
{
    if (verbose) puts("STUB: SLSRegisterNotifyProc called");
    return NULL;
}

void* SLSRegisterUpdateInformationHandler(void)
{
    if (verbose) puts("STUB: SLSRegisterUpdateInformationHandler called");
    return NULL;
}

void* SLSReleaseCIFilter(void)
{
    if (verbose) puts("STUB: SLSReleaseCIFilter called");
    return NULL;
}

void* SLSReleaseConnection(void)
{
    if (verbose) puts("STUB: SLSReleaseConnection called");
    return NULL;
}

void* SLSReleaseSession(void)
{
    if (verbose) puts("STUB: SLSReleaseSession called");
    return NULL;
}

void* SLSReleaseSessionTemporaryBridge(void)
{
    if (verbose) puts("STUB: SLSReleaseSessionTemporaryBridge called");
    return NULL;
}

void* SLSReleaseTransition(void)
{
    if (verbose) puts("STUB: SLSReleaseTransition called");
    return NULL;
}

void* SLSReleaseWindow(void)
{
    if (verbose) puts("STUB: SLSReleaseWindow called");
    return NULL;
}

void* SLSReleaseWindowAnimation(void)
{
    if (verbose) puts("STUB: SLSReleaseWindowAnimation called");
    return NULL;
}

void* SLSReleaseWindowList(void)
{
    if (verbose) puts("STUB: SLSReleaseWindowList called");
    return NULL;
}

void* SLSRemoteContextAddNativeSpaceTrackingRegion(void)
{
    if (verbose) puts("STUB: SLSRemoteContextAddNativeSpaceTrackingRegion called");
    return NULL;
}

void* SLSRemoteContextAddStructuralRegion(void)
{
    if (verbose) puts("STUB: SLSRemoteContextAddStructuralRegion called");
    return NULL;
}

void* SLSRemoteContextAddTrackingRegion(void)
{
    if (verbose) puts("STUB: SLSRemoteContextAddTrackingRegion called");
    return NULL;
}

void* SLSRemoveAllSurfaces(void)
{
    if (verbose) puts("STUB: SLSRemoveAllSurfaces called");
    return NULL;
}

void* SLSRemoveAllTrackingAreas(void)
{
    if (verbose) puts("STUB: SLSRemoveAllTrackingAreas called");
    return NULL;
}

void* SLSRemoveConnectionNotifyProc(void)
{
    if (verbose) puts("STUB: SLSRemoveConnectionNotifyProc called");
    return NULL;
}

void* SLSRemoveDragSubregionForCommandModifier(void)
{
    if (verbose) puts("STUB: SLSRemoveDragSubregionForCommandModifier called");
    return NULL;
}

void* SLSRemoveDragSubregionInWindow(void)
{
    if (verbose) puts("STUB: SLSRemoveDragSubregionInWindow called");
    return NULL;
}

void* SLSRemoveFromOrderingGroup(void)
{
    if (verbose) puts("STUB: SLSRemoveFromOrderingGroup called");
    return NULL;
}

void* SLSRemoveHotKey(void)
{
    if (verbose) puts("STUB: SLSRemoveHotKey called");
    return NULL;
}

void* SLSRemoveNotifyProc(void)
{
    if (verbose) puts("STUB: SLSRemoveNotifyProc called");
    return NULL;
}

void* SLSRemoveRegisteredCursor(void)
{
    if (verbose) puts("STUB: SLSRemoveRegisteredCursor called");
    return NULL;
}

void* SLSRemoveRemoteContextNotificationHandler(void)
{
    if (verbose) puts("STUB: SLSRemoveRemoteContextNotificationHandler called");
    return NULL;
}

void* SLSRemoveStructuralRegion(void)
{
    if (verbose) puts("STUB: SLSRemoveStructuralRegion called");
    return NULL;
}

void* SLSRemoveSurface(void)
{
    if (verbose) puts("STUB: SLSRemoveSurface called");
    return NULL;
}

void* SLSRemoveTrackingArea(void)
{
    if (verbose) puts("STUB: SLSRemoveTrackingArea called");
    return NULL;
}

void* SLSRemoveTrackingAreas(void)
{
    if (verbose) puts("STUB: SLSRemoveTrackingAreas called");
    return NULL;
}

void* SLSRemoveWindowFilter(void)
{
    if (verbose) puts("STUB: SLSRemoveWindowFilter called");
    return NULL;
}

void* SLSRemoveWindowFromWindowMovementGroup(void)
{
    if (verbose) puts("STUB: SLSRemoveWindowFromWindowMovementGroup called");
    return NULL;
}

void* SLSRemoveWindowsFromSpaces(void)
{
    if (verbose) puts("STUB: SLSRemoveWindowsFromSpaces called");
    return NULL;
}

void* SLSReorderWindows(void)
{
    if (verbose) puts("STUB: SLSReorderWindows called");
    return NULL;
}

void* SLSRequestListenEventAccess(void)
{
    if (verbose) puts("STUB: SLSRequestListenEventAccess called");
    return NULL;
}

void* SLSRequestNotificationsForWindows(void)
{
    if (verbose) puts("STUB: SLSRequestNotificationsForWindows called");
    return NULL;
}

void* SLSRequestPostEventAccess(void)
{
    if (verbose) puts("STUB: SLSRequestPostEventAccess called");
    return NULL;
}

void* SLSRequestScreenCaptureAccess(void)
{
    if (verbose) puts("STUB: SLSRequestScreenCaptureAccess called");
    return NULL;
}

void* SLSResetMenuBarSystemOverrideAlphas(void)
{
    if (verbose) puts("STUB: SLSResetMenuBarSystemOverrideAlphas called");
    return NULL;
}

void* SLSResetWindows(void)
{
    if (verbose) puts("STUB: SLSResetWindows called");
    return NULL;
}

void* SLSRestorePackagesManagementPersistenceData(void)
{
    if (verbose) puts("STUB: SLSRestorePackagesManagementPersistenceData called");
    return NULL;
}

void* SLSRevealCursor(void)
{
    if (verbose) puts("STUB: SLSRevealCursor called");
    return NULL;
}

void* SLSRunningOnInstallerDisk(void)
{
    if (verbose) puts("STUB: SLSRunningOnInstallerDisk called");
    return NULL;
}

void* SLSSaveConfiguration(void)
{
    if (verbose) puts("STUB: SLSSaveConfiguration called");
    return NULL;
}

void* SLSSavePerUserConfigurationData(void)
{
    if (verbose) puts("STUB: SLSSavePerUserConfigurationData called");
    return NULL;
}

void* SLSScreenTimeSuppressWindows(void)
{
    if (verbose) puts("STUB: SLSScreenTimeSuppressWindows called");
    return NULL;
}

void* SLSSecondsSinceLastInputEvent(void)
{
    if (verbose) puts("STUB: SLSSecondsSinceLastInputEvent called");
    return NULL;
}

void* SLSSecondsSinceMostRecentEventInMask(void)
{
    if (verbose) puts("STUB: SLSSecondsSinceMostRecentEventInMask called");
    return NULL;
}

void* SLSServerHasValidConnections(void)
{
    if (verbose) puts("STUB: SLSServerHasValidConnections called");
    return NULL;
}

void* SLSServerIsConnected(void)
{
    if (verbose) puts("STUB: SLSServerIsConnected called");
    return NULL;
}

void* SLSServerMightHaveAcceleration(void)
{
    if (verbose) puts("STUB: SLSServerMightHaveAcceleration called");
    return NULL;
}

void* SLSServerOperationState(void)
{
    if (verbose) puts("STUB: SLSServerOperationState called");
    return NULL;
}

void* SLSServerPort(void)
{
    if (verbose) puts("STUB: SLSServerPort called");
    return NULL;
}

void* SLSServiceForDisplayAlias(void)
{
    if (verbose) puts("STUB: SLSServiceForDisplayAlias called");
    return NULL;
}

void* SLSServiceForDisplayNumber(void)
{
    if (verbose) puts("STUB: SLSServiceForDisplayNumber called");
    return NULL;
}

void* SLSSessionCopyAllSessionProperties(void)
{
    if (verbose) puts("STUB: SLSSessionCopyAllSessionProperties called");
    return NULL;
}

void* SLSSessionCopyCurrentSessionProperties(void)
{
    if (verbose) puts("STUB: SLSSessionCopyCurrentSessionProperties called");
    return NULL;
}

void* SLSSessionCopyCurrentSessionReadOnceData(void)
{
    if (verbose) puts("STUB: SLSSessionCopyCurrentSessionReadOnceData called");
    return NULL;
}

void* SLSSessionCreateLoginSessionID(void)
{
    if (verbose) puts("STUB: SLSSessionCreateLoginSessionID called");
    return NULL;
}

void* SLSSessionCreateSessionIDWithOptions(void)
{
    if (verbose) puts("STUB: SLSSessionCreateSessionIDWithOptions called");
    return NULL;
}

void* SLSSessionFullScreenAppPid(void)
{
    if (verbose) puts("STUB: SLSSessionFullScreenAppPid called");
    return NULL;
}

void* SLSSessionHasExcludeShapes(void)
{
    if (verbose) puts("STUB: SLSSessionHasExcludeShapes called");
    return NULL;
}

void* SLSSessionIDGetFullScreenAppPid(void)
{
    if (verbose) puts("STUB: SLSSessionIDGetFullScreenAppPid called");
    return NULL;
}

void* SLSSessionIDSetScreenLockWindowID(void)
{
    if (verbose) puts("STUB: SLSSessionIDSetScreenLockWindowID called");
    return NULL;
}

void* SLSSessionIDSetSleepWindowID(void)
{
    if (verbose) puts("STUB: SLSSessionIDSetSleepWindowID called");
    return NULL;
}

void* SLSSessionReleaseSessionID(void)
{
    if (verbose) puts("STUB: SLSSessionReleaseSessionID called");
    return NULL;
}

void* SLSSessionSetApplicationBindingsForWorkspaces(void)
{
    if (verbose) puts("STUB: SLSSessionSetApplicationBindingsForWorkspaces called");
    return NULL;
}

void* SLSSessionSetCurrentSessionProperties(void)
{
    if (verbose) puts("STUB: SLSSessionSetCurrentSessionProperties called");
    return NULL;
}

void* SLSSessionSetCurrentSessionWorkspaceApplicationBindings(void)
{
    if (verbose) puts("STUB: SLSSessionSetCurrentSessionWorkspaceApplicationBindings called");
    return NULL;
}

void* SLSSessionSetCurrentSessionWorkspaceLaunchPath(void)
{
    if (verbose) puts("STUB: SLSSessionSetCurrentSessionWorkspaceLaunchPath called");
    return NULL;
}

void* SLSSessionSetCurrentSessionWorkspaceNotificationConnectionID(void)
{
    if (verbose) puts("STUB: SLSSessionSetCurrentSessionWorkspaceNotificationConnectionID called");
    return NULL;
}

void* SLSSessionSetCurrentSessionWorkspacePreferences(void)
{
    if (verbose) puts("STUB: SLSSessionSetCurrentSessionWorkspacePreferences called");
    return NULL;
}

void* SLSSessionSetNotificationConnectionForWorkspaces(void)
{
    if (verbose) puts("STUB: SLSSessionSetNotificationConnectionForWorkspaces called");
    return NULL;
}

void* SLSSessionSetPreferencesForWorkspaces(void)
{
    if (verbose) puts("STUB: SLSSessionSetPreferencesForWorkspaces called");
    return NULL;
}

void* SLSSessionSwitchToAuditSessionID(void)
{
    if (verbose) puts("STUB: SLSSessionSwitchToAuditSessionID called");
    return NULL;
}

void* SLSSessionSwitchToAuditSessionIDWithOptions(void)
{
    if (verbose) puts("STUB: SLSSessionSwitchToAuditSessionIDWithOptions called");
    return NULL;
}

void* SLSSessionSwitchToSessionID(void)
{
    if (verbose) puts("STUB: SLSSessionSwitchToSessionID called");
    return NULL;
}

void* SLSSetAccessibilityAdjustments(void)
{
    if (verbose) puts("STUB: SLSSetAccessibilityAdjustments called");
    return NULL;
}

void* SLSSetActiveMenuBarDisplayIdentifier(void)
{
    if (verbose) puts("STUB: SLSSetActiveMenuBarDisplayIdentifier called");
    return NULL;
}

void* SLSSetAppearanceThemeLegacy(void)
{
    if (verbose) puts("STUB: SLSSetAppearanceThemeLegacy called");
    return NULL;
}

void* SLSSetAppearanceThemeNotifying(void)
{
    if (verbose) puts("STUB: SLSSetAppearanceThemeNotifying called");
    return NULL;
}

void* SLSSetAppearanceThemeSwitchesAutomatically(void)
{
    if (verbose) puts("STUB: SLSSetAppearanceThemeSwitchesAutomatically called");
    return NULL;
}

void* SLSSetAvoidsActivation(void)
{
    if (verbose) puts("STUB: SLSSetAvoidsActivation called");
    return NULL;
}

void* SLSSetBackgroundEventMask(void)
{
    if (verbose) puts("STUB: SLSSetBackgroundEventMask called");
    return NULL;
}

void* SLSSetBackgroundEventMaskAndShape(void)
{
    if (verbose) puts("STUB: SLSSetBackgroundEventMaskAndShape called");
    return NULL;
}

void* SLSSetCIFilterValues(void)
{
    if (verbose) puts("STUB: SLSSetCIFilterValues called");
    return NULL;
}

void* SLSSetCIFilterValuesFromDictionary(void)
{
    if (verbose) puts("STUB: SLSSetCIFilterValuesFromDictionary called");
    return NULL;
}

void* SLSSetChameleonOverrideColor(void)
{
    if (verbose) puts("STUB: SLSSetChameleonOverrideColor called");
    return NULL;
}

void* SLSSetConnectionProperty(void)
{
    if (verbose) puts("STUB: SLSSetConnectionProperty called");
    return NULL;
}

void* SLSSetCursorDragLock(void)
{
    if (verbose) puts("STUB: SLSSetCursorDragLock called");
    return NULL;
}

void* SLSSetCursorFromData(void)
{
    if (verbose) puts("STUB: SLSSetCursorFromData called");
    return NULL;
}

void* SLSSetCursorFromPremultipliedDataWithSeed(void)
{
    if (verbose) puts("STUB: SLSSetCursorFromPremultipliedDataWithSeed called");
    return NULL;
}

void* SLSSetCursorRegionLock(void)
{
    if (verbose) puts("STUB: SLSSetCursorRegionLock called");
    return NULL;
}

void* SLSSetCursorScale(void)
{
    if (verbose) puts("STUB: SLSSetCursorScale called");
    return NULL;
}

void* SLSSetDebugOptions(void)
{
    if (verbose) puts("STUB: SLSSetDebugOptions called");
    return NULL;
}

void* SLSSetDebugOptionsData(void)
{
    if (verbose) puts("STUB: SLSSetDebugOptionsData called");
    return NULL;
}

void* SLSSetDebugOptionsPSN(void)
{
    if (verbose) puts("STUB: SLSSetDebugOptionsPSN called");
    return NULL;
}

void* SLSSetDebugOptionsWithDataObtainingPort(void)
{
    if (verbose) puts("STUB: SLSSetDebugOptionsWithDataObtainingPort called");
    return NULL;
}

void* SLSSetDebugOptionsWithDataObtainingResult(void)
{
    if (verbose) puts("STUB: SLSSetDebugOptionsWithDataObtainingResult called");
    return NULL;
}

void* SLSSetDefaultSpace(void)
{
    if (verbose) puts("STUB: SLSSetDefaultSpace called");
    return NULL;
}

void* SLSSetDeferActivation(void)
{
    if (verbose) puts("STUB: SLSSetDeferActivation called");
    return NULL;
}

void* SLSSetDeferOrdering(void)
{
    if (verbose) puts("STUB: SLSSetDeferOrdering called");
    return NULL;
}

void* SLSSetDeferredUpdateNotificationPort(void)
{
    if (verbose) puts("STUB: SLSSetDeferredUpdateNotificationPort called");
    return NULL;
}

void* SLSSetDenyWindowServerConnections(void)
{
    if (verbose) puts("STUB: SLSSetDenyWindowServerConnections called");
    return NULL;
}

void* SLSSetDictionaryForCurrentSession(void)
{
    if (verbose) puts("STUB: SLSSetDictionaryForCurrentSession called");
    return NULL;
}

void* SLSSetDisplayAcceleratorBoost(void)
{
    if (verbose) puts("STUB: SLSSetDisplayAcceleratorBoost called");
    return NULL;
}

void* SLSSetDisplayBrightness(void)
{
    if (verbose) puts("STUB: SLSSetDisplayBrightness called");
    return NULL;
}

void* SLSSetDisplayColorSpace(void)
{
    if (verbose) puts("STUB: SLSSetDisplayColorSpace called");
    return NULL;
}

void* SLSSetDisplayColorSpaceAndTransfer(void)
{
    if (verbose) puts("STUB: SLSSetDisplayColorSpaceAndTransfer called");
    return NULL;
}

void* SLSSetDisplayContrast(void)
{
    if (verbose) puts("STUB: SLSSetDisplayContrast called");
    return NULL;
}

void* SLSSetDisplayIntention(void)
{
    if (verbose) puts("STUB: SLSSetDisplayIntention called");
    return NULL;
}

void* SLSSetDisplayIntentionWiper(void)
{
    if (verbose) puts("STUB: SLSSetDisplayIntentionWiper called");
    return NULL;
}

void* SLSSetDisplayOption(void)
{
    if (verbose) puts("STUB: SLSSetDisplayOption called");
    return NULL;
}

void* SLSSetDisplayRotation(void)
{
    if (verbose) puts("STUB: SLSSetDisplayRotation called");
    return NULL;
}

void* SLSSetDisplayTransfer(void)
{
    if (verbose) puts("STUB: SLSSetDisplayTransfer called");
    return NULL;
}

void* SLSSetDisplayTransferByFormula(void)
{
    if (verbose) puts("STUB: SLSSetDisplayTransferByFormula called");
    return NULL;
}

void* SLSSetDisplayTransferByFormulaWithOption(void)
{
    if (verbose) puts("STUB: SLSSetDisplayTransferByFormulaWithOption called");
    return NULL;
}

void* SLSSetDisplayTransferByTable(void)
{
    if (verbose) puts("STUB: SLSSetDisplayTransferByTable called");
    return NULL;
}

void* SLSSetDisplayTransferByTableWithOption(void)
{
    if (verbose) puts("STUB: SLSSetDisplayTransferByTableWithOption called");
    return NULL;
}

void* SLSSetDockCursorOverride(void)
{
    if (verbose) puts("STUB: SLSSetDockCursorOverride called");
    return NULL;
}

void* SLSSetDockRectWithOrientation(void)
{
    if (verbose) puts("STUB: SLSSetDockRectWithOrientation called");
    return NULL;
}

void* SLSSetDockRectWithReason(void)
{
    if (verbose) puts("STUB: SLSSetDockRectWithReason called");
    return NULL;
}

void* SLSSetEventCapture(void)
{
    if (verbose) puts("STUB: SLSSetEventCapture called");
    return NULL;
}

void* SLSSetEventCaptureWindowAndMask(void)
{
    if (verbose) puts("STUB: SLSSetEventCaptureWindowAndMask called");
    return NULL;
}

void* SLSSetEventMask(void)
{
    if (verbose) puts("STUB: SLSSetEventMask called");
    return NULL;
}

void* SLSSetEventTapOption(void)
{
    if (verbose) puts("STUB: SLSSetEventTapOption called");
    return NULL;
}

void* SLSSetEventTapTimeout(void)
{
    if (verbose) puts("STUB: SLSSetEventTapTimeout called");
    return NULL;
}

void* SLSSetFrontWindow(void)
{
    if (verbose) puts("STUB: SLSSetFrontWindow called");
    return NULL;
}

void* SLSSetGestureEventSubmask(void)
{
    if (verbose) puts("STUB: SLSSetGestureEventSubmask called");
    return NULL;
}

void* SLSSetGlobalForceConfig(void)
{
    if (verbose) puts("STUB: SLSSetGlobalForceConfig called");
    return NULL;
}

void* SLSSetGlobalHotKeyOperatingMode(void)
{
    if (verbose) puts("STUB: SLSSetGlobalHotKeyOperatingMode called");
    return NULL;
}

void* SLSSetHideOnDeact(void)
{
    if (verbose) puts("STUB: SLSSetHideOnDeact called");
    return NULL;
}

void* SLSSetHotButtonWithExclusion(void)
{
    if (verbose) puts("STUB: SLSSetHotButtonWithExclusion called");
    return NULL;
}

void* SLSSetHotKey(void)
{
    if (verbose) puts("STUB: SLSSetHotKey called");
    return NULL;
}

void* SLSSetHotKeyEnabled(void)
{
    if (verbose) puts("STUB: SLSSetHotKeyEnabled called");
    return NULL;
}

void* SLSSetHotKeyRepresentation(void)
{
    if (verbose) puts("STUB: SLSSetHotKeyRepresentation called");
    return NULL;
}

void* SLSSetHotKeyRepresentationWithOptions(void)
{
    if (verbose) puts("STUB: SLSSetHotKeyRepresentationWithOptions called");
    return NULL;
}

void* SLSSetHotKeyType(void)
{
    if (verbose) puts("STUB: SLSSetHotKeyType called");
    return NULL;
}

void* SLSSetHotKeyWithExclusion(void)
{
    if (verbose) puts("STUB: SLSSetHotKeyWithExclusion called");
    return NULL;
}

void* SLSSetHotKeyWithOptions(void)
{
    if (verbose) puts("STUB: SLSSetHotKeyWithOptions called");
    return NULL;
}

void* SLSSetHotModifierWithExclusion(void)
{
    if (verbose) puts("STUB: SLSSetHotModifierWithExclusion called");
    return NULL;
}

void* SLSSetHotModifierWithOptions(void)
{
    if (verbose) puts("STUB: SLSSetHotModifierWithOptions called");
    return NULL;
}

void* SLSSetIOHIDEventSystemClientProperty(void)
{
    if (verbose) puts("STUB: SLSSetIOHIDEventSystemClientProperty called");
    return NULL;
}

void* SLSSetIgnoreAsFrontWindow(void)
{
    if (verbose) puts("STUB: SLSSetIgnoreAsFrontWindow called");
    return NULL;
}

void* SLSSetIgnoresCycle(void)
{
    if (verbose) puts("STUB: SLSSetIgnoresCycle called");
    return NULL;
}

void* SLSSetIgnoresOption(void)
{
    if (verbose) puts("STUB: SLSSetIgnoresOption called");
    return NULL;
}

void* SLSSetInkingState(void)
{
    if (verbose) puts("STUB: SLSSetInkingState called");
    return NULL;
}

void* SLSSetKeyTranslationTableFromPtr(void)
{
    if (verbose) puts("STUB: SLSSetKeyTranslationTableFromPtr called");
    return NULL;
}

void* SLSSetKitVersion(void)
{
    if (verbose) puts("STUB: SLSSetKitVersion called");
    return NULL;
}

void* SLSSetLocalEventsSuppressionInterval(void)
{
    if (verbose) puts("STUB: SLSSetLocalEventsSuppressionInterval called");
    return NULL;
}

void* SLSSetLoginName(void)
{
    if (verbose) puts("STUB: SLSSetLoginName called");
    return NULL;
}

void* SLSSetLoginwindowConnection(void)
{
    if (verbose) puts("STUB: SLSSetLoginwindowConnection called");
    return NULL;
}

void* SLSSetMagicMirrorProperties(void)
{
    if (verbose) puts("STUB: SLSSetMagicMirrorProperties called");
    return NULL;
}

void* SLSSetMagicZoom(void)
{
    if (verbose) puts("STUB: SLSSetMagicZoom called");
    return NULL;
}

void* SLSSetMenuBar(void)
{
    if (verbose) puts("STUB: SLSSetMenuBar called");
    return NULL;
}

void* SLSSetMenuBarAutohideEnabled(void)
{
    if (verbose) puts("STUB: SLSSetMenuBarAutohideEnabled called");
    return NULL;
}

void* SLSSetMenuBarBounds(void)
{
    if (verbose) puts("STUB: SLSSetMenuBarBounds called");
    return NULL;
}

void* SLSSetMenuBarCompanionWindow(void)
{
    if (verbose) puts("STUB: SLSSetMenuBarCompanionWindow called");
    return NULL;
}

void* SLSSetMenuBarDrawingStyle(void)
{
    if (verbose) puts("STUB: SLSSetMenuBarDrawingStyle called");
    return NULL;
}

void* SLSSetMenuBarInsetAndAlpha(void)
{
    if (verbose) puts("STUB: SLSSetMenuBarInsetAndAlpha called");
    return NULL;
}

void* SLSSetMenuBarSystemOverrideAlpha(void)
{
    if (verbose) puts("STUB: SLSSetMenuBarSystemOverrideAlpha called");
    return NULL;
}

void* SLSSetMenuBarVisibilityOverrideOnDisplay(void)
{
    if (verbose) puts("STUB: SLSSetMenuBarVisibilityOverrideOnDisplay called");
    return NULL;
}

void* SLSSetMenuBars(void)
{
    if (verbose) puts("STUB: SLSSetMenuBars called");
    return NULL;
}

void* SLSSetMouseEventEnableFlags(void)
{
    if (verbose) puts("STUB: SLSSetMouseEventEnableFlags called");
    return NULL;
}

void* SLSSetMouseFocusWindow(void)
{
    if (verbose) puts("STUB: SLSSetMouseFocusWindow called");
    return NULL;
}

void* SLSSetOtherUniversalConnection(void)
{
    if (verbose) puts("STUB: SLSSetOtherUniversalConnection called");
    return NULL;
}

void* SLSSetPerUserConfigurationData(void)
{
    if (verbose) puts("STUB: SLSSetPerUserConfigurationData called");
    return NULL;
}

void* SLSSetPostProcessColorMatrix(void)
{
    if (verbose) puts("STUB: SLSSetPostProcessColorMatrix called");
    return NULL;
}

void* SLSSetPressureConfigurationOverride(void)
{
    if (verbose) puts("STUB: SLSSetPressureConfigurationOverride called");
    return NULL;
}

void* SLSSetPreventsActivation(void)
{
    if (verbose) puts("STUB: SLSSetPreventsActivation called");
    return NULL;
}

void* SLSSetReenableUpdatesCoalesceOnSemaphore(void)
{
    if (verbose) puts("STUB: SLSSetReenableUpdatesCoalesceOnSemaphore called");
    return NULL;
}

void* SLSSetRegisteredCursor(void)
{
    if (verbose) puts("STUB: SLSSetRegisteredCursor called");
    return NULL;
}

void* SLSSetSecureEventInput(void)
{
    if (verbose) puts("STUB: SLSSetSecureEventInput called");
    return NULL;
}

void* SLSSetSessionPropertiesTemporaryBridge(void)
{
    if (verbose) puts("STUB: SLSSetSessionPropertiesTemporaryBridge called");
    return NULL;
}

void* SLSSetSessionSwitchCubeAnimation(void)
{
    if (verbose) puts("STUB: SLSSetSessionSwitchCubeAnimation called");
    return NULL;
}

void* SLSSetSpaceManagementMode(void)
{
    if (verbose) puts("STUB: SLSSetSpaceManagementMode called");
    return NULL;
}

void* SLSSetSurfaceBackgroundBlur(void)
{
    if (verbose) puts("STUB: SLSSetSurfaceBackgroundBlur called");
    return NULL;
}

void* SLSSetSurfaceBounds(void)
{
    if (verbose) puts("STUB: SLSSetSurfaceBounds called");
    return NULL;
}

void* SLSSetSurfaceColorSpace(void)
{
    if (verbose) puts("STUB: SLSSetSurfaceColorSpace called");
    return NULL;
}

void* SLSSetSurfaceLayerBackingOptions(void)
{
    if (verbose) puts("STUB: SLSSetSurfaceLayerBackingOptions called");
    return NULL;
}

void* SLSSetSurfaceListAlpha(void)
{
    if (verbose) puts("STUB: SLSSetSurfaceListAlpha called");
    return NULL;
}

void* SLSSetSurfaceOpacity(void)
{
    if (verbose) puts("STUB: SLSSetSurfaceOpacity called");
    return NULL;
}

void* SLSSetSurfaceProperty(void)
{
    if (verbose) puts("STUB: SLSSetSurfaceProperty called");
    return NULL;
}

void* SLSSetSurfaceProxiesForNextFlush(void)
{
    if (verbose) puts("STUB: SLSSetSurfaceProxiesForNextFlush called");
    return NULL;
}

void* SLSSetSurfaceResolution(void)
{
    if (verbose) puts("STUB: SLSSetSurfaceResolution called");
    return NULL;
}

void* SLSSetSurfaceShape(void)
{
    if (verbose) puts("STUB: SLSSetSurfaceShape called");
    return NULL;
}

void* SLSSetSurfaceVersionedProxiesForNextFlush(void)
{
    if (verbose) puts("STUB: SLSSetSurfaceVersionedProxiesForNextFlush called");
    return NULL;
}

void* SLSSetSwipeScrollDirection(void)
{
    if (verbose) puts("STUB: SLSSetSwipeScrollDirection called");
    return NULL;
}

void* SLSSetSymbolicHotKey(void)
{
    if (verbose) puts("STUB: SLSSetSymbolicHotKey called");
    return NULL;
}

void* SLSSetSymbolicHotKeyButtonValue(void)
{
    if (verbose) puts("STUB: SLSSetSymbolicHotKeyButtonValue called");
    return NULL;
}

void* SLSSetSymbolicHotKeyEnabled(void)
{
    if (verbose) puts("STUB: SLSSetSymbolicHotKeyEnabled called");
    return NULL;
}

void* SLSSetSymbolicHotKeyEnabledForConnection(void)
{
    if (verbose) puts("STUB: SLSSetSymbolicHotKeyEnabledForConnection called");
    return NULL;
}

void* SLSSetSymbolicHotKeyRepresentation(void)
{
    if (verbose) puts("STUB: SLSSetSymbolicHotKeyRepresentation called");
    return NULL;
}

void* SLSSetSymbolicHotKeyValue(void)
{
    if (verbose) puts("STUB: SLSSetSymbolicHotKeyValue called");
    return NULL;
}

void* SLSSetSymbolicHotKeyWithExclusion(void)
{
    if (verbose) puts("STUB: SLSSetSymbolicHotKeyWithExclusion called");
    return NULL;
}

void* SLSSetSymbolicHotKeyWithOptions(void)
{
    if (verbose) puts("STUB: SLSSetSymbolicHotKeyWithOptions called");
    return NULL;
}

void* SLSSetSymbolicHotModifierValue(void)
{
    if (verbose) puts("STUB: SLSSetSymbolicHotModifierValue called");
    return NULL;
}

void* SLSSetSystemDefinedCursor(void)
{
    if (verbose) puts("STUB: SLSSetSystemDefinedCursor called");
    return NULL;
}

void* SLSSetSystemDefinedCursorWithSeed(void)
{
    if (verbose) puts("STUB: SLSSetSystemDefinedCursorWithSeed called");
    return NULL;
}

void* SLSSetSystemStatusBarWindowSubitemOffsets(void)
{
    if (verbose) puts("STUB: SLSSetSystemStatusBarWindowSubitemOffsets called");
    return NULL;
}

void* SLSSetTrackingAreaClip(void)
{
    if (verbose) puts("STUB: SLSSetTrackingAreaClip called");
    return NULL;
}

void* SLSSetTrackingAreaEnabled(void)
{
    if (verbose) puts("STUB: SLSSetTrackingAreaEnabled called");
    return NULL;
}

void* SLSSetTrackingAreaOption(void)
{
    if (verbose) puts("STUB: SLSSetTrackingAreaOption called");
    return NULL;
}

void* SLSSetTrackingAreaRect(void)
{
    if (verbose) puts("STUB: SLSSetTrackingAreaRect called");
    return NULL;
}

void* SLSSetTrackingAreaShape(void)
{
    if (verbose) puts("STUB: SLSSetTrackingAreaShape called");
    return NULL;
}

void* SLSSetTrackingAreasEnabled(void)
{
    if (verbose) puts("STUB: SLSSetTrackingAreasEnabled called");
    return NULL;
}

void* SLSSetUniversalOwner(void)
{
    if (verbose) puts("STUB: SLSSetUniversalOwner called");
    return NULL;
}

void* SLSSetWindowAccelerationState(void)
{
    if (verbose) puts("STUB: SLSSetWindowAccelerationState called");
    return NULL;
}

void* SLSSetWindowActive(void)
{
    if (verbose) puts("STUB: SLSSetWindowActive called");
    return NULL;
}

void* SLSSetWindowAlpha(void)
{
    if (verbose) puts("STUB: SLSSetWindowAlpha called");
    return NULL;
}

void* SLSSetWindowAlphaShape(void)
{
    if (verbose) puts("STUB: SLSSetWindowAlphaShape called");
    return NULL;
}

void* SLSSetWindowAnimationProgress(void)
{
    if (verbose) puts("STUB: SLSSetWindowAnimationProgress called");
    return NULL;
}

void* SLSSetWindowAutofill(void)
{
    if (verbose) puts("STUB: SLSSetWindowAutofill called");
    return NULL;
}

void* SLSSetWindowAutofillColor(void)
{
    if (verbose) puts("STUB: SLSSetWindowAutofillColor called");
    return NULL;
}

void* SLSSetWindowBackgroundBlurRadius(void)
{
    if (verbose) puts("STUB: SLSSetWindowBackgroundBlurRadius called");
    return NULL;
}

void* SLSSetWindowBackgroundBlurRadiusStyle(void)
{
    if (verbose) puts("STUB: SLSSetWindowBackgroundBlurRadiusStyle called");
    return NULL;
}

void* SLSSetWindowBackgroundBlurRadiusWithOpacityHint(void)
{
    if (verbose) puts("STUB: SLSSetWindowBackgroundBlurRadiusWithOpacityHint called");
    return NULL;
}

void* SLSSetWindowCaptureExcludeShape(void)
{
    if (verbose) puts("STUB: SLSSetWindowCaptureExcludeShape called");
    return NULL;
}

void* SLSSetWindowClipShape(void)
{
    if (verbose) puts("STUB: SLSSetWindowClipShape called");
    return NULL;
}

void* SLSSetWindowColorSpace(void)
{
    if (verbose) puts("STUB: SLSSetWindowColorSpace called");
    return NULL;
}

void* SLSSetWindowCornerMask(void)
{
    if (verbose) puts("STUB: SLSSetWindowCornerMask called");
    return NULL;
}

void* SLSSetWindowCursorRestrictionShape(void)
{
    if (verbose) puts("STUB: SLSSetWindowCursorRestrictionShape called");
    return NULL;
}

void* SLSSetWindowDepth(void)
{
    if (verbose) puts("STUB: SLSSetWindowDepth called");
    return NULL;
}

void* SLSSetWindowDidOrderInOnNonCurrentManagedSpacesOnlyNotificationBlock(void)
{
    if (verbose) puts("STUB: SLSSetWindowDidOrderInOnNonCurrentManagedSpacesOnlyNotificationBlock called");
    return NULL;
}

void* SLSSetWindowDidOrderOutOnNonCurrentManagedSpacesNotificationBlock(void)
{
    if (verbose) puts("STUB: SLSSetWindowDidOrderOutOnNonCurrentManagedSpacesNotificationBlock called");
    return NULL;
}

void* SLSSetWindowEventMask(void)
{
    if (verbose) puts("STUB: SLSSetWindowEventMask called");
    return NULL;
}

void* SLSSetWindowEventShape(void)
{
    if (verbose) puts("STUB: SLSSetWindowEventShape called");
    return NULL;
}

void* SLSSetWindowFiltering(void)
{
    if (verbose) puts("STUB: SLSSetWindowFiltering called");
    return NULL;
}

void* SLSSetWindowHasKeyAppearance(void)
{
    if (verbose) puts("STUB: SLSSetWindowHasKeyAppearance called");
    return NULL;
}

void* SLSSetWindowHasMainAppearance(void)
{
    if (verbose) puts("STUB: SLSSetWindowHasMainAppearance called");
    return NULL;
}

void* SLSSetWindowLevel(void)
{
    if (verbose) puts("STUB: SLSSetWindowLevel called");
    return NULL;
}

void* SLSSetWindowLevelForGroup(void)
{
    if (verbose) puts("STUB: SLSSetWindowLevelForGroup called");
    return NULL;
}

void* SLSSetWindowLevelsForActivation(void)
{
    if (verbose) puts("STUB: SLSSetWindowLevelsForActivation called");
    return NULL;
}

void* SLSSetWindowListAlpha(void)
{
    if (verbose) puts("STUB: SLSSetWindowListAlpha called");
    return NULL;
}

void* SLSSetWindowListBrightness(void)
{
    if (verbose) puts("STUB: SLSSetWindowListBrightness called");
    return NULL;
}

void* SLSSetWindowListFadeProperties(void)
{
    if (verbose) puts("STUB: SLSSetWindowListFadeProperties called");
    return NULL;
}

void* SLSSetWindowListGlobalClipShape(void)
{
    if (verbose) puts("STUB: SLSSetWindowListGlobalClipShape called");
    return NULL;
}

void* SLSSetWindowListSystemAlpha(void)
{
    if (verbose) puts("STUB: SLSSetWindowListSystemAlpha called");
    return NULL;
}

void* SLSSetWindowListSystemLevel(void)
{
    if (verbose) puts("STUB: SLSSetWindowListSystemLevel called");
    return NULL;
}

void* SLSSetWindowListTags(void)
{
    if (verbose) puts("STUB: SLSSetWindowListTags called");
    return NULL;
}

void* SLSSetWindowListWorkspace(void)
{
    if (verbose) puts("STUB: SLSSetWindowListWorkspace called");
    return NULL;
}

void* SLSSetWindowMovementGroup(void)
{
    if (verbose) puts("STUB: SLSSetWindowMovementGroup called");
    return NULL;
}

void* SLSSetWindowOpacity(void)
{
    if (verbose) puts("STUB: SLSSetWindowOpacity called");
    return NULL;
}

void* SLSSetWindowOpaqueShape(void)
{
    if (verbose) puts("STUB: SLSSetWindowOpaqueShape called");
    return NULL;
}

void* SLSSetWindowOriginRelativeToWindow(void)
{
    if (verbose) puts("STUB: SLSSetWindowOriginRelativeToWindow called");
    return NULL;
}

void* SLSSetWindowParent(void)
{
    if (verbose) puts("STUB: SLSSetWindowParent called");
    return NULL;
}

void* SLSSetWindowProperty(void)
{
    if (verbose) puts("STUB: SLSSetWindowProperty called");
    return NULL;
}

void* SLSSetWindowRegionsLegacy(void)
{
    if (verbose) puts("STUB: SLSSetWindowRegionsLegacy called");
    return NULL;
}

void* SLSSetWindowResizeState(void)
{
    if (verbose) puts("STUB: SLSSetWindowResizeState called");
    return NULL;
}

void* SLSSetWindowResolution(void)
{
    if (verbose) puts("STUB: SLSSetWindowResolution called");
    return NULL;
}

void* SLSSetWindowShadowAndRimParameters(void)
{
    if (verbose) puts("STUB: SLSSetWindowShadowAndRimParameters called");
    return NULL;
}

void* SLSSetWindowShadowAndRimParametersWithStretch(void)
{
    if (verbose) puts("STUB: SLSSetWindowShadowAndRimParametersWithStretch called");
    return NULL;
}

void* SLSSetWindowShadowGroup(void)
{
    if (verbose) puts("STUB: SLSSetWindowShadowGroup called");
    return NULL;
}

void* SLSSetWindowShadowParameters(void)
{
    if (verbose) puts("STUB: SLSSetWindowShadowParameters called");
    return NULL;
}

void* SLSSetWindowShape(void)
{
    if (verbose) puts("STUB: SLSSetWindowShape called");
    return NULL;
}

void* SLSSetWindowShapeInWindowCoordinates(void)
{
    if (verbose) puts("STUB: SLSSetWindowShapeInWindowCoordinates called");
    return NULL;
}

void* SLSSetWindowShapeInWindowCoordinatesWithWeighting(void)
{
    if (verbose) puts("STUB: SLSSetWindowShapeInWindowCoordinatesWithWeighting called");
    return NULL;
}

void* SLSSetWindowShapeWithWeighting(void)
{
    if (verbose) puts("STUB: SLSSetWindowShapeWithWeighting called");
    return NULL;
}

void* SLSSetWindowSharingState(void)
{
    if (verbose) puts("STUB: SLSSetWindowSharingState called");
    return NULL;
}

void* SLSSetWindowSubLevel(void)
{
    if (verbose) puts("STUB: SLSSetWindowSubLevel called");
    return NULL;
}

void* SLSSetWindowTags(void)
{
    if (verbose) puts("STUB: SLSSetWindowTags called");
    return NULL;
}

void* SLSSetWindowTitle(void)
{
    if (verbose) puts("STUB: SLSSetWindowTitle called");
    return NULL;
}

void* SLSSetWindowToReleaseBackingOnOrderOut(void)
{
    if (verbose) puts("STUB: SLSSetWindowToReleaseBackingOnOrderOut called");
    return NULL;
}

void* SLSSetWindowTransform(void)
{
    if (verbose) puts("STUB: SLSSetWindowTransform called");
    return NULL;
}

void* SLSSetWindowTransformAtPlacement(void)
{
    if (verbose) puts("STUB: SLSSetWindowTransformAtPlacement called");
    return NULL;
}

void* SLSSetWindowTransforms(void)
{
    if (verbose) puts("STUB: SLSSetWindowTransforms called");
    return NULL;
}

void* SLSSetWindowTransformsAtPlacement(void)
{
    if (verbose) puts("STUB: SLSSetWindowTransformsAtPlacement called");
    return NULL;
}

void* SLSSetWindowType(void)
{
    if (verbose) puts("STUB: SLSSetWindowType called");
    return NULL;
}

void* SLSSetWindowWarp(void)
{
    if (verbose) puts("STUB: SLSSetWindowWarp called");
    return NULL;
}

void* SLSSetZoomDisplay(void)
{
    if (verbose) puts("STUB: SLSSetZoomDisplay called");
    return NULL;
}

void* SLSSetZoomParameters(void)
{
    if (verbose) puts("STUB: SLSSetZoomParameters called");
    return NULL;
}

void* SLSShapeSurface(void)
{
    if (verbose) puts("STUB: SLSShapeSurface called");
    return NULL;
}

void* SLSShapeWindow(void)
{
    if (verbose) puts("STUB: SLSShapeWindow called");
    return NULL;
}

void* SLSShapeWindowInWindowCoordinates(void)
{
    if (verbose) puts("STUB: SLSShapeWindowInWindowCoordinates called");
    return NULL;
}

void* SLSShieldCursor(void)
{
    if (verbose) puts("STUB: SLSShieldCursor called");
    return NULL;
}

void* SLSShowCursor(void)
{
    if (verbose) puts("STUB: SLSShowCursor called");
    return NULL;
}

void* SLSShowSpaces(void)
{
    if (verbose) puts("STUB: SLSShowSpaces called");
    return NULL;
}

void* SLSShutdownServerConnections(void)
{
    if (verbose) puts("STUB: SLSShutdownServerConnections called");
    return NULL;
}

void* SLSSnappingInfoAddMovement(void)
{
    if (verbose) puts("STUB: SLSSnappingInfoAddMovement called");
    return NULL;
}

void* SLSSnappingInfoCreate(void)
{
    if (verbose) puts("STUB: SLSSnappingInfoCreate called");
    return NULL;
}

void* SLSSnappingInfoEnumerateSnappingRects(void)
{
    if (verbose) puts("STUB: SLSSnappingInfoEnumerateSnappingRects called");
    return NULL;
}

void* SLSSnappingInfoGetCurrentSnappedEdgesForRect(void)
{
    if (verbose) puts("STUB: SLSSnappingInfoGetCurrentSnappedEdgesForRect called");
    return NULL;
}

void* SLSSnappingInfoGetCurrentVelocity(void)
{
    if (verbose) puts("STUB: SLSSnappingInfoGetCurrentVelocity called");
    return NULL;
}

void* SLSSnappingInfoGetLastSnappedRect(void)
{
    if (verbose) puts("STUB: SLSSnappingInfoGetLastSnappedRect called");
    return NULL;
}

void* SLSSnappingInfoGetSizeSnapRectForFrame(void)
{
    if (verbose) puts("STUB: SLSSnappingInfoGetSizeSnapRectForFrame called");
    return NULL;
}

void* SLSSnappingInfoGetSnappedEdges(void)
{
    if (verbose) puts("STUB: SLSSnappingInfoGetSnappedEdges called");
    return NULL;
}

void* SLSSnappingInfoRelease(void)
{
    if (verbose) puts("STUB: SLSSnappingInfoRelease called");
    return NULL;
}

void* SLSSnappingInfoResetMovement(void)
{
    if (verbose) puts("STUB: SLSSnappingInfoResetMovement called");
    return NULL;
}

void* SLSSnappingInfoSetIsForResizing(void)
{
    if (verbose) puts("STUB: SLSSnappingInfoSetIsForResizing called");
    return NULL;
}

void* SLSSnappingInfoSetPrefs(void)
{
    if (verbose) puts("STUB: SLSSnappingInfoSetPrefs called");
    return NULL;
}

void* SLSSnappingInfoSetSnappedEdges(void)
{
    if (verbose) puts("STUB: SLSSnappingInfoSetSnappedEdges called");
    return NULL;
}

void* SLSSnappingInfoSnapFrameForResizing(void)
{
    if (verbose) puts("STUB: SLSSnappingInfoSnapFrameForResizing called");
    return NULL;
}

void* SLSSnappingInfoSnapOriginWithFrame(void)
{
    if (verbose) puts("STUB: SLSSnappingInfoSnapOriginWithFrame called");
    return NULL;
}

void* SLSSpaceAddOwner(void)
{
    if (verbose) puts("STUB: SLSSpaceAddOwner called");
    return NULL;
}

void* SLSSpaceAddWindowsAndRemoveFromSpaces(void)
{
    if (verbose) puts("STUB: SLSSpaceAddWindowsAndRemoveFromSpaces called");
    return NULL;
}

void* SLSSpaceCanCreateTile(void)
{
    if (verbose) puts("STUB: SLSSpaceCanCreateTile called");
    return NULL;
}

void* SLSSpaceClientDrivenMoveSpacersToPoint(void)
{
    if (verbose) puts("STUB: SLSSpaceClientDrivenMoveSpacersToPoint called");
    return NULL;
}

void* SLSSpaceCopyManagedShape(void)
{
    if (verbose) puts("STUB: SLSSpaceCopyManagedShape called");
    return NULL;
}

void* SLSSpaceCopyName(void)
{
    if (verbose) puts("STUB: SLSSpaceCopyName called");
    return NULL;
}

void* SLSSpaceCopyOwners(void)
{
    if (verbose) puts("STUB: SLSSpaceCopyOwners called");
    return NULL;
}

void* SLSSpaceCopyShape(void)
{
    if (verbose) puts("STUB: SLSSpaceCopyShape called");
    return NULL;
}

void* SLSSpaceCopyTileSpaces(void)
{
    if (verbose) puts("STUB: SLSSpaceCopyTileSpaces called");
    return NULL;
}

void* SLSSpaceCopyValues(void)
{
    if (verbose) puts("STUB: SLSSpaceCopyValues called");
    return NULL;
}

void* SLSSpaceCreate(void)
{
    if (verbose) puts("STUB: SLSSpaceCreate called");
    return NULL;
}

void* SLSSpaceCreateTile(void)
{
    if (verbose) puts("STUB: SLSSpaceCreateTile called");
    return NULL;
}

void* SLSSpaceDestroy(void)
{
    if (verbose) puts("STUB: SLSSpaceDestroy called");
    return NULL;
}

void* SLSSpaceFinishedResizeForRect(void)
{
    if (verbose) puts("STUB: SLSSpaceFinishedResizeForRect called");
    return NULL;
}

void* SLSSpaceGetAbsoluteLevel(void)
{
    if (verbose) puts("STUB: SLSSpaceGetAbsoluteLevel called");
    return NULL;
}

void* SLSSpaceGetAlpha(void)
{
    if (verbose) puts("STUB: SLSSpaceGetAlpha called");
    return NULL;
}

void* SLSSpaceGetCompatID(void)
{
    if (verbose) puts("STUB: SLSSpaceGetCompatID called");
    return NULL;
}

void* SLSSpaceGetInterTileSpacing(void)
{
    if (verbose) puts("STUB: SLSSpaceGetInterTileSpacing called");
    return NULL;
}

void* SLSSpaceGetRect(void)
{
    if (verbose) puts("STUB: SLSSpaceGetRect called");
    return NULL;
}

void* SLSSpaceGetSizeForProposedTile(void)
{
    if (verbose) puts("STUB: SLSSpaceGetSizeForProposedTile called");
    return NULL;
}

void* SLSSpaceGetSpacersAtPoint(void)
{
    if (verbose) puts("STUB: SLSSpaceGetSpacersAtPoint called");
    return NULL;
}

void* SLSSpaceGetTransform(void)
{
    if (verbose) puts("STUB: SLSSpaceGetTransform called");
    return NULL;
}

void* SLSSpaceGetType(void)
{
    if (verbose) puts("STUB: SLSSpaceGetType called");
    return NULL;
}

void* SLSSpaceIteratorAdvance(void)
{
    if (verbose) puts("STUB: SLSSpaceIteratorAdvance called");
    return NULL;
}

void* SLSSpaceIteratorGetAbsoluteLevel(void)
{
    if (verbose) puts("STUB: SLSSpaceIteratorGetAbsoluteLevel called");
    return NULL;
}

void* SLSSpaceIteratorGetAttributes(void)
{
    if (verbose) puts("STUB: SLSSpaceIteratorGetAttributes called");
    return NULL;
}

void* SLSSpaceIteratorGetCount(void)
{
    if (verbose) puts("STUB: SLSSpaceIteratorGetCount called");
    return NULL;
}

void* SLSSpaceIteratorGetParentSpaceID(void)
{
    if (verbose) puts("STUB: SLSSpaceIteratorGetParentSpaceID called");
    return NULL;
}

void* SLSSpaceIteratorGetSpaceID(void)
{
    if (verbose) puts("STUB: SLSSpaceIteratorGetSpaceID called");
    return NULL;
}

void* SLSSpaceIteratorGetType(void)
{
    if (verbose) puts("STUB: SLSSpaceIteratorGetType called");
    return NULL;
}

void* SLSSpaceIteratorGetTypeID(void)
{
    if (verbose) puts("STUB: SLSSpaceIteratorGetTypeID called");
    return NULL;
}

void* SLSSpaceMoveSpacersToPoint(void)
{
    if (verbose) puts("STUB: SLSSpaceMoveSpacersToPoint called");
    return NULL;
}

void* SLSSpacePreferCurrentDisplay(void)
{
    if (verbose) puts("STUB: SLSSpacePreferCurrentDisplay called");
    return NULL;
}

void* SLSSpaceRemoveOwner(void)
{
    if (verbose) puts("STUB: SLSSpaceRemoveOwner called");
    return NULL;
}

void* SLSSpaceRemoveValuesForKeys(void)
{
    if (verbose) puts("STUB: SLSSpaceRemoveValuesForKeys called");
    return NULL;
}

void* SLSSpaceResetMenuBar(void)
{
    if (verbose) puts("STUB: SLSSpaceResetMenuBar called");
    return NULL;
}

void* SLSSpaceSetAbsoluteLevel(void)
{
    if (verbose) puts("STUB: SLSSpaceSetAbsoluteLevel called");
    return NULL;
}

void* SLSSpaceSetAlpha(void)
{
    if (verbose) puts("STUB: SLSSpaceSetAlpha called");
    return NULL;
}

void* SLSSpaceSetCompatID(void)
{
    if (verbose) puts("STUB: SLSSpaceSetCompatID called");
    return NULL;
}

void* SLSSpaceSetEdgeReservation(void)
{
    if (verbose) puts("STUB: SLSSpaceSetEdgeReservation called");
    return NULL;
}

void* SLSSpaceSetFrontPSN(void)
{
    if (verbose) puts("STUB: SLSSpaceSetFrontPSN called");
    return NULL;
}

void* SLSSpaceSetInterTileSpacing(void)
{
    if (verbose) puts("STUB: SLSSpaceSetInterTileSpacing called");
    return NULL;
}

void* SLSSpaceSetName(void)
{
    if (verbose) puts("STUB: SLSSpaceSetName called");
    return NULL;
}

void* SLSSpaceSetOrderingWeight(void)
{
    if (verbose) puts("STUB: SLSSpaceSetOrderingWeight called");
    return NULL;
}

void* SLSSpaceSetOwners(void)
{
    if (verbose) puts("STUB: SLSSpaceSetOwners called");
    return NULL;
}

void* SLSSpaceSetRelativeLevel(void)
{
    if (verbose) puts("STUB: SLSSpaceSetRelativeLevel called");
    return NULL;
}

void* SLSSpaceSetShape(void)
{
    if (verbose) puts("STUB: SLSSpaceSetShape called");
    return NULL;
}

void* SLSSpaceSetTransform(void)
{
    if (verbose) puts("STUB: SLSSpaceSetTransform called");
    return NULL;
}

void* SLSSpaceSetType(void)
{
    if (verbose) puts("STUB: SLSSpaceSetType called");
    return NULL;
}

void* SLSSpaceSetValues(void)
{
    if (verbose) puts("STUB: SLSSpaceSetValues called");
    return NULL;
}

void* SLSSpaceTileMoveToSpaceAtIndex(void)
{
    if (verbose) puts("STUB: SLSSpaceTileMoveToSpaceAtIndex called");
    return NULL;
}

void* SLSSpaceWithName(void)
{
    if (verbose) puts("STUB: SLSSpaceWithName called");
    return NULL;
}

void* SLSSpinControlDecodeMessage(void)
{
    if (verbose) puts("STUB: SLSSpinControlDecodeMessage called");
    return NULL;
}

void* SLSSpinControlSetNotificationPort(void)
{
    if (verbose) puts("STUB: SLSSpinControlSetNotificationPort called");
    return NULL;
}

void* SLSStallServer(void)
{
    if (verbose) puts("STUB: SLSStallServer called");
    return NULL;
}

void* SLSStatusItemNavigationRecordGetCurrentNavigationOptions(void)
{
    if (verbose) puts("STUB: SLSStatusItemNavigationRecordGetCurrentNavigationOptions called");
    return NULL;
}

void* SLSStatusItemNavigationRecordGetCurrentNavigationWindow(void)
{
    if (verbose) puts("STUB: SLSStatusItemNavigationRecordGetCurrentNavigationWindow called");
    return NULL;
}

void* SLSStructuralRegionMove(void)
{
    if (verbose) puts("STUB: SLSStructuralRegionMove called");
    return NULL;
}

void* SLSStructuralRegionSetChildRegionTransform(void)
{
    if (verbose) puts("STUB: SLSStructuralRegionSetChildRegionTransform called");
    return NULL;
}

void* SLSStructuralRegionSetClippedByWindow(void)
{
    if (verbose) puts("STUB: SLSStructuralRegionSetClippedByWindow called");
    return NULL;
}

void* SLSStructuralRegionSetEventMask(void)
{
    if (verbose) puts("STUB: SLSStructuralRegionSetEventMask called");
    return NULL;
}

void* SLSStructuralRegionSetLevel(void)
{
    if (verbose) puts("STUB: SLSStructuralRegionSetLevel called");
    return NULL;
}

void* SLSStructuralRegionSetShape(void)
{
    if (verbose) puts("STUB: SLSStructuralRegionSetShape called");
    return NULL;
}

void* SLSSuperShieldingWindowID(void)
{
    if (verbose) puts("STUB: SLSSuperShieldingWindowID called");
    return NULL;
}

void* SLSSwitchConsoleToSession(void)
{
    if (verbose) puts("STUB: SLSSwitchConsoleToSession called");
    return NULL;
}

void* SLSSystemStatusBarGetCurrentNavigationWindow(void)
{
    if (verbose) puts("STUB: SLSSystemStatusBarGetCurrentNavigationWindow called");
    return NULL;
}

void* SLSSystemStatusBarGetWindowPreferredPosition(void)
{
    if (verbose) puts("STUB: SLSSystemStatusBarGetWindowPreferredPosition called");
    return NULL;
}

void* SLSSystemStatusBarNavigateFromWindow(void)
{
    if (verbose) puts("STUB: SLSSystemStatusBarNavigateFromWindow called");
    return NULL;
}

void* SLSSystemStatusBarRegisterReplicantWindow(void)
{
    if (verbose) puts("STUB: SLSSystemStatusBarRegisterReplicantWindow called");
    return NULL;
}

void* SLSSystemStatusBarRegisterSortedWindow(void)
{
    if (verbose) puts("STUB: SLSSystemStatusBarRegisterSortedWindow called");
    return NULL;
}

void* SLSSystemStatusBarRegisterWindow(void)
{
    if (verbose) puts("STUB: SLSSystemStatusBarRegisterWindow called");
    return NULL;
}

void* SLSSystemStatusBarWindowIsDraggedOutOfBar(void)
{
    if (verbose) puts("STUB: SLSSystemStatusBarWindowIsDraggedOutOfBar called");
    return NULL;
}

void* SLSTileEvictionRecordGetManagedSpaceID(void)
{
    if (verbose) puts("STUB: SLSTileEvictionRecordGetManagedSpaceID called");
    return NULL;
}

void* SLSTileEvictionRecordGetTileID(void)
{
    if (verbose) puts("STUB: SLSTileEvictionRecordGetTileID called");
    return NULL;
}

void* SLSTileOwnerChangeRecordGetManagedSpaceID(void)
{
    if (verbose) puts("STUB: SLSTileOwnerChangeRecordGetManagedSpaceID called");
    return NULL;
}

void* SLSTileOwnerChangeRecordGetNewOwner(void)
{
    if (verbose) puts("STUB: SLSTileOwnerChangeRecordGetNewOwner called");
    return NULL;
}

void* SLSTileOwnerChangeRecordGetOldOwner(void)
{
    if (verbose) puts("STUB: SLSTileOwnerChangeRecordGetOldOwner called");
    return NULL;
}

void* SLSTileOwnerChangeRecordGetTileID(void)
{
    if (verbose) puts("STUB: SLSTileOwnerChangeRecordGetTileID called");
    return NULL;
}

void* SLSTileSpaceMoveSpacersForSize(void)
{
    if (verbose) puts("STUB: SLSTileSpaceMoveSpacersForSize called");
    return NULL;
}

void* SLSTileSpaceReplaceWithSnapshotWindow(void)
{
    if (verbose) puts("STUB: SLSTileSpaceReplaceWithSnapshotWindow called");
    return NULL;
}

void* SLSTileSpaceResizeRecordGetParentSpaceID(void)
{
    if (verbose) puts("STUB: SLSTileSpaceResizeRecordGetParentSpaceID called");
    return NULL;
}

void* SLSTileSpaceResizeRecordGetSpaceID(void)
{
    if (verbose) puts("STUB: SLSTileSpaceResizeRecordGetSpaceID called");
    return NULL;
}

void* SLSTileSpaceResizeRecordIsLiveResizing(void)
{
    if (verbose) puts("STUB: SLSTileSpaceResizeRecordIsLiveResizing called");
    return NULL;
}

void* SLSTileSpaceSetDividerWindow(void)
{
    if (verbose) puts("STUB: SLSTileSpaceSetDividerWindow called");
    return NULL;
}

void* SLSToggleDisplayZoom(void)
{
    if (verbose) puts("STUB: SLSToggleDisplayZoom called");
    return NULL;
}

void* SLSTouchBarServerRegister(void)
{
    if (verbose) puts("STUB: SLSTouchBarServerRegister called");
    return NULL;
}

void* SLSTrackingRegionSetForceConfig(void)
{
    if (verbose) puts("STUB: SLSTrackingRegionSetForceConfig called");
    return NULL;
}

void* SLSTrackingRegionSetIsEnabled(void)
{
    if (verbose) puts("STUB: SLSTrackingRegionSetIsEnabled called");
    return NULL;
}

void* SLSTrackingRegionSetRespectsSystemAlphaThreshold(void)
{
    if (verbose) puts("STUB: SLSTrackingRegionSetRespectsSystemAlphaThreshold called");
    return NULL;
}

void* SLSTransactionAddWindowToSpace(void)
{
    if (verbose) puts("STUB: SLSTransactionAddWindowToSpace called");
    return NULL;
}

void* SLSTransactionAddWindowToSpaceAndRemoveFromSpaces(void)
{
    if (verbose) puts("STUB: SLSTransactionAddWindowToSpaceAndRemoveFromSpaces called");
    return NULL;
}

void* SLSTransactionBindSurface(void)
{
    if (verbose) puts("STUB: SLSTransactionBindSurface called");
    return NULL;
}

void* SLSTransactionClearMenuBarSystemOverrideAlphas(void)
{
    if (verbose) puts("STUB: SLSTransactionClearMenuBarSystemOverrideAlphas called");
    return NULL;
}

void* SLSTransactionClearWindowLockedBounds(void)
{
    if (verbose) puts("STUB: SLSTransactionClearWindowLockedBounds called");
    return NULL;
}

void* SLSTransactionClearWindowSystemLevel(void)
{
    if (verbose) puts("STUB: SLSTransactionClearWindowSystemLevel called");
    return NULL;
}

void* SLSTransactionCommit(void)
{
    if (verbose) puts("STUB: SLSTransactionCommit called");
    return NULL;
}

void* SLSTransactionCommitCoalescing(void)
{
    if (verbose) puts("STUB: SLSTransactionCommitCoalescing called");
    return NULL;
}

void* SLSTransactionCreate(void)
{
    if (verbose) puts("STUB: SLSTransactionCreate called");
    return NULL;
}

void* SLSTransactionDeferWindowMoveEvents(void)
{
    if (verbose) puts("STUB: SLSTransactionDeferWindowMoveEvents called");
    return NULL;
}

void* SLSTransactionDestroySpace(void)
{
    if (verbose) puts("STUB: SLSTransactionDestroySpace called");
    return NULL;
}

void* SLSTransactionGetTypeID(void)
{
    if (verbose) puts("STUB: SLSTransactionGetTypeID called");
    return NULL;
}

void* SLSTransactionHideSpace(void)
{
    if (verbose) puts("STUB: SLSTransactionHideSpace called");
    return NULL;
}

void* SLSTransactionMoveWindowWithGroup(void)
{
    if (verbose) puts("STUB: SLSTransactionMoveWindowWithGroup called");
    return NULL;
}

void* SLSTransactionMoveWindowsToManagedSpace(void)
{
    if (verbose) puts("STUB: SLSTransactionMoveWindowsToManagedSpace called");
    return NULL;
}

void* SLSTransactionOrderSurface(void)
{
    if (verbose) puts("STUB: SLSTransactionOrderSurface called");
    return NULL;
}

void* SLSTransactionOrderWindow(void)
{
    if (verbose) puts("STUB: SLSTransactionOrderWindow called");
    return NULL;
}

void* SLSTransactionOrderWindowGroup(void)
{
    if (verbose) puts("STUB: SLSTransactionOrderWindowGroup called");
    return NULL;
}

void* SLSTransactionOverrideAppSleepNotifications(void)
{
    if (verbose) puts("STUB: SLSTransactionOverrideAppSleepNotifications called");
    return NULL;
}

void* SLSTransactionPostBroadcastNotification(void)
{
    if (verbose) puts("STUB: SLSTransactionPostBroadcastNotification called");
    return NULL;
}

void* SLSTransactionPostNotificationToConnection(void)
{
    if (verbose) puts("STUB: SLSTransactionPostNotificationToConnection called");
    return NULL;
}

void* SLSTransactionRemoveWindowFromSpace(void)
{
    if (verbose) puts("STUB: SLSTransactionRemoveWindowFromSpace called");
    return NULL;
}

void* SLSTransactionRemoveWindowFromSpaces(void)
{
    if (verbose) puts("STUB: SLSTransactionRemoveWindowFromSpaces called");
    return NULL;
}

void* SLSTransactionResetSpaceMenuBar(void)
{
    if (verbose) puts("STUB: SLSTransactionResetSpaceMenuBar called");
    return NULL;
}

void* SLSTransactionResetWindow(void)
{
    if (verbose) puts("STUB: SLSTransactionResetWindow called");
    return NULL;
}

void* SLSTransactionResetWindowSubLevel(void)
{
    if (verbose) puts("STUB: SLSTransactionResetWindowSubLevel called");
    return NULL;
}

void* SLSTransactionSetBackdropChameleonContribution(void)
{
    if (verbose) puts("STUB: SLSTransactionSetBackdropChameleonContribution called");
    return NULL;
}

void* SLSTransactionSetChameleonUpdatesEnabled(void)
{
    if (verbose) puts("STUB: SLSTransactionSetChameleonUpdatesEnabled called");
    return NULL;
}

void* SLSTransactionSetClientAdvisory(void)
{
    if (verbose) puts("STUB: SLSTransactionSetClientAdvisory called");
    return NULL;
}

void* SLSTransactionSetEventCapture(void)
{
    if (verbose) puts("STUB: SLSTransactionSetEventCapture called");
    return NULL;
}

void* SLSTransactionSetManagedDisplayCurrentSpace(void)
{
    if (verbose) puts("STUB: SLSTransactionSetManagedDisplayCurrentSpace called");
    return NULL;
}

void* SLSTransactionSetManagedDisplayIsAnimating(void)
{
    if (verbose) puts("STUB: SLSTransactionSetManagedDisplayIsAnimating called");
    return NULL;
}

void* SLSTransactionSetMenuBarBounds(void)
{
    if (verbose) puts("STUB: SLSTransactionSetMenuBarBounds called");
    return NULL;
}

void* SLSTransactionSetMenuBarSystemOverrideAlpha(void)
{
    if (verbose) puts("STUB: SLSTransactionSetMenuBarSystemOverrideAlpha called");
    return NULL;
}

void* SLSTransactionSetSpaceAbsoluteLevel(void)
{
    if (verbose) puts("STUB: SLSTransactionSetSpaceAbsoluteLevel called");
    return NULL;
}

void* SLSTransactionSetSpaceAlpha(void)
{
    if (verbose) puts("STUB: SLSTransactionSetSpaceAlpha called");
    return NULL;
}

void* SLSTransactionSetSpaceOrderingWeight(void)
{
    if (verbose) puts("STUB: SLSTransactionSetSpaceOrderingWeight called");
    return NULL;
}

void* SLSTransactionSetSpaceShape(void)
{
    if (verbose) puts("STUB: SLSTransactionSetSpaceShape called");
    return NULL;
}

void* SLSTransactionSetSpaceTransform(void)
{
    if (verbose) puts("STUB: SLSTransactionSetSpaceTransform called");
    return NULL;
}

void* SLSTransactionSetSurfaceBounds(void)
{
    if (verbose) puts("STUB: SLSTransactionSetSurfaceBounds called");
    return NULL;
}

void* SLSTransactionSetSurfaceOpacity(void)
{
    if (verbose) puts("STUB: SLSTransactionSetSurfaceOpacity called");
    return NULL;
}

void* SLSTransactionSetSurfaceResolution(void)
{
    if (verbose) puts("STUB: SLSTransactionSetSurfaceResolution called");
    return NULL;
}

void* SLSTransactionSetWindowAlpha(void)
{
    if (verbose) puts("STUB: SLSTransactionSetWindowAlpha called");
    return NULL;
}

void* SLSTransactionSetWindowBrightness(void)
{
    if (verbose) puts("STUB: SLSTransactionSetWindowBrightness called");
    return NULL;
}

void* SLSTransactionSetWindowGlobalClipShape(void)
{
    if (verbose) puts("STUB: SLSTransactionSetWindowGlobalClipShape called");
    return NULL;
}

void* SLSTransactionSetWindowLevel(void)
{
    if (verbose) puts("STUB: SLSTransactionSetWindowLevel called");
    return NULL;
}

void* SLSTransactionSetWindowLockedBounds(void)
{
    if (verbose) puts("STUB: SLSTransactionSetWindowLockedBounds called");
    return NULL;
}

void* SLSTransactionSetWindowOpaqueShape(void)
{
    if (verbose) puts("STUB: SLSTransactionSetWindowOpaqueShape called");
    return NULL;
}

void* SLSTransactionSetWindowProperty(void)
{
    if (verbose) puts("STUB: SLSTransactionSetWindowProperty called");
    return NULL;
}

void* SLSTransactionSetWindowReleasesBackingOnOrderOut(void)
{
    if (verbose) puts("STUB: SLSTransactionSetWindowReleasesBackingOnOrderOut called");
    return NULL;
}

void* SLSTransactionSetWindowResolution(void)
{
    if (verbose) puts("STUB: SLSTransactionSetWindowResolution called");
    return NULL;
}

void* SLSTransactionSetWindowShape(void)
{
    if (verbose) puts("STUB: SLSTransactionSetWindowShape called");
    return NULL;
}

void* SLSTransactionSetWindowSubLevel(void)
{
    if (verbose) puts("STUB: SLSTransactionSetWindowSubLevel called");
    return NULL;
}

void* SLSTransactionSetWindowSystemAlpha(void)
{
    if (verbose) puts("STUB: SLSTransactionSetWindowSystemAlpha called");
    return NULL;
}

void* SLSTransactionSetWindowSystemLevel(void)
{
    if (verbose) puts("STUB: SLSTransactionSetWindowSystemLevel called");
    return NULL;
}

void* SLSTransactionSetWindowTransform(void)
{
    if (verbose) puts("STUB: SLSTransactionSetWindowTransform called");
    return NULL;
}

void* SLSTransactionSetWindowWarp(void)
{
    if (verbose) puts("STUB: SLSTransactionSetWindowWarp called");
    return NULL;
}

void* SLSTransactionShowSpace(void)
{
    if (verbose) puts("STUB: SLSTransactionShowSpace called");
    return NULL;
}

void* SLSTransactionSpaceTileMoveToSpaceAtIndex(void)
{
    if (verbose) puts("STUB: SLSTransactionSpaceTileMoveToSpaceAtIndex called");
    return NULL;
}

void* SLSTransactionUpdateRegion(void)
{
    if (verbose) puts("STUB: SLSTransactionUpdateRegion called");
    return NULL;
}

void* SLSTransactionWillSwitchSpaces(void)
{
    if (verbose) puts("STUB: SLSTransactionWillSwitchSpaces called");
    return NULL;
}

void* SLSTranslatePointForWindow(void)
{
    if (verbose) puts("STUB: SLSTranslatePointForWindow called");
    return NULL;
}

void* SLSUniqueSurfaceID(void)
{
    if (verbose) puts("STUB: SLSUniqueSurfaceID called");
    return NULL;
}

void* SLSUnlockWindowBackingStore(void)
{
    if (verbose) puts("STUB: SLSUnlockWindowBackingStore called");
    return NULL;
}

void* SLSUnlockWindowBits(void)
{
    if (verbose) puts("STUB: SLSUnlockWindowBits called");
    return NULL;
}

void* SLSUnlockWindowRectBits(void)
{
    if (verbose) puts("STUB: SLSUnlockWindowRectBits called");
    return NULL;
}

void* SLSUnlockWindowVisibleRegion(void)
{
    if (verbose) puts("STUB: SLSUnlockWindowVisibleRegion called");
    return NULL;
}

void* SLSUnpackDeferredUpdateNotification(void)
{
    if (verbose) puts("STUB: SLSUnpackDeferredUpdateNotification called");
    return NULL;
}

void* SLSUnregisterUpdateInformationHandler(void)
{
    if (verbose) puts("STUB: SLSUnregisterUpdateInformationHandler called");
    return NULL;
}

void* SLSUnregisterWindowWithSystemStatusBar(void)
{
    if (verbose) puts("STUB: SLSUnregisterWindowWithSystemStatusBar called");
    return NULL;
}

void* SLSUnshieldCursor(void)
{
    if (verbose) puts("STUB: SLSUnshieldCursor called");
    return NULL;
}

void* SLSUpdateCurrentSessionWorkspaceData(void)
{
    if (verbose) puts("STUB: SLSUpdateCurrentSessionWorkspaceData called");
    return NULL;
}

void* SLSUpdateLastEventTime(void)
{
    if (verbose) puts("STUB: SLSUpdateLastEventTime called");
    return NULL;
}

void* SLSUpdateSystemActivity(void)
{
    if (verbose) puts("STUB: SLSUpdateSystemActivity called");
    return NULL;
}

void* SLSUpdateWindowAnimationEdges(void)
{
    if (verbose) puts("STUB: SLSUpdateWindowAnimationEdges called");
    return NULL;
}

void* SLSUpdateWindowAnimationOrigin(void)
{
    if (verbose) puts("STUB: SLSUpdateWindowAnimationOrigin called");
    return NULL;
}

void* SLSValidateUnobscuredLayerSecurityToken(void)
{
    if (verbose) puts("STUB: SLSValidateUnobscuredLayerSecurityToken called");
    return NULL;
}

void* SLSWaitOnCoalescedUpdates(void)
{
    if (verbose) puts("STUB: SLSWaitOnCoalescedUpdates called");
    return NULL;
}

void* SLSWarpCursorPosition(void)
{
    if (verbose) puts("STUB: SLSWarpCursorPosition called");
    return NULL;
}

void* SLSWasBootApplePreserved(void)
{
    if (verbose) puts("STUB: SLSWasBootApplePreserved called");
    return NULL;
}

void* SLSWillSwitchSpaces(void)
{
    if (verbose) puts("STUB: SLSWillSwitchSpaces called");
    return NULL;
}

void* SLSWindowAnimationChangeLevel(void)
{
    if (verbose) puts("STUB: SLSWindowAnimationChangeLevel called");
    return NULL;
}

void* SLSWindowAnimationChangeShiftPoint(void)
{
    if (verbose) puts("STUB: SLSWindowAnimationChangeShiftPoint called");
    return NULL;
}

void* SLSWindowAnimationChangeSubLevel(void)
{
    if (verbose) puts("STUB: SLSWindowAnimationChangeSubLevel called");
    return NULL;
}

void* SLSWindowAnimationHideSlot(void)
{
    if (verbose) puts("STUB: SLSWindowAnimationHideSlot called");
    return NULL;
}

void* SLSWindowAnimationSetParent(void)
{
    if (verbose) puts("STUB: SLSWindowAnimationSetParent called");
    return NULL;
}

void* SLSWindowBackdropActivate(void)
{
    if (verbose) puts("STUB: SLSWindowBackdropActivate called");
    return NULL;
}

void* SLSWindowBackdropActivateAll(void)
{
    if (verbose) puts("STUB: SLSWindowBackdropActivateAll called");
    return NULL;
}

void* SLSWindowBackdropBlendModeStringToEnum(void)
{
    if (verbose) puts("STUB: SLSWindowBackdropBlendModeStringToEnum called");
    return NULL;
}

void* SLSWindowBackdropCreateWithLevel(void)
{
    if (verbose) puts("STUB: SLSWindowBackdropCreateWithLevel called");
    return NULL;
}

void* SLSWindowBackdropCreateWithLevelAndMask(void)
{
    if (verbose) puts("STUB: SLSWindowBackdropCreateWithLevelAndMask called");
    return NULL;
}

void* SLSWindowBackdropCreateWithLevelAndTintColor(void)
{
    if (verbose) puts("STUB: SLSWindowBackdropCreateWithLevelAndTintColor called");
    return NULL;
}

void* SLSWindowBackdropDeactivate(void)
{
    if (verbose) puts("STUB: SLSWindowBackdropDeactivate called");
    return NULL;
}

void* SLSWindowBackdropDeactivateAll(void)
{
    if (verbose) puts("STUB: SLSWindowBackdropDeactivateAll called");
    return NULL;
}

void* SLSWindowBackdropGetTypeID(void)
{
    if (verbose) puts("STUB: SLSWindowBackdropGetTypeID called");
    return NULL;
}

void* SLSWindowBackdropMaterialStringToEnum(void)
{
    if (verbose) puts("STUB: SLSWindowBackdropMaterialStringToEnum called");
    return NULL;
}

void* SLSWindowBackdropRelease(void)
{
    if (verbose) puts("STUB: SLSWindowBackdropRelease called");
    return NULL;
}

void* SLSWindowBackdropSetSaturation(void)
{
    if (verbose) puts("STUB: SLSWindowBackdropSetSaturation called");
    return NULL;
}

void* SLSWindowBackdropSetSelectionTintColor(void)
{
    if (verbose) puts("STUB: SLSWindowBackdropSetSelectionTintColor called");
    return NULL;
}

void* SLSWindowDirtyRegionSeed(void)
{
    if (verbose) puts("STUB: SLSWindowDirtyRegionSeed called");
    return NULL;
}

void* SLSWindowFreezeBackdrops(void)
{
    if (verbose) puts("STUB: SLSWindowFreezeBackdrops called");
    return NULL;
}

void* SLSWindowFreezeWithOptions(void)
{
    if (verbose) puts("STUB: SLSWindowFreezeWithOptions called");
    return NULL;
}

void* SLSWindowGetTileRect(void)
{
    if (verbose) puts("STUB: SLSWindowGetTileRect called");
    return NULL;
}

void* SLSWindowIsOnCurrentSpace(void)
{
    if (verbose) puts("STUB: SLSWindowIsOnCurrentSpace called");
    return NULL;
}

void* SLSWindowIsOrderedIn(void)
{
    if (verbose) puts("STUB: SLSWindowIsOrderedIn called");
    return NULL;
}

void* SLSWindowIsVisible(void)
{
    if (verbose) puts("STUB: SLSWindowIsVisible called");
    return NULL;
}

void* SLSWindowIteratorAdvance(void)
{
    if (verbose) puts("STUB: SLSWindowIteratorAdvance called");
    return NULL;
}

void* SLSWindowIteratorCopyAttachedWindows(void)
{
    if (verbose) puts("STUB: SLSWindowIteratorCopyAttachedWindows called");
    return NULL;
}

void* SLSWindowIteratorCopySpaces(void)
{
    if (verbose) puts("STUB: SLSWindowIteratorCopySpaces called");
    return NULL;
}

void* SLSWindowIteratorCopyTitle(void)
{
    if (verbose) puts("STUB: SLSWindowIteratorCopyTitle called");
    return NULL;
}

void* SLSWindowIteratorGetAlpha(void)
{
    if (verbose) puts("STUB: SLSWindowIteratorGetAlpha called");
    return NULL;
}

void* SLSWindowIteratorGetAttachedWindowCount(void)
{
    if (verbose) puts("STUB: SLSWindowIteratorGetAttachedWindowCount called");
    return NULL;
}

void* SLSWindowIteratorGetAttributes(void)
{
    if (verbose) puts("STUB: SLSWindowIteratorGetAttributes called");
    return NULL;
}

void* SLSWindowIteratorGetBounds(void)
{
    if (verbose) puts("STUB: SLSWindowIteratorGetBounds called");
    return NULL;
}

void* SLSWindowIteratorGetConstraints(void)
{
    if (verbose) puts("STUB: SLSWindowIteratorGetConstraints called");
    return NULL;
}

void* SLSWindowIteratorGetCount(void)
{
    if (verbose) puts("STUB: SLSWindowIteratorGetCount called");
    return NULL;
}

void* SLSWindowIteratorGetFrameBounds(void)
{
    if (verbose) puts("STUB: SLSWindowIteratorGetFrameBounds called");
    return NULL;
}

void* SLSWindowIteratorGetLevel(void)
{
    if (verbose) puts("STUB: SLSWindowIteratorGetLevel called");
    return NULL;
}

void* SLSWindowIteratorGetMatchingSpaceID(void)
{
    if (verbose) puts("STUB: SLSWindowIteratorGetMatchingSpaceID called");
    return NULL;
}

void* SLSWindowIteratorGetOwner(void)
{
    if (verbose) puts("STUB: SLSWindowIteratorGetOwner called");
    return NULL;
}

void* SLSWindowIteratorGetPID(void)
{
    if (verbose) puts("STUB: SLSWindowIteratorGetPID called");
    return NULL;
}

void* SLSWindowIteratorGetPSN(void)
{
    if (verbose) puts("STUB: SLSWindowIteratorGetPSN called");
    return NULL;
}

void* SLSWindowIteratorGetParentID(void)
{
    if (verbose) puts("STUB: SLSWindowIteratorGetParentID called");
    return NULL;
}

void* SLSWindowIteratorGetScreenRect(void)
{
    if (verbose) puts("STUB: SLSWindowIteratorGetScreenRect called");
    return NULL;
}

void* SLSWindowIteratorGetSpaceAttributes(void)
{
    if (verbose) puts("STUB: SLSWindowIteratorGetSpaceAttributes called");
    return NULL;
}

void* SLSWindowIteratorGetSpaceCount(void)
{
    if (verbose) puts("STUB: SLSWindowIteratorGetSpaceCount called");
    return NULL;
}

void* SLSWindowIteratorGetSpaceTypeMask(void)
{
    if (verbose) puts("STUB: SLSWindowIteratorGetSpaceTypeMask called");
    return NULL;
}

void* SLSWindowIteratorGetTags(void)
{
    if (verbose) puts("STUB: SLSWindowIteratorGetTags called");
    return NULL;
}

void* SLSWindowIteratorGetTypeID(void)
{
    if (verbose) puts("STUB: SLSWindowIteratorGetTypeID called");
    return NULL;
}

void* SLSWindowIteratorGetWindowID(void)
{
    if (verbose) puts("STUB: SLSWindowIteratorGetWindowID called");
    return NULL;
}

void* SLSWindowIteratorIsInSpace(void)
{
    if (verbose) puts("STUB: SLSWindowIteratorIsInSpace called");
    return NULL;
}

void* SLSWindowListSetLockedBounds(void)
{
    if (verbose) puts("STUB: SLSWindowListSetLockedBounds called");
    return NULL;
}

void* SLSWindowQueryCopyValue(void)
{
    if (verbose) puts("STUB: SLSWindowQueryCopyValue called");
    return NULL;
}

void* SLSWindowQueryCreate(void)
{
    if (verbose) puts("STUB: SLSWindowQueryCreate called");
    return NULL;
}

void* SLSWindowQueryGetTypeID(void)
{
    if (verbose) puts("STUB: SLSWindowQueryGetTypeID called");
    return NULL;
}

void* SLSWindowQueryResultCopyManagedDisplays(void)
{
    if (verbose) puts("STUB: SLSWindowQueryResultCopyManagedDisplays called");
    return NULL;
}

void* SLSWindowQueryResultCopySpaces(void)
{
    if (verbose) puts("STUB: SLSWindowQueryResultCopySpaces called");
    return NULL;
}

void* SLSWindowQueryResultCopyWindows(void)
{
    if (verbose) puts("STUB: SLSWindowQueryResultCopyWindows called");
    return NULL;
}

void* SLSWindowQueryResultGetManagedDisplayCount(void)
{
    if (verbose) puts("STUB: SLSWindowQueryResultGetManagedDisplayCount called");
    return NULL;
}

void* SLSWindowQueryResultGetSpaceCount(void)
{
    if (verbose) puts("STUB: SLSWindowQueryResultGetSpaceCount called");
    return NULL;
}

void* SLSWindowQueryResultGetTypeID(void)
{
    if (verbose) puts("STUB: SLSWindowQueryResultGetTypeID called");
    return NULL;
}

void* SLSWindowQueryResultGetWindowCount(void)
{
    if (verbose) puts("STUB: SLSWindowQueryResultGetWindowCount called");
    return NULL;
}

void* SLSWindowQueryRun(void)
{
    if (verbose) puts("STUB: SLSWindowQueryRun called");
    return NULL;
}

void* SLSWindowQuerySetValue(void)
{
    if (verbose) puts("STUB: SLSWindowQuerySetValue called");
    return NULL;
}

void* SLSWindowQueryWindows(void)
{
    if (verbose) puts("STUB: SLSWindowQueryWindows called");
    return NULL;
}

void* SLSWindowRightForKey(void)
{
    if (verbose) puts("STUB: SLSWindowRightForKey called");
    return NULL;
}

void* SLSWindowRightKeyForMask(void)
{
    if (verbose) puts("STUB: SLSWindowRightKeyForMask called");
    return NULL;
}

void* SLSWindowRightsSetGrantCompletedBlockForToken(void)
{
    if (verbose) puts("STUB: SLSWindowRightsSetGrantCompletedBlockForToken called");
    return NULL;
}

void* SLSWindowRightsSetGrantOfferedBlockForToken(void)
{
    if (verbose) puts("STUB: SLSWindowRightsSetGrantOfferedBlockForToken called");
    return NULL;
}

void* SLSWindowScrollRegion(void)
{
    if (verbose) puts("STUB: SLSWindowScrollRegion called");
    return NULL;
}

void* SLSWindowSetActiveShadowLegacy(void)
{
    if (verbose) puts("STUB: SLSWindowSetActiveShadowLegacy called");
    return NULL;
}

void* SLSWindowSetBackdropBackgroundBleed(void)
{
    if (verbose) puts("STUB: SLSWindowSetBackdropBackgroundBleed called");
    return NULL;
}

void* SLSWindowSetKeepsExcessBackingAllocation(void)
{
    if (verbose) puts("STUB: SLSWindowSetKeepsExcessBackingAllocation called");
    return NULL;
}

void* SLSWindowSetResizeRegionShape(void)
{
    if (verbose) puts("STUB: SLSWindowSetResizeRegionShape called");
    return NULL;
}

void* SLSWindowSetShadowProperties(void)
{
    if (verbose) puts("STUB: SLSWindowSetShadowProperties called");
    return NULL;
}

void* SLSWindowSetTrustedForObscuring(void)
{
    if (verbose) puts("STUB: SLSWindowSetTrustedForObscuring called");
    return NULL;
}

void* SLSWindowThaw(void)
{
    if (verbose) puts("STUB: SLSWindowThaw called");
    return NULL;
}

void* SLSWindowThawBackdrops(void)
{
    if (verbose) puts("STUB: SLSWindowThawBackdrops called");
    return NULL;
}

void* SLSWindowVisibleRegionSeed(void)
{
    if (verbose) puts("STUB: SLSWindowVisibleRegionSeed called");
    return NULL;
}

void* SLSZoomPoint(void)
{
    if (verbose) puts("STUB: SLSZoomPoint called");
    return NULL;
}

void* SLScreenRegisterMoveCallback(void)
{
    if (verbose) puts("STUB: SLScreenRegisterMoveCallback called");
    return NULL;
}

void* SLScreenUnregisterMoveCallback(void)
{
    if (verbose) puts("STUB: SLScreenUnregisterMoveCallback called");
    return NULL;
}

void* SLSecureCursorAssertionCreate(void)
{
    if (verbose) puts("STUB: SLSecureCursorAssertionCreate called");
    return NULL;
}

void* SLSecureCursorAssertionInvalidate(void)
{
    if (verbose) puts("STUB: SLSecureCursorAssertionInvalidate called");
    return NULL;
}

void* SLSecureCursorAssertionIsValid(void)
{
    if (verbose) puts("STUB: SLSecureCursorAssertionIsValid called");
    return NULL;
}

void* SLSessionCopyCurrentDictionary(void)
{
    if (verbose) puts("STUB: SLSessionCopyCurrentDictionary called");
    return NULL;
}

void* SLSessionOwnerCopyXPCSerializationAndInvalidate(void)
{
    if (verbose) puts("STUB: SLSessionOwnerCopyXPCSerializationAndInvalidate called");
    return NULL;
}

void* SLSessionOwnerCreateBySettingLoginwindowConnection(void)
{
    if (verbose) puts("STUB: SLSessionOwnerCreateBySettingLoginwindowConnection called");
    return NULL;
}

void* SLSessionOwnerCreateNewSessionWithAuditSessionID(void)
{
    if (verbose) puts("STUB: SLSessionOwnerCreateNewSessionWithAuditSessionID called");
    return NULL;
}

void* SLSessionOwnerCreateWithPort(void)
{
    if (verbose) puts("STUB: SLSessionOwnerCreateWithPort called");
    return NULL;
}

void* SLSessionOwnerCreateWithXPCSerialization(void)
{
    if (verbose) puts("STUB: SLSessionOwnerCreateWithXPCSerialization called");
    return NULL;
}

void* SLSessionOwnerGetAuditSessionID(void)
{
    if (verbose) puts("STUB: SLSessionOwnerGetAuditSessionID called");
    return NULL;
}

void* SLSessionOwnerGetSessionID(void)
{
    if (verbose) puts("STUB: SLSessionOwnerGetSessionID called");
    return NULL;
}

void* SLSessionOwnerIsValid(void)
{
    if (verbose) puts("STUB: SLSessionOwnerIsValid called");
    return NULL;
}

void* SLSetDisplayTransferByByteTable(void)
{
    if (verbose) puts("STUB: SLSetDisplayTransferByByteTable called");
    return NULL;
}

void* SLSetDisplayTransferByFormula(void)
{
    if (verbose) puts("STUB: SLSetDisplayTransferByFormula called");
    return NULL;
}

void* SLSetDisplayTransferByTable(void)
{
    if (verbose) puts("STUB: SLSetDisplayTransferByTable called");
    return NULL;
}

void* SLSetLocalEventsFilterDuringSuppressionState(void)
{
    if (verbose) puts("STUB: SLSetLocalEventsFilterDuringSuppressionState called");
    return NULL;
}

void* SLSetLocalEventsFilterDuringSupressionState(void)
{
    if (verbose) puts("STUB: SLSetLocalEventsFilterDuringSupressionState called");
    return NULL;
}

void* SLSetLocalEventsSuppressionInterval(void)
{
    if (verbose) puts("STUB: SLSetLocalEventsSuppressionInterval called");
    return NULL;
}

void* SLShieldingWindowID(void)
{
    if (verbose) puts("STUB: SLShieldingWindowID called");
    return NULL;
}

void* SLShieldingWindowLevel(void)
{
    if (verbose) puts("STUB: SLShieldingWindowLevel called");
    return NULL;
}

void* SLUnregisterScreenRefreshCallback(void)
{
    if (verbose) puts("STUB: SLUnregisterScreenRefreshCallback called");
    return NULL;
}

void* SLWaitForScreenRefreshRects(void)
{
    if (verbose) puts("STUB: SLWaitForScreenRefreshRects called");
    return NULL;
}

void* SLWaitForScreenUpdateRects(void)
{
    if (verbose) puts("STUB: SLWaitForScreenUpdateRects called");
    return NULL;
}

void* SLWarpMouseCursorPosition(void)
{
    if (verbose) puts("STUB: SLWarpMouseCursorPosition called");
    return NULL;
}

void* SLWindowContextCreate(void)
{
    if (verbose) puts("STUB: SLWindowContextCreate called");
    return NULL;
}

void* SLWindowContextCreateImage(void)
{
    if (verbose) puts("STUB: SLWindowContextCreateImage called");
    return NULL;
}

void* SLWindowContextGetConnection(void)
{
    if (verbose) puts("STUB: SLWindowContextGetConnection called");
    return NULL;
}

void* SLWindowContextGetWindow(void)
{
    if (verbose) puts("STUB: SLWindowContextGetWindow called");
    return NULL;
}

void* SLWindowLevelForKey(void)
{
    if (verbose) puts("STUB: SLWindowLevelForKey called");
    return NULL;
}

void* SLWindowListCopyWindowInfo(void)
{
    if (verbose) puts("STUB: SLWindowListCopyWindowInfo called");
    return NULL;
}

void* SLWindowListCreate(void)
{
    if (verbose) puts("STUB: SLWindowListCreate called");
    return NULL;
}

void* SLWindowListCreateArray(void)
{
    if (verbose) puts("STUB: SLWindowListCreateArray called");
    return NULL;
}

void* SLWindowListCreateDescriptionFromArray(void)
{
    if (verbose) puts("STUB: SLWindowListCreateDescriptionFromArray called");
    return NULL;
}

void* SLWindowListCreateImage(void)
{
    if (verbose) puts("STUB: SLWindowListCreateImage called");
    return NULL;
}

void* SLWindowListCreateImageFromArray(void)
{
    if (verbose) puts("STUB: SLWindowListCreateImageFromArray called");
    return NULL;
}

void* SLWindowServerCFMachPort(void)
{
    if (verbose) puts("STUB: SLWindowServerCFMachPort called");
    return NULL;
}

void* SLWindowServerCreateServerPort(void)
{
    if (verbose) puts("STUB: SLWindowServerCreateServerPort called");
    return NULL;
}

void* SLXServer(void)
{
    if (verbose) puts("STUB: SLXServer called");
    return NULL;
}

void* _SLDisplayCapturedByPsn(void)
{
    if (verbose) puts("STUB: _SLDisplayCapturedByPsn called");
    return NULL;
}

void* _SLPSCreateStubApplicationWithConnection(void)
{
    if (verbose) puts("STUB: _SLPSCreateStubApplicationWithConnection called");
    return NULL;
}

void* _SLPSEqualProcess(void)
{
    if (verbose) puts("STUB: _SLPSEqualProcess called");
    return NULL;
}

void* _SLPSGetCurrentProcess(void)
{
    if (verbose) puts("STUB: _SLPSGetCurrentProcess called");
    return NULL;
}

void* _SLPSGetFrontProcess(void)
{
    if (verbose) puts("STUB: _SLPSGetFrontProcess called");
    return NULL;
}

void* _SLPSRegisterWithServer(void)
{
    if (verbose) puts("STUB: _SLPSRegisterWithServer called");
    return NULL;
}

void* _SLPSSetFrontProcessWithOptions(void)
{
    if (verbose) puts("STUB: _SLPSSetFrontProcessWithOptions called");
    return NULL;
}

void* _SLSDefaultConnection(void)
{
    if (verbose) puts("STUB: _SLSDefaultConnection called");
    return NULL;
}

void* slglsAttachDrawable(void)
{
    if (verbose) puts("STUB: slglsAttachDrawable called");
    return NULL;
}

void* slglsAttachPBuffer(void)
{
    if (verbose) puts("STUB: slglsAttachPBuffer called");
    return NULL;
}

void* slglsAttachState(void)
{
    if (verbose) puts("STUB: slglsAttachState called");
    return NULL;
}

void* slglsCreateContext(void)
{
    if (verbose) puts("STUB: slglsCreateContext called");
    return NULL;
}

void* slglsCreatePBuffer(void)
{
    if (verbose) puts("STUB: slglsCreatePBuffer called");
    return NULL;
}

void* slglsDestroyContext(void)
{
    if (verbose) puts("STUB: slglsDestroyContext called");
    return NULL;
}

void* slglsDestroyPBuffer(void)
{
    if (verbose) puts("STUB: slglsDestroyPBuffer called");
    return NULL;
}

void* slglsDestroyRendererInfo(void)
{
    if (verbose) puts("STUB: slglsDestroyRendererInfo called");
    return NULL;
}

void* slglsGetInteger(void)
{
    if (verbose) puts("STUB: slglsGetInteger called");
    return NULL;
}

void* slglsGetPixelFormat(void)
{
    if (verbose) puts("STUB: slglsGetPixelFormat called");
    return NULL;
}

void* slglsGetState(void)
{
    if (verbose) puts("STUB: slglsGetState called");
    return NULL;
}

void* slglsGetSurface(void)
{
    if (verbose) puts("STUB: slglsGetSurface called");
    return NULL;
}

void* slglsGetVirtualScreen(void)
{
    if (verbose) puts("STUB: slglsGetVirtualScreen called");
    return NULL;
}

void* slglsQueryRendererInfo(void)
{
    if (verbose) puts("STUB: slglsQueryRendererInfo called");
    return NULL;
}

void* slglsSetInteger(void)
{
    if (verbose) puts("STUB: slglsSetInteger called");
    return NULL;
}

void* slglsTexImageIOSurface2D(void)
{
    if (verbose) puts("STUB: slglsTexImageIOSurface2D called");
    return NULL;
}

void* slglsTexImagePBuffer(void)
{
    if (verbose) puts("STUB: slglsTexImagePBuffer called");
    return NULL;
}
