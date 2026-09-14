/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

#ifndef _ScreenCaptureKit_H_
#define _ScreenCaptureKit_H_

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <CoreMedia/CMTime.h>

// Stub of ScreenCaptureKit. Darling has no screen capture service, so every
// capture request completes with SCStreamErrorUserDeclined, the same result an
// app gets on macOS when screen recording permission is denied.

extern NSString *const SCStreamErrorDomain;

typedef NS_ENUM(NSInteger, SCStreamError) {
	SCStreamErrorUserDeclined = -3801,
	SCStreamErrorFailedToStart = -3802,
	SCStreamErrorMissingEntitlements = -3803,
	SCStreamErrorAttemptToStopStreamState = -3808,
};

typedef NS_ENUM(NSInteger, SCStreamOutputType) {
	SCStreamOutputTypeScreen,
	SCStreamOutputTypeAudio,
	SCStreamOutputTypeMicrophone,
};

@interface SCRunningApplication : NSObject
@property (readonly) NSString *bundleIdentifier;
@property (readonly) NSString *applicationName;
@property (readonly) pid_t processID;
@end

@interface SCWindow : NSObject
@property (readonly) CGWindowID windowID;
@property (readonly) CGRect frame;
@property (readonly) NSString *title;
@property (readonly) NSInteger windowLayer;
@property (readonly) SCRunningApplication *owningApplication;
@property (readonly, getter=isOnScreen) BOOL onScreen;
@end

@interface SCDisplay : NSObject
@property (readonly) CGDirectDisplayID displayID;
@property (readonly) NSInteger width;
@property (readonly) NSInteger height;
@property (readonly) CGRect frame;
@end

@interface SCShareableContent : NSObject
@property (readonly) NSArray<SCWindow *> *windows;
@property (readonly) NSArray<SCDisplay *> *displays;
@property (readonly) NSArray<SCRunningApplication *> *applications;
+ (void)getShareableContentWithCompletionHandler:(void (^)(SCShareableContent *content, NSError *error))completionHandler;
+ (void)getCurrentProcessShareableContentWithCompletionHandler:(void (^)(SCShareableContent *content, NSError *error))completionHandler;
+ (void)getShareableContentExcludingDesktopWindows:(BOOL)excludeDesktopWindows onScreenWindowsOnly:(BOOL)onScreenWindowsOnly completionHandler:(void (^)(SCShareableContent *content, NSError *error))completionHandler;
@end

@interface SCContentFilter : NSObject
@property (readonly) CGRect contentRect;
@property (readonly) CGFloat pointPixelScale;
- (instancetype)initWithDesktopIndependentWindow:(SCWindow *)window;
- (instancetype)initWithDisplay:(SCDisplay *)display excludingWindows:(NSArray<SCWindow *> *)excluded;
- (instancetype)initWithDisplay:(SCDisplay *)display includingWindows:(NSArray<SCWindow *> *)included;
- (instancetype)initWithDisplay:(SCDisplay *)display excludingApplications:(NSArray<SCRunningApplication *> *)applications exceptingWindows:(NSArray<SCWindow *> *)exceptingWindows;
@end

@interface SCStreamConfiguration : NSObject
@property size_t width;
@property size_t height;
@property CMTime minimumFrameInterval;
@property OSType pixelFormat;
@property BOOL scalesToFit;
@property CGRect sourceRect;
@property CGRect destinationRect;
@property NSInteger queueDepth;
@property BOOL showsCursor;
@property BOOL capturesAudio;
@property NSInteger sampleRate;
@property NSInteger channelCount;
@property BOOL excludesCurrentProcessAudio;
@end

@class SCStream;

@protocol SCStreamDelegate <NSObject>
@optional
- (void)stream:(SCStream *)stream didStopWithError:(NSError *)error;
@end

@protocol SCStreamOutput <NSObject>
@end

@class SCRecordingOutput;

@protocol SCRecordingOutputDelegate <NSObject>
@optional
- (void)recordingOutputDidStartRecording:(SCRecordingOutput *)recordingOutput;
- (void)recordingOutput:(SCRecordingOutput *)recordingOutput didFailWithError:(NSError *)error;
- (void)recordingOutputDidFinishRecording:(SCRecordingOutput *)recordingOutput;
@end

@interface SCRecordingOutputConfiguration : NSObject
@property (copy) NSURL *outputURL;
@property (copy) NSString *videoCodecType;
@property (copy) NSString *outputFileType;
@end

@interface SCRecordingOutput : NSObject
- (instancetype)initWithConfiguration:(SCRecordingOutputConfiguration *)recordingOutputConfiguration delegate:(id<SCRecordingOutputDelegate>)delegate;
@end

@interface SCStream : NSObject
- (instancetype)initWithFilter:(SCContentFilter *)contentFilter configuration:(SCStreamConfiguration *)streamConfig delegate:(id<SCStreamDelegate>)delegate;
- (BOOL)addStreamOutput:(id<SCStreamOutput>)output type:(SCStreamOutputType)type sampleHandlerQueue:(dispatch_queue_t)sampleHandlerQueue error:(NSError **)error;
- (BOOL)removeStreamOutput:(id<SCStreamOutput>)output type:(SCStreamOutputType)type error:(NSError **)error;
- (BOOL)addRecordingOutput:(SCRecordingOutput *)recordingOutput error:(NSError **)error;
- (BOOL)removeRecordingOutput:(SCRecordingOutput *)recordingOutput error:(NSError **)error;
- (void)updateConfiguration:(SCStreamConfiguration *)streamConfig completionHandler:(void (^)(NSError *error))completionHandler;
- (void)updateContentFilter:(SCContentFilter *)contentFilter completionHandler:(void (^)(NSError *error))completionHandler;
- (void)startCaptureWithCompletionHandler:(void (^)(NSError *error))completionHandler;
- (void)stopCaptureWithCompletionHandler:(void (^)(NSError *error))completionHandler;
@end

@interface SCScreenshotConfiguration : NSObject
@property NSInteger width;
@property NSInteger height;
@property BOOL showsCursor;
@property CGRect sourceRect;
@property CGRect destinationRect;
@property BOOL ignoreShadows;
@property BOOL ignoreClipping;
@property BOOL includeChildWindows;
@end

@interface SCScreenshotManager : NSObject
+ (void)captureImageWithFilter:(SCContentFilter *)contentFilter configuration:(SCStreamConfiguration *)config completionHandler:(void (^)(CGImageRef image, NSError *error))completionHandler;
+ (void)captureSampleBufferWithFilter:(SCContentFilter *)contentFilter configuration:(SCStreamConfiguration *)config completionHandler:(void (^)(void *sampleBuffer, NSError *error))completionHandler;
+ (void)captureImageInRect:(CGRect)rect completionHandler:(void (^)(CGImageRef image, NSError *error))completionHandler;
+ (void)captureScreenshotWithFilter:(SCContentFilter *)contentFilter configuration:(SCScreenshotConfiguration *)config completionHandler:(void (^)(id output, NSError *error))completionHandler;
+ (void)captureScreenshotWithRect:(CGRect)rect configuration:(SCScreenshotConfiguration *)config completionHandler:(void (^)(id output, NSError *error))completionHandler;
@end

#endif
