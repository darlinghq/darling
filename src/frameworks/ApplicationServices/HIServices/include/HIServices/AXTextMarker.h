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

#ifndef _HISERVICES_AXTEXTMARKER_H_
#define _HISERVICES_AXTEXTMARKER_H_

// Private HIServices API that WebKit uses (as declared by its HIServicesSPI.h).
// Not included by the umbrella headers.

#include <CoreFoundation/CoreFoundation.h>
#include <HIServices/AXError.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// A text marker is an opaque run of bytes an accessible application hands
// out for a position in its text; a range is a pair of them.
typedef const struct __AXTextMarker *AXTextMarkerRef;
typedef const struct __AXTextMarkerRange *AXTextMarkerRangeRef;

CFTypeID AXTextMarkerGetTypeID(void);
AXTextMarkerRef AXTextMarkerCreate(CFAllocatorRef allocator, const UInt8 *bytes, CFIndex length);
CFIndex AXTextMarkerGetLength(AXTextMarkerRef marker);
const UInt8 *AXTextMarkerGetBytePtr(AXTextMarkerRef marker);

CFTypeID AXTextMarkerRangeGetTypeID(void);
AXTextMarkerRangeRef AXTextMarkerRangeCreate(CFAllocatorRef allocator, AXTextMarkerRef startMarker, AXTextMarkerRef endMarker);
AXTextMarkerRef AXTextMarkerRangeCopyStartMarker(AXTextMarkerRangeRef range);
AXTextMarkerRef AXTextMarkerRangeCopyEndMarker(AXTextMarkerRangeRef range);

// The kind of assistive application whose request is being served.
typedef CF_ENUM(int32_t, AXClientType) {
	kAXClientTypeNoActiveRequestFound = 0,
	kAXClientTypeUnknown,
	kAXClientTypeRaft,
	kAXClientTypeXCUITest,
	kAXClientTypeXCTest,
	kAXClientTypeScripter2,
	kAXClientTypeSystemEvents,
	kAXClientTypeVoiceOver,
	kAXClientTypeAssistiveControl,
	kAXClientTypeFullKeyboardAccess,
	kAXClientTypeDictation,
};

AXClientType _AXGetClientForCurrentRequestUntrusted(void);
void _AXSetClientIdentificationOverride(AXClientType client);
bool _AXUIElementRequestServicedBySecondaryAXThread(void);
AXError _AXUIElementUseSecondaryAXThread(bool enabled);

#ifdef __cplusplus
}
#endif

#endif
