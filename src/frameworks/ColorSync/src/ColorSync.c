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


#include <ColorSync/ColorSync.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* CGDisplayCreateUUIDFromDisplayID(void)
{
    if (verbose) puts("STUB: CGDisplayCreateUUIDFromDisplayID called");
    return NULL;
}

void* CGDisplayGetDisplayIDFromUUID(void)
{
    if (verbose) puts("STUB: CGDisplayGetDisplayIDFromUUID called");
    return NULL;
}

void* CMProfileFromColorSyncProfile(void)
{
    if (verbose) puts("STUB: CMProfileFromColorSyncProfile called");
    return NULL;
}

void* ColorSyncCMMCopyCMMIdentifier(void)
{
    if (verbose) puts("STUB: ColorSyncCMMCopyCMMIdentifier called");
    return NULL;
}

void* ColorSyncCMMCopyLocalizedName(void)
{
    if (verbose) puts("STUB: ColorSyncCMMCopyLocalizedName called");
    return NULL;
}

void* ColorSyncCMMCreate(void)
{
    if (verbose) puts("STUB: ColorSyncCMMCreate called");
    return NULL;
}

void* ColorSyncCMMGetBundle(void)
{
    if (verbose) puts("STUB: ColorSyncCMMGetBundle called");
    return NULL;
}

void* ColorSyncCMMGetTypeID(void)
{
    if (verbose) puts("STUB: ColorSyncCMMGetTypeID called");
    return NULL;
}

void* ColorSyncCodeFragmentGetMD5(void)
{
    if (verbose) puts("STUB: ColorSyncCodeFragmentGetMD5 called");
    return NULL;
}

void* ColorSyncConvertDoubleXYZToD50(void)
{
    if (verbose) puts("STUB: ColorSyncConvertDoubleXYZToD50 called");
    return NULL;
}

void* ColorSyncConvertXYZToXYZWithChromaticAdaptationMatrix(void)
{
    if (verbose) puts("STUB: ColorSyncConvertXYZToXYZWithChromaticAdaptationMatrix called");
    return NULL;
}

void* ColorSyncCreateCodeFragment(void)
{
    if (verbose) puts("STUB: ColorSyncCreateCodeFragment called");
    return NULL;
}

void* ColorSyncCreateCodeFragmentPriv(void)
{
    if (verbose) puts("STUB: ColorSyncCreateCodeFragmentPriv called");
    return NULL;
}

void* ColorSyncCreateGammaTablesFromVCGT(void)
{
    if (verbose) puts("STUB: ColorSyncCreateGammaTablesFromVCGT called");
    return NULL;
}

void* ColorSyncCreateInputPoppyProfile(void)
{
    if (verbose) puts("STUB: ColorSyncCreateInputPoppyProfile called");
    return NULL;
}

void* ColorSyncCreateOutputPoppyProfile(void)
{
    if (verbose) puts("STUB: ColorSyncCreateOutputPoppyProfile called");
    return NULL;
}

void* ColorSyncCreateOutputPoppyProfileForRGBData(void)
{
    if (verbose) puts("STUB: ColorSyncCreateOutputPoppyProfileForRGBData called");
    return NULL;
}

void* ColorSyncCreateSignatureFromFourCharCode(void)
{
    if (verbose) puts("STUB: ColorSyncCreateSignatureFromFourCharCode called");
    return NULL;
}

void* ColorSyncDataCanBeIgnoredOnSRGBDevice(void)
{
    if (verbose) puts("STUB: ColorSyncDataCanBeIgnoredOnSRGBDevice called");
    return NULL;
}

void* ColorSyncDeviceCopyDeviceInfo(void)
{
    if (verbose) puts("STUB: ColorSyncDeviceCopyDeviceInfo called");
    return NULL;
}

void* ColorSyncDeviceCopyDeviceRegistrationInfo(void)
{
    if (verbose) puts("STUB: ColorSyncDeviceCopyDeviceRegistrationInfo called");
    return NULL;
}

