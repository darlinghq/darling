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


#ifndef _ColorSyncLegacy_H_
#define _ColorSyncLegacy_H_

#include <CoreServices/CoreServices.h>

#ifdef __cplusplus
extern "C" {
#endif

// The legacy ColorSync profile reference. It is a ColorSync profile (the reference of the
// ColorSyncProfile calls of the ColorSync framework).
typedef struct OpaqueCMProfileRef* CMProfileRef;

#pragma pack(push, 2)
enum {
	cmNoProfileBase = 0,
	cmFileBasedProfile = 1,
	cmHandleBasedProfile = 2,
	cmPtrBasedProfile = 3,
	cmProcedureBasedProfile = 4,
	cmPathBasedProfile = 5,
	cmBufferBasedProfile = 6
};

struct CMFileLocation { FSSpec spec; };
struct CMHandleLocation { Handle h; };
struct CMPtrLocation { Ptr p; };
struct CMPathLocation { char path[255]; };
struct CMBufferLocation { void* buffer; UInt32 size; };
typedef struct CMFileLocation CMFileLocation;
typedef struct CMHandleLocation CMHandleLocation;
typedef struct CMPtrLocation CMPtrLocation;
typedef struct CMPathLocation CMPathLocation;
typedef struct CMBufferLocation CMBufferLocation;

union CMProfLoc {
	CMFileLocation fileLoc;
	CMHandleLocation handleLoc;
	CMPtrLocation ptrLoc;
	CMPathLocation pathLoc;
	CMBufferLocation bufferLoc;
};
typedef union CMProfLoc CMProfLoc;

struct CMProfileLocation {
	short locType;
	CMProfLoc u;
};
typedef struct CMProfileLocation CMProfileLocation;
#pragma pack(pop)

// Opens the profile at the location (a file given by its POSIX path, or the profile's data in
// memory) and returns a reference that has to be closed with CMCloseProfile()
OSErr CMOpenProfile(CMProfileRef* prof, const CMProfileLocation* theProfile);
// Closes one reference of the profile. CMCloneProfileRef() adds one.
OSErr CMCloseProfile(CMProfileRef prof);
void CMCloneProfileRef(CMProfileRef prof);

void* CMConvertDoubleXYZToD50(void);
void* CMConvertRGBFloatBitmap(void);
void* CMConvertXYZFloatBitmap(void);
void* CMCopyProfile(void);
void* CMCopyProfileDescriptionString(void);
void* CMCopyProfileLocalizedString(void);
void* CMCopyProfileLocalizedStringDictionary(void);
void* CMCountProfileElements(void);
void* CMFloatBitmapMakeChunky(void);
void* CMGetColorSyncVersion(void);
void* CMGetDefaultDevice(void);
void* CMGetDefaultProfileBySpace(void);
void* CMGetDefaultProfileByUse(void);
void* CMGetDeviceDefaultProfileID(void);
void* CMGetDeviceFactoryProfiles(void);
void* CMGetDeviceInfo(void);
void* CMGetDeviceProfile(void);
void* CMGetDeviceState(void);
void* CMGetDeviceTrackingByUse(void);
void* CMGetGammaByAVID(void);
void* CMGetIndNamedColorValue(void);
void* CMGetIndProfileElement(void);
void* CMGetIndProfileElementInfo(void);
void* CMGetNamedColorIndex(void);
void* CMGetNamedColorInfo(void);
void* CMGetNamedColorName(void);
void* CMGetNamedColorValue(void);
void* CMGetPS2ColorRendering(void);
void* CMGetPS2ColorRenderingIntent(void);
void* CMGetPS2ColorRenderingVMSize(void);
void* CMGetPS2ColorSpace(void);
void* CMGetPartialProfileElement(void);
void* CMGetProfileByAVID(void);
void* CMGetProfileDescriptions(void);
void* CMGetProfileElement(void);
void* CMGetProfileGamma(void);
void* CMGetProfileHeader(void);
void* CMGetProfileLocation(void);
void* CMGetProfileMD5(void);
void* CMGetProfileRefCount(void);
void* CMGetProfileTransformInfo(void);
void* CMGetSystemProfile(void);
void* CMGetSystemProfileAVID(void);
void* CMGetSystemProfilePriv(void);
void* CMIterateCMMInfo(void);
void* CMIterateColorDevices(void);
void* CMIterateColorSyncFolder(void);
void* CMIterateDeviceProfiles(void);
void* CMLaunchControlPanel(void);
void* CMMakeProfile(void);
void* CMMatchFloatBitmap(void);
void* CMNewProfile(void);
void* CMPostLegacyDeviceNotification(void);
void* CMProfileCopyICCData(void);
void* CMProfileElementExists(void);
void* CMProfileModified(void);
void* CMProfileWriteToFile(void);
void* CMRegisterColorDevice(void);
void* CMRemoveProfileElement(void);
void* CMSetDefaultDevice(void);
void* CMSetDeviceDefaultProfileID(void);
void* CMSetDeviceFactoryProfiles(void);
void* CMSetDeviceProfile(void);
void* CMSetDeviceState(void);
void* CMSetDeviceTrackingByUse(void);
void* CMSetGammaByAVID(void);
void* CMSetPartialProfileElement(void);
void* CMSetProfileByAVID(void);
void* CMSetProfileDescriptions(void);
void* CMSetProfileElement(void);
void* CMSetProfileElementReference(void);
void* CMSetProfileElementSize(void);
void* CMSetProfileHeader(void);
void* CMSetProfileLocalizedStringDictionary(void);
void* CMSetSystemProfile(void);
void* CMSetSystemProfileAVID(void);
void* CMUnregisterColorDevice(void);
void* CMUpdateProfile(void);
void* CMValidateProfile(void);
void* CWCheckBitmap(void);
void* CWCheckColors(void);
void* CWColorWorldGetProperty(void);
void* CWColorWorldSetProperty(void);
void* CWConcatColorWorld(void);
void* CWDisposeColorWorld(void);
void* CWFillLookupTexture(void);
void* CWGetCMMSignature(void);
void* CWMatchBitmap(void);
void* CWMatchColors(void);
void* CWNewLinkProfile(void);
void* CreateCMProfileFromColorSyncProfile(void);
void* CreateColorSyncProfileFromCMProfile(void);
void* DisposeCMBitmapCallBackUPP(void);
void* DisposeCMConcatCallBackUPP(void);
void* DisposeCMFlattenUPP(void);
void* DisposeCMMIterateUPP(void);
void* DisposeCMProfileAccessUPP(void);
void* DisposeCMProfileIterateUPP(void);
void* InvokeCMBitmapCallBackUPP(void);
void* InvokeCMConcatCallBackUPP(void);
void* InvokeCMFlattenUPP(void);
void* InvokeCMMIterateUPP(void);
void* InvokeCMProfileAccessUPP(void);
void* InvokeCMProfileIterateUPP(void);
void* NCMGetProfileLocation(void);
void* NCWConcatColorWorld(void);
void* NCWNewColorWorld(void);
void* NCWNewLinkProfile(void);
void* NewCMBitmapCallBackUPP(void);
void* NewCMConcatCallBackUPP(void);
void* NewCMFlattenUPP(void);
void* NewCMMIterateUPP(void);
void* NewCMProfileAccessUPP(void);
void* NewCMProfileIterateUPP(void);
void* _CMGetProfileOfSuite(void);
void* _CMLoadProfileLutsByAVID(void);
void* _CMProfileGetProperty(void);
void* _CMProfileSetProperty(void);
void* _CWColorWorldGetProperty(void);
void* _CWColorWorldSetProperty(void);

#ifdef __cplusplus
}
#endif

#endif
