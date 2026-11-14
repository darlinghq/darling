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

extern CFStringRef const kVTCompressionPropertyKey_MaxKeyFrameIntervalDuration;

extern CFStringRef const kVTCompressionPropertyKey_DataRateLimits;

extern CFStringRef const kVTProfileLevel_H264_High_AutoLevel;

extern CFStringRef const kVTCompressionPropertyKey_MaxFrameDelayCount;

extern CFStringRef const kVTCompressionPropertyKey_ExpectedFrameRate;

extern CFStringRef const kVTVideoEncoderSpecification_RequireHardwareAcceleratedVideoEncoder;

extern CFStringRef const kVTCompressionPropertyKey_ColorPrimaries;
extern CFStringRef const kVTCompressionPropertyKey_TransferFunction;
extern CFStringRef const kVTCompressionPropertyKey_YCbCrMatrix;
extern CFStringRef const kVTCompressionPropertyKey_BaseLayerFrameRateFraction;
extern CFStringRef const kVTVideoEncoderSpecification_EnableLowLatencyRateControl;

extern CFStringRef const kVTCompressionPropertyKey_PixelAspectRatio;
extern CFStringRef const kVTCompressionPropertyKey_Quality;
extern CFStringRef const kVTCompressionPropertyKey_MoreFramesAfterEnd;
extern CFStringRef const kVTCompressionPropertyKey_MoreFramesBeforeStart;
extern CFStringRef const kVTCompressionPropertyKey_MaxH264SliceBytes;

extern CFStringRef const kVTProfileLevel_H264_Baseline_AutoLevel;
extern CFStringRef const kVTProfileLevel_H264_Main_AutoLevel;
extern CFStringRef const kVTProfileLevel_H264_Baseline_1_3;
extern CFStringRef const kVTProfileLevel_H264_Baseline_3_0;
extern CFStringRef const kVTProfileLevel_H264_Baseline_3_1;
extern CFStringRef const kVTProfileLevel_H264_Baseline_3_2;
extern CFStringRef const kVTProfileLevel_H264_Baseline_4_0;
extern CFStringRef const kVTProfileLevel_H264_Baseline_4_1;
extern CFStringRef const kVTProfileLevel_H264_Baseline_4_2;
extern CFStringRef const kVTProfileLevel_H264_Baseline_5_0;
extern CFStringRef const kVTProfileLevel_H264_Baseline_5_1;
extern CFStringRef const kVTProfileLevel_H264_Baseline_5_2;
extern CFStringRef const kVTProfileLevel_H264_Extended_5_0;
extern CFStringRef const kVTProfileLevel_H264_High_3_0;
extern CFStringRef const kVTProfileLevel_H264_High_3_1;
extern CFStringRef const kVTProfileLevel_H264_High_3_2;
extern CFStringRef const kVTProfileLevel_H264_High_4_0;
extern CFStringRef const kVTProfileLevel_H264_High_4_1;
extern CFStringRef const kVTProfileLevel_H264_High_4_2;
extern CFStringRef const kVTProfileLevel_H264_High_5_0;
extern CFStringRef const kVTProfileLevel_H264_High_5_1;
extern CFStringRef const kVTProfileLevel_H264_High_5_2;
extern CFStringRef const kVTProfileLevel_H264_Main_3_0;
extern CFStringRef const kVTProfileLevel_H264_Main_3_1;
extern CFStringRef const kVTProfileLevel_H264_Main_3_2;
extern CFStringRef const kVTProfileLevel_H264_Main_4_0;
extern CFStringRef const kVTProfileLevel_H264_Main_4_1;
extern CFStringRef const kVTProfileLevel_H264_Main_4_2;
extern CFStringRef const kVTProfileLevel_H264_Main_5_0;
extern CFStringRef const kVTProfileLevel_H264_Main_5_1;
extern CFStringRef const kVTProfileLevel_H264_Main_5_2;

extern CFStringRef const kVTEncodeFrameOptionKey_ForceKeyFrame;
extern CFStringRef const kVTEncodeFrameOptionKey_BaseFrameQP;

extern CFStringRef const kVTCompressionPropertyKey_EncoderID;
extern CFStringRef const kVTProfileLevel_HEVC_Main_AutoLevel;

#endif