void* ColorSyncDeviceSetCustomProfiles(void)
{
    if (verbose) puts("STUB: ColorSyncDeviceSetCustomProfiles called");
    return NULL;
}

void* ColorSyncDisplayServicesUpdateProfiles(void)
{
    if (verbose) puts("STUB: ColorSyncDisplayServicesUpdateProfiles called");
    return NULL;
}

void* ColorSyncGetAdaptationMtx(void)
{
    if (verbose) puts("STUB: ColorSyncGetAdaptationMtx called");
    return NULL;
}

void* ColorSyncGetDisplayTransferFormulaFromVCGT(void)
{
    if (verbose) puts("STUB: ColorSyncGetDisplayTransferFormulaFromVCGT called");
    return NULL;
}

void* ColorSyncGetFourCharCodeFromSignature(void)
{
    if (verbose) puts("STUB: ColorSyncGetFourCharCodeFromSignature called");
    return NULL;
}

void* ColorSyncGetMainDisplayID(void)
{
    if (verbose) puts("STUB: ColorSyncGetMainDisplayID called");
    return NULL;
}

void* ColorSyncGetUInt32FromUUID(void)
{
    if (verbose) puts("STUB: ColorSyncGetUInt32FromUUID called");
    return NULL;
}

void* ColorSyncIterateDeviceProfiles(void)
{
    if (verbose) puts("STUB: ColorSyncIterateDeviceProfiles called");
    return NULL;
}

void* ColorSyncIterateInstalledCMMs(void)
{
    if (verbose) puts("STUB: ColorSyncIterateInstalledCMMs called");
    return NULL;
}

void* ColorSyncIterateInstalledProfiles(void)
{
    if (verbose) puts("STUB: ColorSyncIterateInstalledProfiles called");
    return NULL;
}

void* ColorSyncIterateInstalledProfilesWithOptions(void)
{
    if (verbose) puts("STUB: ColorSyncIterateInstalledProfilesWithOptions called");
    return NULL;
}

void* ColorSyncMakeProfile(void)
{
    if (verbose) puts("STUB: ColorSyncMakeProfile called");
    return NULL;
}

void* ColorSyncMatrixGetFunction(void)
{
    if (verbose) puts("STUB: ColorSyncMatrixGetFunction called");
    return NULL;
}

void* ColorSyncMatrixGetOptions(void)
{
    if (verbose) puts("STUB: ColorSyncMatrixGetOptions called");
    return NULL;
}

void* ColorSyncMatrixGetProperties(void)
{
    if (verbose) puts("STUB: ColorSyncMatrixGetProperties called");
    return NULL;
}

void* ColorSyncMatrixGetRange(void)
{
    if (verbose) puts("STUB: ColorSyncMatrixGetRange called");
    return NULL;
}

void* ColorSyncMatrixGetTypeID(void)
{
    if (verbose) puts("STUB: ColorSyncMatrixGetTypeID called");
    return NULL;
}

void* ColorSyncNxMLUTGetFunction(void)
{
    if (verbose) puts("STUB: ColorSyncNxMLUTGetFunction called");
    return NULL;
}

void* ColorSyncNxMLUTGetOptions(void)
{
    if (verbose) puts("STUB: ColorSyncNxMLUTGetOptions called");
    return NULL;
}

void* ColorSyncNxMLUTGetProperties(void)
{
    if (verbose) puts("STUB: ColorSyncNxMLUTGetProperties called");
    return NULL;
}

void* ColorSyncNxMLUTGetRange(void)
{
    if (verbose) puts("STUB: ColorSyncNxMLUTGetRange called");
    return NULL;
}

void* ColorSyncNxMLUTGetTypeID(void)
{
    if (verbose) puts("STUB: ColorSyncNxMLUTGetTypeID called");
    return NULL;
}

void* ColorSyncProfileContainsTag(void)
{
    if (verbose) puts("STUB: ColorSyncProfileContainsTag called");
    return NULL;
}

