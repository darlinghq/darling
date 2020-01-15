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


#include <ColorSyncLegacy/ColorSyncLegacy.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* CMCloneProfileRef(void)
{
    if (verbose) puts("STUB: CMCloneProfileRef called");
    return NULL;
}

void* CMCloseProfile(void)
{
    if (verbose) puts("STUB: CMCloseProfile called");
    return NULL;
}

void* CMConvertDoubleXYZToD50(void)
{
    if (verbose) puts("STUB: CMConvertDoubleXYZToD50 called");
    return NULL;
}

void* CMConvertRGBFloatBitmap(void)
{
    if (verbose) puts("STUB: CMConvertRGBFloatBitmap called");
    return NULL;
}

void* CMConvertXYZFloatBitmap(void)
{
    if (verbose) puts("STUB: CMConvertXYZFloatBitmap called");
    return NULL;
}

void* CMCopyProfile(void)
{
    if (verbose) puts("STUB: CMCopyProfile called");
    return NULL;
}

void* CMCopyProfileDescriptionString(void)
{
    if (verbose) puts("STUB: CMCopyProfileDescriptionString called");
    return NULL;
}

void* CMCopyProfileLocalizedString(void)
{
    if (verbose) puts("STUB: CMCopyProfileLocalizedString called");
    return NULL;
}

void* CMCopyProfileLocalizedStringDictionary(void)
{
    if (verbose) puts("STUB: CMCopyProfileLocalizedStringDictionary called");
    return NULL;
}

void* CMCountProfileElements(void)
{
    if (verbose) puts("STUB: CMCountProfileElements called");
    return NULL;
}

void* CMFloatBitmapMakeChunky(void)
{
    if (verbose) puts("STUB: CMFloatBitmapMakeChunky called");
    return NULL;
}

void* CMGetColorSyncVersion(void)
{
    if (verbose) puts("STUB: CMGetColorSyncVersion called");
    return NULL;
}

void* CMGetDefaultDevice(void)
{
    if (verbose) puts("STUB: CMGetDefaultDevice called");
    return NULL;
}

void* CMGetDefaultProfileBySpace(void)
{
    if (verbose) puts("STUB: CMGetDefaultProfileBySpace called");
    return NULL;
}

void* CMGetDefaultProfileByUse(void)
{
    if (verbose) puts("STUB: CMGetDefaultProfileByUse called");
    return NULL;
}

void* CMGetDeviceDefaultProfileID(void)
{
    if (verbose) puts("STUB: CMGetDeviceDefaultProfileID called");
    return NULL;
}

void* CMGetDeviceFactoryProfiles(void)
{
    if (verbose) puts("STUB: CMGetDeviceFactoryProfiles called");
    return NULL;
}

void* CMGetDeviceInfo(void)
{
    if (verbose) puts("STUB: CMGetDeviceInfo called");
    return NULL;
}

void* CMGetDeviceProfile(void)
{
    if (verbose) puts("STUB: CMGetDeviceProfile called");
    return NULL;
}

void* CMGetDeviceState(void)
{
    if (verbose) puts("STUB: CMGetDeviceState called");
    return NULL;
}

void* CMGetDeviceTrackingByUse(void)
{
    if (verbose) puts("STUB: CMGetDeviceTrackingByUse called");
    return NULL;
}

void* CMGetGammaByAVID(void)
{
    if (verbose) puts("STUB: CMGetGammaByAVID called");
    return NULL;
}

void* CMGetIndNamedColorValue(void)
{
    if (verbose) puts("STUB: CMGetIndNamedColorValue called");
    return NULL;
}

void* CMGetIndProfileElement(void)
{
    if (verbose) puts("STUB: CMGetIndProfileElement called");
    return NULL;
}

void* CMGetIndProfileElementInfo(void)
{
    if (verbose) puts("STUB: CMGetIndProfileElementInfo called");
    return NULL;
}

void* CMGetNamedColorIndex(void)
{
    if (verbose) puts("STUB: CMGetNamedColorIndex called");
    return NULL;
}

void* CMGetNamedColorInfo(void)
{
    if (verbose) puts("STUB: CMGetNamedColorInfo called");
    return NULL;
}

void* CMGetNamedColorName(void)
{
    if (verbose) puts("STUB: CMGetNamedColorName called");
    return NULL;
}

void* CMGetNamedColorValue(void)
{
    if (verbose) puts("STUB: CMGetNamedColorValue called");
    return NULL;
}

void* CMGetPS2ColorRendering(void)
{
    if (verbose) puts("STUB: CMGetPS2ColorRendering called");
    return NULL;
}

