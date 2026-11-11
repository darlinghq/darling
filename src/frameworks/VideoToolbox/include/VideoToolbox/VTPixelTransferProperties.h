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

#ifndef _VTPIXELTRANSFERPROPERTIES_H_
#define _VTPIXELTRANSFERPROPERTIES_H_

#include <CoreFoundation/CoreFoundation.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const CFStringRef kVTPixelTransferPropertyKey_ScalingMode;
extern const CFStringRef kVTScalingMode_Normal;
extern const CFStringRef kVTScalingMode_CropSourceToCleanAperture;
extern const CFStringRef kVTScalingMode_Letterbox;
extern const CFStringRef kVTScalingMode_Trim;
extern const CFStringRef kVTPixelTransferPropertyKey_DestinationCleanAperture;
extern const CFStringRef kVTPixelTransferPropertyKey_DestinationPixelAspectRatio;
extern const CFStringRef kVTPixelTransferPropertyKey_DownsamplingMode;
extern const CFStringRef kVTDownsamplingMode_Decimate;
extern const CFStringRef kVTDownsamplingMode_Average;
extern const CFStringRef kVTPixelTransferPropertyKey_DestinationColorPrimaries;
extern const CFStringRef kVTPixelTransferPropertyKey_DestinationTransferFunction;
extern const CFStringRef kVTPixelTransferPropertyKey_DestinationICCProfile;
extern const CFStringRef kVTPixelTransferPropertyKey_DestinationYCbCrMatrix;
extern const CFStringRef kVTPixelTransferPropertyKey_RealTime;

#ifdef __cplusplus
}
#endif

#endif
