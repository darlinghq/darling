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

#include <CoreFoundation/CoreFoundation.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const CFStringRef kCMFormatDescriptionTransferFunction_ITU_R_709_2;
extern const CFStringRef kCMFormatDescriptionTransferFunction_SMPTE_240M_1995;
extern const CFStringRef kCMFormatDescriptionTransferFunction_UseGamma;
extern const CFStringRef kCMFormatDescriptionTransferFunction_ITU_R_2020;
extern const CFStringRef kCMFormatDescriptionTransferFunction_ITU_R_2100_HLG;
extern const CFStringRef kCMFormatDescriptionTransferFunction_SMPTE_ST_2084_PQ;
extern const CFStringRef kCMFormatDescriptionTransferFunction_SMPTE_ST_428_1;
extern const CFStringRef kCMFormatDescriptionTransferFunction_Linear;
extern const CFStringRef kCMFormatDescriptionTransferFunction_sRGB;
extern const CFStringRef kCMFormatDescriptionColorPrimaries_ITU_R_709_2;
extern const CFStringRef kCMFormatDescriptionColorPrimaries_EBU_3213;
extern const CFStringRef kCMFormatDescriptionColorPrimaries_SMPTE_C;
extern const CFStringRef kCMFormatDescriptionColorPrimaries_ITU_R_2020;
extern const CFStringRef kCMFormatDescriptionColorPrimaries_P3_D65;
extern const CFStringRef kCMFormatDescriptionColorPrimaries_DCI_P3;
extern const CFStringRef kCMFormatDescriptionYCbCrMatrix_ITU_R_709_2;
extern const CFStringRef kCMFormatDescriptionYCbCrMatrix_ITU_R_601_4;
extern const CFStringRef kCMFormatDescriptionYCbCrMatrix_SMPTE_240M_1995;
extern const CFStringRef kCMFormatDescriptionYCbCrMatrix_ITU_R_2020;
extern const CFStringRef kCMFormatDescriptionExtension_ContentLightLevelInfo;
extern const CFStringRef kCMFormatDescriptionExtension_Depth;
extern const CFStringRef kCMFormatDescriptionExtension_FormatName;
extern const CFStringRef kCMFormatDescriptionExtension_FullRangeVideo;
extern const CFStringRef kCMFormatDescriptionExtension_MasteringDisplayColorVolume;
extern const CFStringRef kCMFormatDescriptionExtension_SampleDescriptionExtensionAtoms;

#ifdef __cplusplus
}
#endif

#endif
