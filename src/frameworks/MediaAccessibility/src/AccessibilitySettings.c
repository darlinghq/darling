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


// Audible media, video accommodation and image caption entry points of MediaAccessibility.
// No system settings UI exists, so these report the defaults observed on macOS 26.7.3.

#include <MediaAccessibility/MediaAccessibility.h>
#include <CoreFoundation/CoreFoundation.h>

const CFStringRef kMAAudibleMediaSettingsChangedNotification = CFSTR("com.apple.mediaaccessibility.audibleMediaSettingsChanged");
CFStringRef const MAMediaCharacteristicDescribesVideoForAccessibility = CFSTR("public.accessibility.describes-video");

const CFStringRef kMADimFlashingLightsChangedNotification = CFSTR("com.apple.accessibility.dim.flashing.lights.mitigation");

CFArrayRef MAAudibleMediaCopyPreferredCharacteristics(void)
{
	return CFArrayCreate(NULL, NULL, 0, &kCFTypeArrayCallBacks);
}

bool MADimFlashingLightsEnabled(void)
{
	return false;
}

CFStringRef MAImageCaptioningCopyMetadataTagPath(void)
{
	return CFSTR("Iptc4xmpExt:ArtworkContentDescription");
}