void* ColorSyncProfileCopyASCIIDescriptionString(void)
{
    if (verbose) puts("STUB: ColorSyncProfileCopyASCIIDescriptionString called");
    return NULL;
}

void* ColorSyncProfileCopyData(void)
{
    if (verbose) puts("STUB: ColorSyncProfileCopyData called");
    return NULL;
}

void* ColorSyncProfileCopyDescriptionString(void)
{
    if (verbose) puts("STUB: ColorSyncProfileCopyDescriptionString called");
    return NULL;
}

void* ColorSyncProfileCopyHeader(void)
{
    if (verbose) puts("STUB: ColorSyncProfileCopyHeader called");
    return NULL;
}

void* ColorSyncProfileCopyLocalizedDescriptionString(void)
{
    if (verbose) puts("STUB: ColorSyncProfileCopyLocalizedDescriptionString called");
    return NULL;
}

void* ColorSyncProfileCopyLocalizedStringDictionary(void)
{
    if (verbose) puts("STUB: ColorSyncProfileCopyLocalizedStringDictionary called");
    return NULL;
}

void* ColorSyncProfileCopyTag(void)
{
    if (verbose) puts("STUB: ColorSyncProfileCopyTag called");
    return NULL;
}

void* ColorSyncProfileCopyTagSignatures(void)
{
    if (verbose) puts("STUB: ColorSyncProfileCopyTagSignatures called");
    return NULL;
}

void* ColorSyncProfileCreate(void)
{
    if (verbose) puts("STUB: ColorSyncProfileCreate called");
    return NULL;
}

void* ColorSyncProfileCreateDeviceProfile(void)
{
    if (verbose) puts("STUB: ColorSyncProfileCreateDeviceProfile called");
    return NULL;
}

void* ColorSyncProfileCreateDisplayTransferTablesFromVCGT(void)
{
    if (verbose) puts("STUB: ColorSyncProfileCreateDisplayTransferTablesFromVCGT called");
    return NULL;
}

void* ColorSyncProfileCreateLink(void)
{
    if (verbose) puts("STUB: ColorSyncProfileCreateLink called");
    return NULL;
}

void* ColorSyncProfileCreateMutable(void)
{
    if (verbose) puts("STUB: ColorSyncProfileCreateMutable called");
    return NULL;
}

void* ColorSyncProfileCreateMutableCopy(void)
{
    if (verbose) puts("STUB: ColorSyncProfileCreateMutableCopy called");
    return NULL;
}

void* ColorSyncProfileCreateSanitizedCopy(void)
{
    if (verbose) puts("STUB: ColorSyncProfileCreateSanitizedCopy called");
    return NULL;
}

void* ColorSyncProfileCreateWithDisplayID(void)
{
    if (verbose) puts("STUB: ColorSyncProfileCreateWithDisplayID called");
    return NULL;
}

void* ColorSyncProfileCreateWithName(void)
{
    if (verbose) puts("STUB: ColorSyncProfileCreateWithName called");
    return NULL;
}

void* ColorSyncProfileCreateWithURL(void)
{
    if (verbose) puts("STUB: ColorSyncProfileCreateWithURL called");
    return NULL;
}

void* ColorSyncProfileEstimateGamma(void)
{
    if (verbose) puts("STUB: ColorSyncProfileEstimateGamma called");
    return NULL;
}

void* ColorSyncProfileEstimateGammaWithDisplayID(void)
{
    if (verbose) puts("STUB: ColorSyncProfileEstimateGammaWithDisplayID called");
    return NULL;
}

void* ColorSyncProfileFromCMProfile(void)
{
    if (verbose) puts("STUB: ColorSyncProfileFromCMProfile called");
    return NULL;
}

void* ColorSyncProfileGetData(void)
{
    if (verbose) puts("STUB: ColorSyncProfileGetData called");
    return NULL;
}

