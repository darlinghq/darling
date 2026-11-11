/*
 This file is part of Darling.

 Copyright (C) 2017 Lubos Dolezel

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

#include <Foundation/Foundation.h>
#include <AVFoundation/AVMediaFormat.h>

typedef NS_ENUM(NSInteger, AVAuthorizationStatus) {
	AVAuthorizationStatusNotDetermined = 0,
	AVAuthorizationStatusRestricted = 1,
	AVAuthorizationStatusDenied = 2,
	AVAuthorizationStatusAuthorized = 3,
};

extern NSString *const AVCaptureDeviceTypeBuiltInWideAngleCamera;
extern NSString *const AVCaptureDeviceTypeBuiltInTelephotoCamera;
extern NSString *const AVCaptureDeviceTypeBuiltInUltraWideCamera;
extern NSString *const AVCaptureDeviceTypeBuiltInDualCamera;
extern NSString *const AVCaptureDeviceTypeBuiltInDualWideCamera;
extern NSString *const AVCaptureDeviceTypeBuiltInTripleCamera;
extern NSString *const AVCaptureDeviceTypeBuiltInTrueDepthCamera;
extern NSString *const AVCaptureDeviceTypeBuiltInLiDARDepthCamera;
extern NSString *const AVCaptureDeviceTypeBuiltInMicrophone;
extern NSString *const AVCaptureDeviceTypeMicrophone;
extern NSString *const AVCaptureDeviceTypeExternalUnknown;
extern NSString *const AVCaptureDeviceTypeExternal;
extern NSString *const AVCaptureDeviceTypeContinuityCamera;
extern NSString *const AVCaptureDeviceTypeDeskViewCamera;

@interface AVCaptureDevice : NSObject

+ (NSArray*)devices;
+ (NSArray*)devicesWithMediaType:(AVMediaType)mediaType;
+ (AVCaptureDevice*)defaultDeviceWithMediaType:(AVMediaType)mediaType;

+ (AVAuthorizationStatus)authorizationStatusForMediaType:(AVMediaType)mediaType;
+ (void)requestAccessForMediaType:(AVMediaType)mediaType completionHandler:(void (^)(BOOL granted))handler;

@end
