/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Darling developers
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef DARLING_AV_SCREEN_CAPTURE_BACKEND_H_
#define DARLING_AV_SCREEN_CAPTURE_BACKEND_H_

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>

#import <AVFoundation/AVCaptureConnection.h>
#import <AVFoundation/AVCaptureInputPort.h>
#import <AVFoundation/AVCaptureOutput.h>
#import <AVFoundation/AVMediaFormat.h>
#import <AVFoundation/AVCaptureScreenInput.h>
#import <AVFoundation/AVCaptureVideoDataOutput.h>

#include <stdatomic.h>
#include <stdint.h>

#import <CoreGraphics/CGDisplayStream.h>

/* Defined by the shared AVCapture port/camera plumbing. */
extern const char kDarlingCameraPortsKey;
extern const char kDarlingCameraMediaTypeKey;

@interface AVCaptureInputPort (DarlingScreen)
+ (AVCaptureInputPort *)darling_videoPortForScreenInput:(AVCaptureScreenInput *)input;
@end

@interface AVCaptureScreenInput (DarlingScreen)
- (CGDirectDisplayID)darling_displayID;
@end

/* Owns one CGDisplayStream and delivers its frames to a video data
 * output. The session owns the engine and stops it before releasing it. */
@interface DarlingScreenCaptureEngine : NSObject
{
	AVCaptureVideoDataOutput *_output;
	AVCaptureConnection *_connection;
	NSDictionary *_videoSettings;
	BOOL _alwaysDiscardsLateVideoFrames;
	AVCaptureScreenInput *_screenInput;
	CGDisplayStreamRef _displayStream;
	atomic_bool _running;
	atomic_int _inFlight;
	uint64_t _lastFrameHash;
	BOOL _hasLastFrameHash;
}
- (instancetype)initWithOutput:(AVCaptureVideoDataOutput *)output
	connection:(AVCaptureConnection *)connection
	videoSettings:(NSDictionary *)videoSettings
	alwaysDiscardsLateVideoFrames:(BOOL)alwaysDiscardsLateVideoFrames
	screenInput:(AVCaptureScreenInput *)screenInput;
- (void)start;
- (void)stop;
@end

#endif /* DARLING_AV_SCREEN_CAPTURE_BACKEND_H_ */