void* ColorSyncProfileGetDisplayTransferFormulaFromVCGT(void)
{
    if (verbose) puts("STUB: ColorSyncProfileGetDisplayTransferFormulaFromVCGT called");
    return NULL;
}

void* ColorSyncProfileGetMD5(void)
{
    if (verbose) puts("STUB: ColorSyncProfileGetMD5 called");
    return NULL;
}

void* ColorSyncProfileGetTypeID(void)
{
    if (verbose) puts("STUB: ColorSyncProfileGetTypeID called");
    return NULL;
}

void* ColorSyncProfileGetURL(void)
{
    if (verbose) puts("STUB: ColorSyncProfileGetURL called");
    return NULL;
}

void* ColorSyncProfileInstall(void)
{
    if (verbose) puts("STUB: ColorSyncProfileInstall called");
    return NULL;
}

void* ColorSyncProfileIsWideGamut(void)
{
    if (verbose) puts("STUB: ColorSyncProfileIsWideGamut called");
    return NULL;
}

void* ColorSyncProfileProducesMatrixTransform(void)
{
    if (verbose) puts("STUB: ColorSyncProfileProducesMatrixTransform called");
    return NULL;
}

void* ColorSyncProfileRemoveTag(void)
{
    if (verbose) puts("STUB: ColorSyncProfileRemoveTag called");
    return NULL;
}

void* ColorSyncProfileSetDescriptionTag(void)
{
    if (verbose) puts("STUB: ColorSyncProfileSetDescriptionTag called");
    return NULL;
}

void* ColorSyncProfileSetHeader(void)
{
    if (verbose) puts("STUB: ColorSyncProfileSetHeader called");
    return NULL;
}

void* ColorSyncProfileSetLocalizedStringDictionary(void)
{
    if (verbose) puts("STUB: ColorSyncProfileSetLocalizedStringDictionary called");
    return NULL;
}

void* ColorSyncProfileSetParametricTRCTag(void)
{
    if (verbose) puts("STUB: ColorSyncProfileSetParametricTRCTag called");
    return NULL;
}

void* ColorSyncProfileSetTag(void)
{
    if (verbose) puts("STUB: ColorSyncProfileSetTag called");
    return NULL;
}

void* ColorSyncProfileStreamPS2Definition(void)
{
    if (verbose) puts("STUB: ColorSyncProfileStreamPS2Definition called");
    return NULL;
}

void* ColorSyncProfileUninstall(void)
{
    if (verbose) puts("STUB: ColorSyncProfileUninstall called");
    return NULL;
}

void* ColorSyncProfileVerify(void)
{
    if (verbose) puts("STUB: ColorSyncProfileVerify called");
    return NULL;
}

void* ColorSyncProfileWriteToFile(void)
{
    if (verbose) puts("STUB: ColorSyncProfileWriteToFile called");
    return NULL;
}

void* ColorSyncPureGammaOriginalTRC(void)
{
    if (verbose) puts("STUB: ColorSyncPureGammaOriginalTRC called");
    return NULL;
}

void* ColorSyncSwapProfileHeader(void)
{
    if (verbose) puts("STUB: ColorSyncSwapProfileHeader called");
    return NULL;
}

void* ColorSyncTRCGetFunction(void)
{
    if (verbose) puts("STUB: ColorSyncTRCGetFunction called");
    return NULL;
}

void* ColorSyncTRCGetGammaID(void)
{
    if (verbose) puts("STUB: ColorSyncTRCGetGammaID called");
    return NULL;
}

void* ColorSyncTRCGetLuminanceScale(void)
{
    if (verbose) puts("STUB: ColorSyncTRCGetLuminanceScale called");
    return NULL;
}

void* ColorSyncTRCGetOptions(void)
{
    if (verbose) puts("STUB: ColorSyncTRCGetOptions called");
    return NULL;
}

void* ColorSyncTRCGetProperties(void)
{
    if (verbose) puts("STUB: ColorSyncTRCGetProperties called");
    return NULL;
}

