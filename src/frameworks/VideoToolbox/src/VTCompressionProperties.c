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

#include <VideoToolbox/VTCompressionProperties.h>

const CFStringRef kVTCompressionPropertyKey_MaxKeyFrameIntervalDuration = CFSTR("MaxKeyFrameIntervalDuration");

const CFStringRef kVTCompressionPropertyKey_DataRateLimits = CFSTR("DataRateLimits");

const CFStringRef kVTProfileLevel_H264_High_AutoLevel = CFSTR("H264_High_AutoLevel");

const CFStringRef kVTCompressionPropertyKey_MaxFrameDelayCount = CFSTR("MaxFrameDelayCount");

const CFStringRef kVTCompressionPropertyKey_ExpectedFrameRate = CFSTR("ExpectedFrameRate");

const CFStringRef kVTVideoEncoderSpecification_RequireHardwareAcceleratedVideoEncoder = CFSTR("RequireHardwareAcceleratedVideoEncoder");

// Values observed on macOS by printing the public constants.
const CFStringRef kVTCompressionPropertyKey_ColorPrimaries = CFSTR("ColorPrimaries");
const CFStringRef kVTCompressionPropertyKey_TransferFunction = CFSTR("TransferFunction");
const CFStringRef kVTCompressionPropertyKey_YCbCrMatrix = CFSTR("YCbCrMatrix");
const CFStringRef kVTCompressionPropertyKey_BaseLayerFrameRateFraction = CFSTR("BaseLayerFrameRateFraction");
const CFStringRef kVTVideoEncoderSpecification_EnableLowLatencyRateControl = CFSTR("EnableLowLatencyRateControl");

const CFStringRef kVTProfileLevel_H264_Baseline_1_3 = CFSTR("H264_Baseline_1_3");
const CFStringRef kVTProfileLevel_H264_Baseline_3_0 = CFSTR("H264_Baseline_3_0");
const CFStringRef kVTProfileLevel_H264_Baseline_3_1 = CFSTR("H264_Baseline_3_1");
const CFStringRef kVTProfileLevel_H264_Baseline_3_2 = CFSTR("H264_Baseline_3_2");
const CFStringRef kVTProfileLevel_H264_Baseline_4_0 = CFSTR("H264_Baseline_4_0");
const CFStringRef kVTProfileLevel_H264_Baseline_4_1 = CFSTR("H264_Baseline_4_1");
const CFStringRef kVTProfileLevel_H264_Baseline_4_2 = CFSTR("H264_Baseline_4_2");
const CFStringRef kVTProfileLevel_H264_Baseline_5_0 = CFSTR("H264_Baseline_5_0");
const CFStringRef kVTProfileLevel_H264_Baseline_5_1 = CFSTR("H264_Baseline_5_1");
const CFStringRef kVTProfileLevel_H264_Baseline_5_2 = CFSTR("H264_Baseline_5_2");
const CFStringRef kVTProfileLevel_H264_Extended_5_0 = CFSTR("H264_Extended_5_0");
const CFStringRef kVTProfileLevel_H264_High_3_0 = CFSTR("H264_High_3_0");
const CFStringRef kVTProfileLevel_H264_High_3_1 = CFSTR("H264_High_3_1");
const CFStringRef kVTProfileLevel_H264_High_3_2 = CFSTR("H264_High_3_2");
const CFStringRef kVTProfileLevel_H264_High_4_0 = CFSTR("H264_High_4_0");
const CFStringRef kVTProfileLevel_H264_High_4_1 = CFSTR("H264_High_4_1");
const CFStringRef kVTProfileLevel_H264_High_4_2 = CFSTR("H264_High_4_2");
const CFStringRef kVTProfileLevel_H264_High_5_0 = CFSTR("H264_High_5_0");
const CFStringRef kVTProfileLevel_H264_High_5_1 = CFSTR("H264_High_5_1");
const CFStringRef kVTProfileLevel_H264_High_5_2 = CFSTR("H264_High_5_2");
const CFStringRef kVTProfileLevel_H264_Main_3_0 = CFSTR("H264_Main_3_0");
const CFStringRef kVTProfileLevel_H264_Main_3_1 = CFSTR("H264_Main_3_1");
const CFStringRef kVTProfileLevel_H264_Main_3_2 = CFSTR("H264_Main_3_2");
const CFStringRef kVTProfileLevel_H264_Main_4_0 = CFSTR("H264_Main_4_0");
const CFStringRef kVTProfileLevel_H264_Main_4_1 = CFSTR("H264_Main_4_1");
const CFStringRef kVTProfileLevel_H264_Main_4_2 = CFSTR("H264_Main_4_2");
const CFStringRef kVTProfileLevel_H264_Main_5_0 = CFSTR("H264_Main_5_0");
const CFStringRef kVTProfileLevel_H264_Main_5_1 = CFSTR("H264_Main_5_1");
const CFStringRef kVTProfileLevel_H264_Main_5_2 = CFSTR("H264_Main_5_2");

const CFStringRef kVTCompressionPropertyKey_PixelAspectRatio = CFSTR("PixelAspectRatio");
const CFStringRef kVTCompressionPropertyKey_Quality = CFSTR("Quality");
const CFStringRef kVTCompressionPropertyKey_MoreFramesAfterEnd = CFSTR("MoreFramesAfterEnd");
const CFStringRef kVTCompressionPropertyKey_MoreFramesBeforeStart = CFSTR("MoreFramesBeforeStart");
const CFStringRef kVTCompressionPropertyKey_MaxH264SliceBytes = CFSTR("MaxH264SliceBytes");
