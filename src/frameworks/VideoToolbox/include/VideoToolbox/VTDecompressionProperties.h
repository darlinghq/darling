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

#ifndef _VTDECOMPRESSIONPROPERTIES_H_
#define _VTDECOMPRESSIONPROPERTIES_H_

#include <CoreFoundation/CoreFoundation.h>
#include <VideoToolbox/VTBase.h>

VT_EXPORT CFStringRef const kVTVideoDecoderSpecification_RequireHardwareAcceleratedVideoDecoder;
VT_EXPORT CFStringRef const kVTDecompressionPropertyKey_RealTime;
VT_EXPORT CFStringRef const kVTDecompressionPropertyKey_DeinterlaceMode;
VT_EXPORT CFStringRef const kVTDecompressionPropertyKey_FieldMode;
VT_EXPORT CFStringRef const kVTDecompressionProperty_DeinterlaceMode_Temporal;
VT_EXPORT CFStringRef const kVTDecompressionProperty_FieldMode_DeinterlaceFields;
VT_EXPORT CFStringRef const kVTVideoDecoderSpecification_EnableHardwareAcceleratedVideoDecoder;
VT_EXPORT CFStringRef const kVTDecompressionPropertyKey_UsingHardwareAcceleratedVideoDecoder;
VT_EXPORT CFStringRef const kVTDecompressionPropertyKey_ThreadCount;
VT_EXPORT CFStringRef const kVTDecompressionPropertyKey_NumberOfFramesBeingDecoded;
VT_EXPORT CFStringRef const kVTDecompressionPropertyKey_MinOutputPresentationTimeStampOfFramesBeingDecoded;
VT_EXPORT CFStringRef const kVTDecompressionPropertyKey_MaxOutputPresentationTimeStampOfFramesBeingDecoded;
VT_EXPORT CFStringRef const kVTDecompressionPropertyKey_PixelBufferPool;
VT_EXPORT CFStringRef const kVTDecompressionPropertyKey_OutputPoolRequestedMinimumBufferCount;
VT_EXPORT CFStringRef const kVTDecompressionPropertyKey_ContentHasInterframeDependencies;

#endif
