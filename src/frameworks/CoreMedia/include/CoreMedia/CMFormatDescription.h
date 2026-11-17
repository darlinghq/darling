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

#ifndef _CMFORMATDESCRIPTION_H_
#define _CMFORMATDESCRIPTION_H_

#include <CoreMedia/CMBase.h>
#include <CoreAudio/CoreAudioTypes.h>
#include <CoreVideo/CVImageBuffer.h>

CF_IMPLICIT_BRIDGING_ENABLED

enum {
	kCMFormatDescriptionError_InvalidParameter = -12710,
	kCMFormatDescriptionError_AllocationFailed = -12711,
	kCMFormatDescriptionError_ValueNotAvailable = -12718,
	kCMFormatDescriptionBridgeError_InvalidParameter = -12712,
};

typedef const struct CF_BRIDGED_TYPE(id) opaqueCMFormatDescription *CMFormatDescriptionRef;
typedef CMFormatDescriptionRef CMVideoFormatDescriptionRef;
typedef CMFormatDescriptionRef CMAudioFormatDescriptionRef;

typedef FourCharCode CMMediaType;
typedef FourCharCode CMVideoCodecType;

enum {
	kCMMediaType_Video = 'vide',
	kCMMediaType_Audio = 'soun',
	kCMMediaType_Muxed = 'muxx',
	kCMMediaType_Text = 'text',
	kCMMediaType_ClosedCaption = 'clcp',
	kCMMediaType_Subtitle = 'sbtl',
	kCMMediaType_TimeCode = 'tmcd',
	kCMMediaType_Metadata = 'meta',
	kCMMediaType_TaggedBufferGroup = 'tbgr',
};

enum {
	kCMVideoCodecType_422YpCbCr8 = '2vuy',
	kCMVideoCodecType_Animation = 'rle ',
	kCMVideoCodecType_Cinepak = 'cvid',
	kCMVideoCodecType_JPEG = 'jpeg',
	kCMVideoCodecType_JPEG_OpenDML = 'dmb1',
	kCMVideoCodecType_SorensonVideo = 'SVQ1',
	kCMVideoCodecType_SorensonVideo3 = 'SVQ3',
	kCMVideoCodecType_H263 = 'h263',
	kCMVideoCodecType_H264 = 'avc1',
	kCMVideoCodecType_HEVC = 'hvc1',
	kCMVideoCodecType_HEVCWithAlpha = 'muxa',
	kCMVideoCodecType_DolbyVisionHEVC = 'dvh1',
	kCMVideoCodecType_MPEG4Video = 'mp4v',
	kCMVideoCodecType_MPEG2Video = 'mp2v',
	kCMVideoCodecType_MPEG1Video = 'mp1v',
	kCMVideoCodecType_VP9 = 'vp09',
	kCMVideoCodecType_DVCNTSC = 'dvc ',
	kCMVideoCodecType_DVCPAL = 'dvcp',
	kCMVideoCodecType_DVCProPAL = 'dvpp',
	kCMVideoCodecType_DVCPro50NTSC = 'dv5n',
	kCMVideoCodecType_DVCPro50PAL = 'dv5p',
	kCMVideoCodecType_AppleProRes4444XQ = 'ap4x',
	kCMVideoCodecType_AppleProRes4444 = 'ap4h',
	kCMVideoCodecType_AppleProRes422HQ = 'apch',
	kCMVideoCodecType_AppleProRes422 = 'apcn',
	kCMVideoCodecType_AppleProRes422LT = 'apcs',
	kCMVideoCodecType_AppleProRes422Proxy = 'apco',
	kCMVideoCodecType_AppleProResRAW = 'aprn',
	kCMVideoCodecType_AppleProResRAWHQ = 'aprh',
	kCMVideoCodecType_AV1 = 'av01',
};

typedef struct {
	int32_t width;
	int32_t height;
} CMVideoDimensions;