void* CMGetPS2ColorRenderingIntent(void)
{
    if (verbose) puts("STUB: CMGetPS2ColorRenderingIntent called");
    return NULL;
}

void* CMGetPS2ColorRenderingVMSize(void)
{
    if (verbose) puts("STUB: CMGetPS2ColorRenderingVMSize called");
    return NULL;
}

void* CMGetPS2ColorSpace(void)
{
    if (verbose) puts("STUB: CMGetPS2ColorSpace called");
    return NULL;
}

void* CMGetPartialProfileElement(void)
{
    if (verbose) puts("STUB: CMGetPartialProfileElement called");
    return NULL;
}

void* CMGetProfileByAVID(void)
{
    if (verbose) puts("STUB: CMGetProfileByAVID called");
    return NULL;
}

void* CMGetProfileDescriptions(void)
{
    if (verbose) puts("STUB: CMGetProfileDescriptions called");
    return NULL;
}

void* CMGetProfileElement(void)
{
    if (verbose) puts("STUB: CMGetProfileElement called");
    return NULL;
}

void* CMGetProfileGamma(void)
{
    if (verbose) puts("STUB: CMGetProfileGamma called");
    return NULL;
}

void* CMGetProfileHeader(void)
{
    if (verbose) puts("STUB: CMGetProfileHeader called");
    return NULL;
}

void* CMGetProfileLocation(void)
{
    if (verbose) puts("STUB: CMGetProfileLocation called");
    return NULL;
}

void* CMGetProfileMD5(void)
{
    if (verbose) puts("STUB: CMGetProfileMD5 called");
    return NULL;
}

void* CMGetProfileRefCount(void)
{
    if (verbose) puts("STUB: CMGetProfileRefCount called");
    return NULL;
}

void* CMGetProfileTransformInfo(void)
{
    if (verbose) puts("STUB: CMGetProfileTransformInfo called");
    return NULL;
}

void* CMGetSystemProfile(void)
{
    if (verbose) puts("STUB: CMGetSystemProfile called");
    return NULL;
}

void* CMGetSystemProfileAVID(void)
{
    if (verbose) puts("STUB: CMGetSystemProfileAVID called");
    return NULL;
}

void* CMGetSystemProfilePriv(void)
{
    if (verbose) puts("STUB: CMGetSystemProfilePriv called");
    return NULL;
}

void* CMIterateCMMInfo(void)
{
    if (verbose) puts("STUB: CMIterateCMMInfo called");
    return NULL;
}

void* CMIterateColorDevices(void)
{
    if (verbose) puts("STUB: CMIterateColorDevices called");
    return NULL;
}

void* CMIterateColorSyncFolder(void)
{
    if (verbose) puts("STUB: CMIterateColorSyncFolder called");
    return NULL;
}

void* CMIterateDeviceProfiles(void)
{
    if (verbose) puts("STUB: CMIterateDeviceProfiles called");
    return NULL;
}

void* CMLaunchControlPanel(void)
{
    if (verbose) puts("STUB: CMLaunchControlPanel called");
    return NULL;
}

void* CMMakeProfile(void)
{
    if (verbose) puts("STUB: CMMakeProfile called");
    return NULL;
}

void* CMMatchFloatBitmap(void)
{
    if (verbose) puts("STUB: CMMatchFloatBitmap called");
    return NULL;
}

void* CMNewProfile(void)
{
    if (verbose) puts("STUB: CMNewProfile called");
    return NULL;
}

void* CMOpenProfile(void)
{
    if (verbose) puts("STUB: CMOpenProfile called");
    return NULL;
}

void* CMPostLegacyDeviceNotification(void)
{
    if (verbose) puts("STUB: CMPostLegacyDeviceNotification called");
    return NULL;
}

void* CMProfileCopyICCData(void)
{
    if (verbose) puts("STUB: CMProfileCopyICCData called");
    return NULL;
}

void* CMProfileElementExists(void)
{
    if (verbose) puts("STUB: CMProfileElementExists called");
    return NULL;
}

void* CMProfileModified(void)
{
    if (verbose) puts("STUB: CMProfileModified called");
    return NULL;
}

void* CMProfileWriteToFile(void)
{
    if (verbose) puts("STUB: CMProfileWriteToFile called");
    return NULL;
}

void* CMRegisterColorDevice(void)
{
    if (verbose) puts("STUB: CMRegisterColorDevice called");
    return NULL;
}

