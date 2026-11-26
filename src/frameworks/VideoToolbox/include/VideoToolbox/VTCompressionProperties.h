/*
 This file is part of Darling.

 Copyright (C) 2025 Darling Developers

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

#ifndef _VTCOMPRESSIONPROPERTIES_H_
#define _VTCOMPRESSIONPROPERTIES_H_

#include <CoreFoundation/CoreFoundation.h>
#include <VideoToolbox/VTBase.h>

VT_EXPORT CFStringRef const kVTCompressionPropertyKey_MaxKeyFrameIntervalDuration;
VT_EXPORT CFStringRef const kVTCompressionPropertyKey_AllowFrameReordering;
VT_EXPORT CFStringRef const kVTCompressionPropertyKey_AverageBitRate;
VT_EXPORT CFStringRef const kVTCompressionPropertyKey_MaxKeyFrameInterval;
VT_EXPORT CFStringRef const kVTCompressionPropertyKey_ProfileLevel;
VT_EXPORT CFStringRef const kVTCompressionPropertyKey_RealTime;
VT_EXPORT CFStringRef const kVTCompressionPropertyKey_UsingHardwareAcceleratedVideoEncoder;
VT_EXPORT CFStringRef const kVTEncodeFrameOptionKey_ForceKeyFrame;
VT_EXPORT CFStringRef const kVTProfileLevel_H264_Baseline_AutoLevel;
VT_EXPORT CFStringRef const kVTProfileLevel_H264_Main_AutoLevel;
VT_EXPORT CFStringRef const kVTVideoEncoderSpecification_EnableHardwareAcceleratedVideoEncoder;

VT_EXPORT CFStringRef const kVTCompressionPropertyKey_DataRateLimits;

VT_EXPORT CFStringRef const kVTProfileLevel_H264_High_AutoLevel;

VT_EXPORT CFStringRef const kVTCompressionPropertyKey_MaxFrameDelayCount;

VT_EXPORT CFStringRef const kVTCompressionPropertyKey_ExpectedFrameRate;

VT_EXPORT CFStringRef const kVTVideoEncoderSpecification_RequireHardwareAcceleratedVideoEncoder;

VT_EXPORT CFStringRef const kVTCompressionPropertyKey_ColorPrimaries;
VT_EXPORT CFStringRef const kVTCompressionPropertyKey_TransferFunction;
VT_EXPORT CFStringRef const kVTCompressionPropertyKey_YCbCrMatrix;
VT_EXPORT CFStringRef const kVTCompressionPropertyKey_BaseLayerFrameRateFraction;
VT_EXPORT CFStringRef const kVTVideoEncoderSpecification_EnableLowLatencyRateControl;

VT_EXPORT CFStringRef const kVTCompressionPropertyKey_PixelAspectRatio;
VT_EXPORT CFStringRef const kVTCompressionPropertyKey_Quality;
VT_EXPORT CFStringRef const kVTCompressionPropertyKey_MoreFramesAfterEnd;
VT_EXPORT CFStringRef const kVTCompressionPropertyKey_MoreFramesBeforeStart;
VT_EXPORT CFStringRef const kVTCompressionPropertyKey_MaxH264SliceBytes;

VT_EXPORT CFStringRef const kVTProfileLevel_H264_Baseline_AutoLevel;
VT_EXPORT CFStringRef const kVTProfileLevel_H264_Main_AutoLevel;
VT_EXPORT CFStringRef const kVTProfileLevel_H264_Baseline_1_3;
VT_EXPORT CFStringRef const kVTProfileLevel_H264_Baseline_3_0;
VT_EXPORT CFStringRef const kVTProfileLevel_H264_Baseline_3_1;
VT_EXPORT CFStringRef const kVTProfileLevel_H264_Baseline_3_2;
VT_EXPORT CFStringRef const kVTProfileLevel_H264_Baseline_4_0;
VT_EXPORT CFStringRef const kVTProfileLevel_H264_Baseline_4_1;
VT_EXPORT CFStringRef const kVTProfileLevel_H264_Baseline_4_2;
VT_EXPORT CFStringRef const kVTProfileLevel_H264_Baseline_5_0;
VT_EXPORT CFStringRef const kVTProfileLevel_H264_Baseline_5_1;
VT_EXPORT CFStringRef const kVTProfileLevel_H264_Baseline_5_2;
VT_EXPORT CFStringRef const kVTProfileLevel_H264_Extended_5_0;
VT_EXPORT CFStringRef const kVTProfileLevel_H264_High_3_0;
VT_EXPORT CFStringRef const kVTProfileLevel_H264_High_3_1;
VT_EXPORT CFStringRef const kVTProfileLevel_H264_High_3_2;
VT_EXPORT CFStringRef const kVTProfileLevel_H264_High_4_0;
VT_EXPORT CFStringRef const kVTProfileLevel_H264_High_4_1;
VT_EXPORT CFStringRef const kVTProfileLevel_H264_High_4_2;
VT_EXPORT CFStringRef const kVTProfileLevel_H264_High_5_0;
VT_EXPORT CFStringRef const kVTProfileLevel_H264_High_5_1;
VT_EXPORT CFStringRef const kVTProfileLevel_H264_High_5_2;
VT_EXPORT CFStringRef const kVTProfileLevel_H264_Main_3_0;
VT_EXPORT CFStringRef const kVTProfileLevel_H264_Main_3_1;
VT_EXPORT CFStringRef const kVTProfileLevel_H264_Main_3_2;
VT_EXPORT CFStringRef const kVTProfileLevel_H264_Main_4_0;
VT_EXPORT CFStringRef const kVTProfileLevel_H264_Main_4_1;
VT_EXPORT CFStringRef const kVTProfileLevel_H264_Main_4_2;
VT_EXPORT CFStringRef const kVTProfileLevel_H264_Main_5_0;
VT_EXPORT CFStringRef const kVTProfileLevel_H264_Main_5_1;
VT_EXPORT CFStringRef const kVTProfileLevel_H264_Main_5_2;

VT_EXPORT CFStringRef const kVTEncodeFrameOptionKey_ForceKeyFrame;
VT_EXPORT CFStringRef const kVTEncodeFrameOptionKey_BaseFrameQP;

VT_EXPORT CFStringRef const kVTCompressionPropertyKey_EncoderID;
VT_EXPORT CFStringRef const kVTProfileLevel_HEVC_Main_AutoLevel;
VT_EXPORT CFStringRef const kVTProfileLevel_HEVC_Main10_AutoLevel;

// CFData, 24 bytes (the layout of the mastering display colour volume format description extension).
VT_EXPORT CFStringRef const kVTCompressionPropertyKey_MasteringDisplayColorVolume;

#endif
