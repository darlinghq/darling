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


#ifndef _ImageCaptureCore_H_
#define _ImageCaptureCore_H_

#import <Foundation/Foundation.h>

#import <ImageCaptureCore/ImageCaptureProtocol.h>
#import <ImageCaptureCore/ICScannerFeatureProps.h>
#import <ImageCaptureCore/ICScannerFeatureEnumeration.h>
#import <ImageCaptureCore/ICScannerFeatureBooleanProps.h>
#import <ImageCaptureCore/ICScannerBandData.h>
#import <ImageCaptureCore/ICDeviceBrowser.h>
#import <ImageCaptureCore/ICScannerFunctionalUnitTransparencyProps.h>
#import <ImageCaptureCore/ICScannerFunctionalUnitFlatbedProps.h>
#import <ImageCaptureCore/ICCallbackInfo.h>
#import <ImageCaptureCore/ICCommand.h>
#import <ImageCaptureCore/ICCommandCenter.h>
#import <ImageCaptureCore/ICScannerFeatureEnumerationProps.h>
#import <ImageCaptureCore/ICScannerFeatureTemplateProps.h>
#import <ImageCaptureCore/ICDeviceBrowserPrivateData.h>
#import <ImageCaptureCore/ICScannerImageRep.h>
#import <ImageCaptureCore/ICClientManager.h>
#import <ImageCaptureCore/ICClient.h>
#import <ImageCaptureCore/ICScannerFunctionalUnitDocumentFeederProps.h>
#import <ImageCaptureCore/ICScannerFeatureRange.h>
#import <ImageCaptureCore/ICDevice.h>
#import <ImageCaptureCore/ICCameraDevice.h>
#import <ImageCaptureCore/ICCameraFile.h>
#import <ImageCaptureCore/ICAccessManager.h>
#import <ImageCaptureCore/ICCameraFolder.h>
#import <ImageCaptureCore/ICCameraItem.h>
#import <ImageCaptureCore/ICScannerFeatureBoolean.h>
#import <ImageCaptureCore/ICScannerFunctionalUnit.h>
#import <ImageCaptureCore/ICScannerFeatureRangeProps.h>
#import <ImageCaptureCore/ICScannerFunctionalUnitFlatbed.h>
#import <ImageCaptureCore/ICScannerFunctionalUnitPositiveTransparency.h>
#import <ImageCaptureCore/ICScannerFunctionalUnitNegativeTransparency.h>
#import <ImageCaptureCore/ICScannerFeature.h>
#import <ImageCaptureCore/ICScannerFeatureTemplate.h>
#import <ImageCaptureCore/ICScannerFunctionalUnitDocumentFeeder.h>
#import <ImageCaptureCore/ICDeviceAccessManager.h>
#import <ImageCaptureCore/ICMasterDeviceBrowser.h>
#import <ImageCaptureCore/ICScannerFunctionalUnitProps.h>
#import <ImageCaptureCore/ICScannerProperties.h>
#import <ImageCaptureCore/ICScannerDevice.h>

void* _ICCreateCGColorSpace(void);
void* _ICCreateDeviceMatchingDeviceID(void);
void* _ICDidFindLegacyDevice(void);
void* _ICDidInstallNewImageCaptureSoftware(void);
void* _ICDidRemoveLegacyDevice(void);
void* _ICImageCaptureGetScannerButtonMappingPreference(void);
void* _ICImageCaptureSetScannerButtonMappingPreference(void);
void* _ICInitialize(void);
void* _ICIsValidDevice(void);
void* _ICSetSharingPreferences(void);
void* _ICStartReceivingSharingPreferences(void);
void* __ICAddDecodedErrorToDict(void);
void* __ICLogDateString(void);
void* __ICLogTypeEnabled(void);
void* __ICLongValueToErrorString(void);
void* __ICOSLogCreate(void);
void* __ICReadPrefs(void);
void* __ICValueToErrorString(void);

#endif
