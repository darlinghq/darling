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
#include <dispatch/dispatch.h>
#include <CoreMedia/CMSampleBuffer.h>
#import <AVFoundation/AVCaptureOutput.h>
#import <AVFoundation/AVCaptureConnection.h>

@protocol AVCaptureVideoDataOutputSampleBufferDelegate <NSObject>
@optional
- (void)captureOutput:(AVCaptureOutput *)captureOutput didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer fromConnection:(AVCaptureConnection *)connection;
- (void)captureOutput:(AVCaptureOutput *)captureOutput didDropSampleBuffer:(CMSampleBufferRef)sampleBuffer fromConnection:(AVCaptureConnection *)connection;
@end

@interface AVCaptureVideoDataOutput : AVCaptureOutput

- (void)setSampleBufferDelegate:(id<AVCaptureVideoDataOutputSampleBufferDelegate>)sampleBufferDelegate queue:(dispatch_queue_t)sampleBufferDelegateQueue;

@property(nonatomic, assign, readonly) id<AVCaptureVideoDataOutputSampleBufferDelegate> sampleBufferDelegate;
@property(nonatomic, readonly) dispatch_queue_t sampleBufferDelegateQueue;
@property(nonatomic, copy) NSDictionary<NSString *, id> *videoSettings;
@property(nonatomic, getter=isAlwaysDiscardsLateVideoFrames) BOOL alwaysDiscardsLateVideoFrames;

@end
