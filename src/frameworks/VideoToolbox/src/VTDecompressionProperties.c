/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

#include <VideoToolbox/VTDecompressionProperties.h>

const CFStringRef kVTDecompressionPropertyKey_DeinterlaceMode = CFSTR("DeinterlaceMode");
const CFStringRef kVTDecompressionPropertyKey_FieldMode = CFSTR("FieldMode");
const CFStringRef kVTDecompressionProperty_DeinterlaceMode_Temporal = CFSTR("Temporal");
const CFStringRef kVTDecompressionProperty_FieldMode_DeinterlaceFields = CFSTR("DeinterlaceFields");

const CFStringRef kVTVideoDecoderSpecification_RequireHardwareAcceleratedVideoDecoder = CFSTR("RequireHardwareAcceleratedVideoDecoder");
const CFStringRef kVTVideoDecoderSpecification_EnableHardwareAcceleratedVideoDecoder = CFSTR("EnableHardwareAcceleratedVideoDecoder");
const CFStringRef kVTDecompressionPropertyKey_RealTime = CFSTR("RealTime");
const CFStringRef kVTDecompressionPropertyKey_UsingHardwareAcceleratedVideoDecoder = CFSTR("UsingHardwareAcceleratedVideoDecoder");
const CFStringRef kVTDecompressionPropertyKey_ThreadCount = CFSTR("ThreadCount");
const CFStringRef kVTDecompressionPropertyKey_NumberOfFramesBeingDecoded = CFSTR("NumberOfFramesBeingDecoded");
const CFStringRef kVTDecompressionPropertyKey_MinOutputPresentationTimeStampOfFramesBeingDecoded = CFSTR("MinOutputPresentationTimeStampOfFramesBeingDecoded");
const CFStringRef kVTDecompressionPropertyKey_MaxOutputPresentationTimeStampOfFramesBeingDecoded = CFSTR("MaxOutputPresentationTimeStampOfFramesBeingDecoded");
const CFStringRef kVTDecompressionPropertyKey_PixelBufferPool = CFSTR("PixelBufferPool");
const CFStringRef kVTDecompressionPropertyKey_OutputPoolRequestedMinimumBufferCount = CFSTR("OutputPoolRequestedMinimumBufferCount");
const CFStringRef kVTDecompressionPropertyKey_ContentHasInterframeDependencies = CFSTR("ContentHasInterframeDependencies");

