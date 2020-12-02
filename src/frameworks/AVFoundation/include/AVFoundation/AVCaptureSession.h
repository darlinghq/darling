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

extern NSString *const AVCaptureDeviceWasConnectedNotification;
extern NSString *const AVCaptureDeviceWasDisconnectedNotification;
extern NSString *const AVCaptureSessionDidStartRunningNotification;
extern NSString *const AVCaptureSessionDidStopRunningNotification;
extern NSString *const AVCaptureSessionErrorKey;
extern NSString *const AVCaptureSessionRuntimeErrorNotification;

extern const AVCaptureSessionPreset AVCaptureSessionPreset3840x2160;
extern const AVCaptureSessionPreset AVCaptureSessionPreset1920x1080;
extern const AVCaptureSessionPreset AVCaptureSessionPreset1280x720;
extern const AVCaptureSessionPreset AVCaptureSessionPreset960x540;
extern const AVCaptureSessionPreset AVCaptureSessionPreset640x480;
extern const AVCaptureSessionPreset AVCaptureSessionPreset352x288;
extern const AVCaptureSessionPreset AVCaptureSessionPreset320x240;
extern const AVCaptureSessionPreset AVCaptureSessionPresetiFrame1280x720;
extern const AVCaptureSessionPreset AVCaptureSessionPresetiFrame960x540;
extern const AVCaptureSessionPreset AVCaptureSessionPresetHigh;
extern const AVCaptureSessionPreset AVCaptureSessionPresetLow;
extern const AVCaptureSessionPreset AVCaptureSessionPresetMedium;

@interface AVCaptureSession : NSObject

@end