void* ColorSyncTRCGetRange(void)
{
    if (verbose) puts("STUB: ColorSyncTRCGetRange called");
    return NULL;
}

void* ColorSyncTRCGetTypeID(void)
{
    if (verbose) puts("STUB: ColorSyncTRCGetTypeID called");
    return NULL;
}

void* ColorSyncTransformConvert(void)
{
    if (verbose) puts("STUB: ColorSyncTransformConvert called");
    return NULL;
}

void* ColorSyncTransformCopyProperty(void)
{
    if (verbose) puts("STUB: ColorSyncTransformCopyProperty called");
    return NULL;
}

void* ColorSyncTransformCreate(void)
{
    if (verbose) puts("STUB: ColorSyncTransformCreate called");
    return NULL;
}

void* ColorSyncTransformCreateWithName(void)
{
    if (verbose) puts("STUB: ColorSyncTransformCreateWithName called");
    return NULL;
}

void* ColorSyncTransformGetTypeID(void)
{
    if (verbose) puts("STUB: ColorSyncTransformGetTypeID called");
    return NULL;
}

void* ColorSyncTransformIterate(void)
{
    if (verbose) puts("STUB: ColorSyncTransformIterate called");
    return NULL;
}

void* ColorSyncTransformSetProperty(void)
{
    if (verbose) puts("STUB: ColorSyncTransformSetProperty called");
    return NULL;
}

void* ColorSyncVerifyAdobeRGBData(void)
{
    if (verbose) puts("STUB: ColorSyncVerifyAdobeRGBData called");
    return NULL;
}

void* ColorSyncVerifyDisplayP3Data(void)
{
    if (verbose) puts("STUB: ColorSyncVerifyDisplayP3Data called");
    return NULL;
}

void* ColorSyncVerifyGenericGrayGamma2_2Data(void)
{
    if (verbose) puts("STUB: ColorSyncVerifyGenericGrayGamma2_2Data called");
    return NULL;
}

void* ColorSyncVerifyGenericLabData(void)
{
    if (verbose) puts("STUB: ColorSyncVerifyGenericLabData called");
    return NULL;
}

void* ColorSyncVerifySRGBData(void)
{
    if (verbose) puts("STUB: ColorSyncVerifySRGBData called");
    return NULL;
}

// stubs with fixed-up declarations

CFUUIDRef ColorSyncCreateUUIDFromUInt32(unsigned int uuid) {
    if (verbose) puts("STUB: ColorSyncCreateUUIDFromUInt32 called");
    return NULL;
};

bool ColorSyncRegisterDevice(CFStringRef class, CFUUIDRef uuid, CFDictionaryRef info) {
    if (verbose) puts("STUB: ColorSyncRegisterDevice called");
    return 0;
};

bool ColorSyncUnregisterDevice(CFStringRef class, CFUUIDRef uuid) {
    if (verbose) puts("STUB: ColorSyncUnregisterDevice called");
    return 0;
};

// someone please check these and replace them with the correct values
CFStringRef kColorSyncDeviceProfileURL       = CFSTR("ColorSyncDeviceProfileURL");
CFStringRef kColorSyncDeviceModeDescriptions = CFSTR("ColorSyncDeviceModeDescriptions");
CFStringRef kColorSyncDeviceDefaultProfileID = CFSTR("ColorSyncDeviceDefaultProfileID");
CFStringRef kColorSyncDeviceDescriptions     = CFSTR("ColorSyncDeviceDescriptions");
CFStringRef kColorSyncFactoryProfiles        = CFSTR("ColorSyncFactoryProfiles");
CFStringRef kColorSyncDeviceUserScope        = CFSTR("ColorSyncDeviceUserScope");
CFStringRef kColorSyncDeviceHostScope        = CFSTR("ColorSyncDeviceHostScope");
CFStringRef kColorSyncPrinterDeviceClass     = CFSTR("ColorSyncPrinterDeviceClass");