void* CMRemoveProfileElement(void)
{
    if (verbose) puts("STUB: CMRemoveProfileElement called");
    return NULL;
}

void* CMSetDefaultDevice(void)
{
    if (verbose) puts("STUB: CMSetDefaultDevice called");
    return NULL;
}

void* CMSetDeviceDefaultProfileID(void)
{
    if (verbose) puts("STUB: CMSetDeviceDefaultProfileID called");
    return NULL;
}

void* CMSetDeviceFactoryProfiles(void)
{
    if (verbose) puts("STUB: CMSetDeviceFactoryProfiles called");
    return NULL;
}

void* CMSetDeviceProfile(void)
{
    if (verbose) puts("STUB: CMSetDeviceProfile called");
    return NULL;
}

void* CMSetDeviceState(void)
{
    if (verbose) puts("STUB: CMSetDeviceState called");
    return NULL;
}

void* CMSetDeviceTrackingByUse(void)
{
    if (verbose) puts("STUB: CMSetDeviceTrackingByUse called");
    return NULL;
}

void* CMSetGammaByAVID(void)
{
    if (verbose) puts("STUB: CMSetGammaByAVID called");
    return NULL;
}

void* CMSetPartialProfileElement(void)
{
    if (verbose) puts("STUB: CMSetPartialProfileElement called");
    return NULL;
}

void* CMSetProfileByAVID(void)
{
    if (verbose) puts("STUB: CMSetProfileByAVID called");
    return NULL;
}

void* CMSetProfileDescriptions(void)
{
    if (verbose) puts("STUB: CMSetProfileDescriptions called");
    return NULL;
}

void* CMSetProfileElement(void)
{
    if (verbose) puts("STUB: CMSetProfileElement called");
    return NULL;
}

void* CMSetProfileElementReference(void)
{
    if (verbose) puts("STUB: CMSetProfileElementReference called");
    return NULL;
}

void* CMSetProfileElementSize(void)
{
    if (verbose) puts("STUB: CMSetProfileElementSize called");
    return NULL;
}

void* CMSetProfileHeader(void)
{
    if (verbose) puts("STUB: CMSetProfileHeader called");
    return NULL;
}

void* CMSetProfileLocalizedStringDictionary(void)
{
    if (verbose) puts("STUB: CMSetProfileLocalizedStringDictionary called");
    return NULL;
}

void* CMSetSystemProfile(void)
{
    if (verbose) puts("STUB: CMSetSystemProfile called");
    return NULL;
}

void* CMSetSystemProfileAVID(void)
{
    if (verbose) puts("STUB: CMSetSystemProfileAVID called");
    return NULL;
}

void* CMUnregisterColorDevice(void)
{
    if (verbose) puts("STUB: CMUnregisterColorDevice called");
    return NULL;
}

void* CMUpdateProfile(void)
{
    if (verbose) puts("STUB: CMUpdateProfile called");
    return NULL;
}

void* CMValidateProfile(void)
{
    if (verbose) puts("STUB: CMValidateProfile called");
    return NULL;
}

void* CWCheckBitmap(void)
{
    if (verbose) puts("STUB: CWCheckBitmap called");
    return NULL;
}

void* CWCheckColors(void)
{
    if (verbose) puts("STUB: CWCheckColors called");
    return NULL;
}

void* CWColorWorldGetProperty(void)
{
    if (verbose) puts("STUB: CWColorWorldGetProperty called");
    return NULL;
}

void* CWColorWorldSetProperty(void)
{
    if (verbose) puts("STUB: CWColorWorldSetProperty called");
    return NULL;
}

void* CWConcatColorWorld(void)
{
    if (verbose) puts("STUB: CWConcatColorWorld called");
    return NULL;
}

void* CWDisposeColorWorld(void)
{
    if (verbose) puts("STUB: CWDisposeColorWorld called");
    return NULL;
}

void* CWFillLookupTexture(void)
{
    if (verbose) puts("STUB: CWFillLookupTexture called");
    return NULL;
}

void* CWGetCMMSignature(void)
{
    if (verbose) puts("STUB: CWGetCMMSignature called");
    return NULL;
}

void* CWMatchBitmap(void)
{
    if (verbose) puts("STUB: CWMatchBitmap called");
    return NULL;
}

void* CWMatchColors(void)
{
    if (verbose) puts("STUB: CWMatchColors called");
    return NULL;
}

void* CWNewLinkProfile(void)
{
    if (verbose) puts("STUB: CWNewLinkProfile called");
    return NULL;
}

void* CreateCMProfileFromColorSyncProfile(void)
{
    if (verbose) puts("STUB: CreateCMProfileFromColorSyncProfile called");
    return NULL;
}

