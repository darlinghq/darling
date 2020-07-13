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

typedef NSString *AVCaptureSessionPreset;

NSString *const AVCaptureDeviceWasConnectedNotification;
NSString *const AVCaptureDeviceWasDisconnectedNotification;
NSString *const AVCaptureSessionDidStartRunningNotification;
NSString *const AVCaptureSessionDidStopRunningNotification;
NSString *const AVCaptureSessionErrorKey;
NSString *const AVCaptureSessionRuntimeErrorNotification;

const AVCaptureSessionPreset AVCaptureSessionPreset3840x2160;
const AVCaptureSessionPreset AVCaptureSessionPreset1920x1080;
const AVCaptureSessionPreset AVCaptureSessionPreset1280x720;
const AVCaptureSessionPreset AVCaptureSessionPreset960x540;
const AVCaptureSessionPreset AVCaptureSessionPreset640x480;
const AVCaptureSessionPreset AVCaptureSessionPreset352x288;
const AVCaptureSessionPreset AVCaptureSessionPreset320x240;
const AVCaptureSessionPreset AVCaptureSessionPresetiFrame1280x720;
const AVCaptureSessionPreset AVCaptureSessionPresetiFrame960x540;
const AVCaptureSessionPreset AVCaptureSessionPresetHigh;
const AVCaptureSessionPreset AVCaptureSessionPresetLow;
const AVCaptureSessionPreset AVCaptureSessionPresetMedium;

@interface AVCaptureSession : NSObject

@end
