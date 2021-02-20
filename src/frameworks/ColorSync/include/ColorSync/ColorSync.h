/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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


#ifndef _ColorSync_H_
#define _ColorSync_H_

#include <CoreFoundation/CoreFoundation.h>

#ifdef __cplusplus
extern "C" {
#endif

void* CGDisplayCreateUUIDFromDisplayID(void);
void* CGDisplayGetDisplayIDFromUUID(void);
void* CMProfileFromColorSyncProfile(void);
void* ColorSyncCMMCopyCMMIdentifier(void);
void* ColorSyncCMMCopyLocalizedName(void);
void* ColorSyncCMMCreate(void);
void* ColorSyncCMMGetBundle(void);
void* ColorSyncCMMGetTypeID(void);
void* ColorSyncCodeFragmentGetMD5(void);
void* ColorSyncConvertDoubleXYZToD50(void);
void* ColorSyncConvertXYZToXYZWithChromaticAdaptationMatrix(void);
void* ColorSyncCreateCodeFragment(void);
void* ColorSyncCreateCodeFragmentPriv(void);
void* ColorSyncCreateGammaTablesFromVCGT(void);
void* ColorSyncCreateInputPoppyProfile(void);
void* ColorSyncCreateOutputPoppyProfile(void);
void* ColorSyncCreateOutputPoppyProfileForRGBData(void);
void* ColorSyncCreateSignatureFromFourCharCode(void);
void* ColorSyncDataCanBeIgnoredOnSRGBDevice(void);
void* ColorSyncDeviceCopyDeviceInfo(void);
void* ColorSyncDeviceCopyDeviceRegistrationInfo(void);
void* ColorSyncDeviceSetCustomProfiles(void);
void* ColorSyncDisplayServicesUpdateProfiles(void);
void* ColorSyncGetAdaptationMtx(void);
void* ColorSyncGetDisplayTransferFormulaFromVCGT(void);
void* ColorSyncGetFourCharCodeFromSignature(void);
void* ColorSyncGetMainDisplayID(void);
void* ColorSyncGetUInt32FromUUID(void);
void* ColorSyncIterateDeviceProfiles(void);
void* ColorSyncIterateInstalledCMMs(void);
void* ColorSyncIterateInstalledProfiles(void);
void* ColorSyncIterateInstalledProfilesWithOptions(void);
void* ColorSyncMakeProfile(void);
void* ColorSyncMatrixGetFunction(void);
void* ColorSyncMatrixGetOptions(void);
void* ColorSyncMatrixGetProperties(void);
void* ColorSyncMatrixGetRange(void);
void* ColorSyncMatrixGetTypeID(void);
void* ColorSyncNxMLUTGetFunction(void);
void* ColorSyncNxMLUTGetOptions(void);
void* ColorSyncNxMLUTGetProperties(void);
void* ColorSyncNxMLUTGetRange(void);
void* ColorSyncNxMLUTGetTypeID(void);
void* ColorSyncProfileContainsTag(void);
void* ColorSyncProfileCopyASCIIDescriptionString(void);
void* ColorSyncProfileCopyData(void);
void* ColorSyncProfileCopyDescriptionString(void);
void* ColorSyncProfileCopyHeader(void);
void* ColorSyncProfileCopyLocalizedDescriptionString(void);
void* ColorSyncProfileCopyLocalizedStringDictionary(void);
void* ColorSyncProfileCopyTag(void);
void* ColorSyncProfileCopyTagSignatures(void);
void* ColorSyncProfileCreate(void);
void* ColorSyncProfileCreateDeviceProfile(void);
void* ColorSyncProfileCreateDisplayTransferTablesFromVCGT(void);
void* ColorSyncProfileCreateLink(void);
void* ColorSyncProfileCreateMutable(void);
void* ColorSyncProfileCreateMutableCopy(void);
void* ColorSyncProfileCreateSanitizedCopy(void);
void* ColorSyncProfileCreateWithDisplayID(void);
void* ColorSyncProfileCreateWithName(void);
void* ColorSyncProfileCreateWithURL(void);
void* ColorSyncProfileEstimateGamma(void);
void* ColorSyncProfileEstimateGammaWithDisplayID(void);
void* ColorSyncProfileFromCMProfile(void);
void* ColorSyncProfileGetData(void);
void* ColorSyncProfileGetDisplayTransferFormulaFromVCGT(void);
void* ColorSyncProfileGetMD5(void);
void* ColorSyncProfileGetTypeID(void);
void* ColorSyncProfileGetURL(void);
void* ColorSyncProfileInstall(void);
void* ColorSyncProfileIsWideGamut(void);
void* ColorSyncProfileProducesMatrixTransform(void);
void* ColorSyncProfileRemoveTag(void);
void* ColorSyncProfileSetDescriptionTag(void);
void* ColorSyncProfileSetHeader(void);
void* ColorSyncProfileSetLocalizedStringDictionary(void);
void* ColorSyncProfileSetParametricTRCTag(void);
void* ColorSyncProfileSetTag(void);
void* ColorSyncProfileStreamPS2Definition(void);
void* ColorSyncProfileUninstall(void);
void* ColorSyncProfileVerify(void);
void* ColorSyncProfileWriteToFile(void);
void* ColorSyncPureGammaOriginalTRC(void);
void* ColorSyncSwapProfileHeader(void);
void* ColorSyncTRCGetFunction(void);
void* ColorSyncTRCGetGammaID(void);
void* ColorSyncTRCGetLuminanceScale(void);
void* ColorSyncTRCGetOptions(void);
void* ColorSyncTRCGetProperties(void);
void* ColorSyncTRCGetRange(void);
void* ColorSyncTRCGetTypeID(void);
void* ColorSyncTransformConvert(void);
void* ColorSyncTransformCopyProperty(void);
void* ColorSyncTransformCreate(void);
void* ColorSyncTransformCreateWithName(void);
void* ColorSyncTransformGetTypeID(void);
void* ColorSyncTransformIterate(void);
void* ColorSyncTransformSetProperty(void);
void* ColorSyncVerifyAdobeRGBData(void);
void* ColorSyncVerifyDisplayP3Data(void);
void* ColorSyncVerifyGenericGrayGamma2_2Data(void);
void* ColorSyncVerifyGenericLabData(void);
void* ColorSyncVerifySRGBData(void);

// fixed-up declarations

CFUUIDRef ColorSyncCreateUUIDFromUInt32(unsigned int uuid);
bool ColorSyncRegisterDevice(CFStringRef deviceClass, CFUUIDRef uuid, CFDictionaryRef info);
bool ColorSyncUnregisterDevice(CFStringRef deviceClass, CFUUIDRef uuid);

extern CFStringRef kColorSyncDeviceProfileURL;
extern CFStringRef kColorSyncDeviceModeDescriptions;
extern CFStringRef kColorSyncDeviceDefaultProfileID;
extern CFStringRef kColorSyncDeviceDescriptions;
extern CFStringRef kColorSyncFactoryProfiles;
extern CFStringRef kColorSyncDeviceUserScope;
extern CFStringRef kColorSyncDeviceHostScope;
extern CFStringRef kColorSyncPrinterDeviceClass;

#ifdef __cplusplus
};
#endif

#endif