void* CreateColorSyncProfileFromCMProfile(void)
{
    if (verbose) puts("STUB: CreateColorSyncProfileFromCMProfile called");
    return NULL;
}

void* DisposeCMBitmapCallBackUPP(void)
{
    if (verbose) puts("STUB: DisposeCMBitmapCallBackUPP called");
    return NULL;
}

void* DisposeCMConcatCallBackUPP(void)
{
    if (verbose) puts("STUB: DisposeCMConcatCallBackUPP called");
    return NULL;
}

void* DisposeCMFlattenUPP(void)
{
    if (verbose) puts("STUB: DisposeCMFlattenUPP called");
    return NULL;
}

void* DisposeCMMIterateUPP(void)
{
    if (verbose) puts("STUB: DisposeCMMIterateUPP called");
    return NULL;
}

void* DisposeCMProfileAccessUPP(void)
{
    if (verbose) puts("STUB: DisposeCMProfileAccessUPP called");
    return NULL;
}

void* DisposeCMProfileIterateUPP(void)
{
    if (verbose) puts("STUB: DisposeCMProfileIterateUPP called");
    return NULL;
}

void* InvokeCMBitmapCallBackUPP(void)
{
    if (verbose) puts("STUB: InvokeCMBitmapCallBackUPP called");
    return NULL;
}

void* InvokeCMConcatCallBackUPP(void)
{
    if (verbose) puts("STUB: InvokeCMConcatCallBackUPP called");
    return NULL;
}

void* InvokeCMFlattenUPP(void)
{
    if (verbose) puts("STUB: InvokeCMFlattenUPP called");
    return NULL;
}

void* InvokeCMMIterateUPP(void)
{
    if (verbose) puts("STUB: InvokeCMMIterateUPP called");
    return NULL;
}

void* InvokeCMProfileAccessUPP(void)
{
    if (verbose) puts("STUB: InvokeCMProfileAccessUPP called");
    return NULL;
}

void* InvokeCMProfileIterateUPP(void)
{
    if (verbose) puts("STUB: InvokeCMProfileIterateUPP called");
    return NULL;
}

void* NCMGetProfileLocation(void)
{
    if (verbose) puts("STUB: NCMGetProfileLocation called");
    return NULL;
}

void* NCWConcatColorWorld(void)
{
    if (verbose) puts("STUB: NCWConcatColorWorld called");
    return NULL;
}

void* NCWNewColorWorld(void)
{
    if (verbose) puts("STUB: NCWNewColorWorld called");
    return NULL;
}

void* NCWNewLinkProfile(void)
{
    if (verbose) puts("STUB: NCWNewLinkProfile called");
    return NULL;
}

void* NewCMBitmapCallBackUPP(void)
{
    if (verbose) puts("STUB: NewCMBitmapCallBackUPP called");
    return NULL;
}

void* NewCMConcatCallBackUPP(void)
{
    if (verbose) puts("STUB: NewCMConcatCallBackUPP called");
    return NULL;
}

void* NewCMFlattenUPP(void)
{
    if (verbose) puts("STUB: NewCMFlattenUPP called");
    return NULL;
}

void* NewCMMIterateUPP(void)
{
    if (verbose) puts("STUB: NewCMMIterateUPP called");
    return NULL;
}

void* NewCMProfileAccessUPP(void)
{
    if (verbose) puts("STUB: NewCMProfileAccessUPP called");
    return NULL;
}

void* NewCMProfileIterateUPP(void)
{
    if (verbose) puts("STUB: NewCMProfileIterateUPP called");
    return NULL;
}

void* _CMGetProfileOfSuite(void)
{
    if (verbose) puts("STUB: _CMGetProfileOfSuite called");
    return NULL;
}

void* _CMLoadProfileLutsByAVID(void)
{
    if (verbose) puts("STUB: _CMLoadProfileLutsByAVID called");
    return NULL;
}

void* _CMProfileGetProperty(void)
{
    if (verbose) puts("STUB: _CMProfileGetProperty called");
    return NULL;
}

void* _CMProfileSetProperty(void)
{
    if (verbose) puts("STUB: _CMProfileSetProperty called");
    return NULL;
}

void* _CWColorWorldGetProperty(void)
{
    if (verbose) puts("STUB: _CWColorWorldGetProperty called");
    return NULL;
}

void* _CWColorWorldSetProperty(void)
{
    if (verbose) puts("STUB: _CWColorWorldSetProperty called");
    return NULL;
}