CM_EXPORT const CFStringRef kCMFormatDescriptionTransferFunction_ITU_R_709_2;
CM_EXPORT const CFStringRef kCMFormatDescriptionTransferFunction_SMPTE_240M_1995;
CM_EXPORT const CFStringRef kCMFormatDescriptionTransferFunction_UseGamma;
CM_EXPORT const CFStringRef kCMFormatDescriptionTransferFunction_ITU_R_2020;
CM_EXPORT const CFStringRef kCMFormatDescriptionTransferFunction_ITU_R_2100_HLG;
CM_EXPORT const CFStringRef kCMFormatDescriptionTransferFunction_SMPTE_ST_2084_PQ;
CM_EXPORT const CFStringRef kCMFormatDescriptionTransferFunction_SMPTE_ST_428_1;
CM_EXPORT const CFStringRef kCMFormatDescriptionTransferFunction_Linear;
CM_EXPORT const CFStringRef kCMFormatDescriptionTransferFunction_sRGB;
CM_EXPORT const CFStringRef kCMFormatDescriptionColorPrimaries_ITU_R_709_2;
CM_EXPORT const CFStringRef kCMFormatDescriptionColorPrimaries_EBU_3213;
CM_EXPORT const CFStringRef kCMFormatDescriptionColorPrimaries_SMPTE_C;
CM_EXPORT const CFStringRef kCMFormatDescriptionColorPrimaries_ITU_R_2020;
CM_EXPORT const CFStringRef kCMFormatDescriptionColorPrimaries_P3_D65;
CM_EXPORT const CFStringRef kCMFormatDescriptionColorPrimaries_DCI_P3;
CM_EXPORT const CFStringRef kCMFormatDescriptionYCbCrMatrix_ITU_R_709_2;
CM_EXPORT const CFStringRef kCMFormatDescriptionYCbCrMatrix_ITU_R_601_4;
CM_EXPORT const CFStringRef kCMFormatDescriptionYCbCrMatrix_SMPTE_240M_1995;
CM_EXPORT const CFStringRef kCMFormatDescriptionYCbCrMatrix_ITU_R_2020;
CM_EXPORT const CFStringRef kCMFormatDescriptionExtension_ContentLightLevelInfo;
CM_EXPORT const CFStringRef kCMFormatDescriptionExtension_Depth;
CM_EXPORT const CFStringRef kCMFormatDescriptionExtension_FormatName;
CM_EXPORT const CFStringRef kCMFormatDescriptionExtension_FullRangeVideo;
CM_EXPORT const CFStringRef kCMFormatDescriptionExtension_MasteringDisplayColorVolume;
CM_EXPORT const CFStringRef kCMFormatDescriptionExtension_SampleDescriptionExtensionAtoms;
CM_EXPORT const CFStringRef kCMFormatDescriptionExtension_VerbatimSampleDescription;

CM_EXPORT CFTypeID CMFormatDescriptionGetTypeID(void);
CM_EXPORT Boolean CMFormatDescriptionEqual(CMFormatDescriptionRef desc, CMFormatDescriptionRef otherDesc);
CM_EXPORT CMMediaType CMFormatDescriptionGetMediaType(CMFormatDescriptionRef desc);
CM_EXPORT FourCharCode CMFormatDescriptionGetMediaSubType(CMFormatDescriptionRef desc);
CM_EXPORT CFDictionaryRef CMFormatDescriptionGetExtensions(CMFormatDescriptionRef desc);
CM_EXPORT CFPropertyListRef CMFormatDescriptionGetExtension(CMFormatDescriptionRef desc, CFStringRef extensionKey);

CM_EXPORT OSStatus CMVideoFormatDescriptionCreate(CFAllocatorRef allocator, CMVideoCodecType codecType,
	int32_t width, int32_t height, CFDictionaryRef extensions,
	CF_RETURNS_RETAINED CMVideoFormatDescriptionRef *formatDescriptionOut);
CM_EXPORT OSStatus CMVideoFormatDescriptionCreateForImageBuffer(CFAllocatorRef allocator, CVImageBufferRef imageBuffer,
	CF_RETURNS_RETAINED CMVideoFormatDescriptionRef *formatDescriptionOut);
CM_EXPORT Boolean CMVideoFormatDescriptionMatchesImageBuffer(CMVideoFormatDescriptionRef desc, CVImageBufferRef imageBuffer);
CM_EXPORT CMVideoDimensions CMVideoFormatDescriptionGetDimensions(CMVideoFormatDescriptionRef videoDesc);
CM_EXPORT OSStatus CMVideoFormatDescriptionGetH264ParameterSetAtIndex(CMFormatDescriptionRef videoDesc,
	size_t parameterSetIndex, const uint8_t **parameterSetPointerOut, size_t *parameterSetSizeOut,
	size_t *parameterSetCountOut, int *NALUnitHeaderLengthOut);

CM_EXPORT OSStatus CMAudioFormatDescriptionCreate(CFAllocatorRef allocator, const AudioStreamBasicDescription *asbd,
	size_t layoutSize, const AudioChannelLayout *layout, size_t magicCookieSize, const void *magicCookie,
	CFDictionaryRef extensions, CF_RETURNS_RETAINED CMAudioFormatDescriptionRef *formatDescriptionOut);
CM_EXPORT const AudioStreamBasicDescription *CMAudioFormatDescriptionGetStreamBasicDescription(
	CMAudioFormatDescriptionRef desc);
CM_EXPORT const AudioChannelLayout *CMAudioFormatDescriptionGetChannelLayout(CMAudioFormatDescriptionRef desc,
	size_t *sizeOut);
CM_EXPORT const void *CMAudioFormatDescriptionGetMagicCookie(CMAudioFormatDescriptionRef desc, size_t *sizeOut);

CF_IMPLICIT_BRIDGING_DISABLED

#endif
