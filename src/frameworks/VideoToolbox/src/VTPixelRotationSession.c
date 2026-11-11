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

#include <VideoToolbox/VTPixelRotationSession.h>
#include <VideoToolbox/VTErrors.h>

const CFStringRef kVTPixelRotationPropertyKey_Rotation = CFSTR("Rotation");
const CFStringRef kVTRotation_0 = CFSTR("Rotation0");
const CFStringRef kVTRotation_CW90 = CFSTR("RotationCW90");
const CFStringRef kVTRotation_180 = CFSTR("Rotation180");
const CFStringRef kVTRotation_CCW90 = CFSTR("RotationCCW90");
const CFStringRef kVTPixelRotationPropertyKey_FlipHorizontalOrientation = CFSTR("FlipHorizontalOrientation");
const CFStringRef kVTPixelRotationPropertyKey_FlipVerticalOrientation = CFSTR("FlipVerticalOrientation");

// Rotating pixel buffers is not implemented, so no session can be created.
// Creation reports that through the documented error code and leaves the
// output reference NULL, so callers can fall back to rotating the image
// themselves.
OSStatus VTPixelRotationSessionCreate(CFAllocatorRef allocator,
	VTPixelRotationSessionRef *pixelRotationSessionOut)
{
	(void) allocator;
	if (pixelRotationSessionOut == NULL)
		return kVTParameterErr;
	*pixelRotationSessionOut = NULL;
	return kVTCouldNotCreateInstanceErr;
}

void VTPixelRotationSessionInvalidate(VTPixelRotationSessionRef session)
{
	(void) session;
}

CFTypeID VTPixelRotationSessionGetTypeID(void)
{
	return 0; // no pixel rotation session type is registered
}

OSStatus VTPixelRotationSessionRotateImage(VTPixelRotationSessionRef session,
	CVPixelBufferRef sourceBuffer, CVPixelBufferRef destinationBuffer)
{
	(void) sourceBuffer;
	(void) destinationBuffer;
	if (session == NULL)
		return kVTInvalidSessionErr;
	return kVTPixelRotationNotSupportedErr;
}
