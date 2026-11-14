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

#ifndef _CMSAMPLEBUFFER_H_
#define _CMSAMPLEBUFFER_H_

#include <CoreFoundation/CoreFoundation.h>
#include <CoreMedia/CMBase.h>
#include <CoreMedia/CMTime.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CF_BRIDGED_TYPE(id) opaqueCMSampleBuffer *CMSampleBufferRef;

// Timing of one sample, or of every sample of a buffer when they share it.
#pragma pack(push, 4)
typedef struct
{
	CMTime duration;
	CMTime presentationTimeStamp;
	CMTime decodeTimeStamp;
}
CMSampleTimingInfo;
#pragma pack(pop)

// Every time of the structure is invalid.
extern const CMSampleTimingInfo kCMTimingInfoInvalid;

extern const CFStringRef kCMSampleAttachmentKey_DisplayImmediately;
extern const CFStringRef kCMSampleAttachmentKey_NotSync;
extern const CFStringRef kCMSampleAttachmentKey_IsDependedOnByOthers;

// Sample buffer attachment: the decoder is reset before this buffer is decoded.
extern const CFStringRef kCMSampleBufferAttachmentKey_ResetDecoderBeforeDecoding;

#ifdef __cplusplus
}
#endif

#endif
