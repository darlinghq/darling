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

#include <VideoToolbox/VTPixelTransferProperties.h>

// kVTPixelTransferPropertyKey_RealTime is defined with the other legacy
// constants in VideoToolbox.m.
const CFStringRef kVTPixelTransferPropertyKey_ScalingMode = CFSTR("ScalingMode");
const CFStringRef kVTScalingMode_Normal = CFSTR("Normal");
const CFStringRef kVTScalingMode_CropSourceToCleanAperture = CFSTR("CropSourceToCleanAperture");
const CFStringRef kVTScalingMode_Letterbox = CFSTR("Letterbox");
const CFStringRef kVTScalingMode_Trim = CFSTR("Trim");
const CFStringRef kVTPixelTransferPropertyKey_DestinationCleanAperture = CFSTR("DestinationCleanAperture");
const CFStringRef kVTPixelTransferPropertyKey_DestinationPixelAspectRatio = CFSTR("DestinationPixelAspectRatio");
const CFStringRef kVTPixelTransferPropertyKey_DownsamplingMode = CFSTR("DownsamplingMode");
const CFStringRef kVTDownsamplingMode_Decimate = CFSTR("Decimate");
const CFStringRef kVTDownsamplingMode_Average = CFSTR("Average");
const CFStringRef kVTPixelTransferPropertyKey_DestinationColorPrimaries = CFSTR("DestinationColorPrimaries");
const CFStringRef kVTPixelTransferPropertyKey_DestinationTransferFunction = CFSTR("DestinationTransferFunction");
const CFStringRef kVTPixelTransferPropertyKey_DestinationICCProfile = CFSTR("DestinationICCProfile");
const CFStringRef kVTPixelTransferPropertyKey_DestinationYCbCrMatrix = CFSTR("DestinationYCbCrMatrix");
