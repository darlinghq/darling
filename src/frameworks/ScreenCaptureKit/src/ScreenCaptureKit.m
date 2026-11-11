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

#import <ScreenCaptureKit/ScreenCaptureKit.h>
#include <dispatch/dispatch.h>

NSString *const SCStreamErrorDomain = @"com.apple.ScreenCaptureKit.SCStreamErrorDomain";

static NSError *captureUnavailableError(void)
{
	return [NSError errorWithDomain:SCStreamErrorDomain
	                           code:SCStreamErrorUserDeclined
	                       userInfo:@{ NSLocalizedDescriptionKey: @"Screen capture is not available in Darling" }];
}

// Completion handlers run asynchronously, as they do on macOS.
static void completeLater(void (^handler)(id, NSError *))
{
	if (handler == nil)
		return;
	handler = [handler copy];
	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
		handler(nil, captureUnavailableError());
		[handler release];
	});
}

static void completeErrorLater(void (^handler)(NSError *))
{
	if (handler == nil)
		return;
	handler = [handler copy];
	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
		handler(captureUnavailableError());
		[handler release];
	});
}

static BOOL failWithError(NSError **error)
{
	if (error != NULL)
		*error = captureUnavailableError();
	return NO;
}

@implementation SCRunningApplication
@synthesize bundleIdentifier = _bundleIdentifier, applicationName = _applicationName, processID = _processID;
@end

@implementation SCWindow
@synthesize windowID = _windowID, frame = _frame, title = _title, windowLayer = _windowLayer,
	owningApplication = _owningApplication, onScreen = _onScreen;
@end

@implementation SCDisplay
@synthesize displayID = _displayID, width = _width, height = _height, frame = _frame;
@end

@implementation SCShareableContent

- (NSArray *)windows { return @[]; }
- (NSArray *)displays { return @[]; }
- (NSArray *)applications { return @[]; }

+ (void)getShareableContentWithCompletionHandler:(void (^)(SCShareableContent *, NSError *))completionHandler
{
	completeLater((void (^)(id, NSError *))completionHandler);
}

+ (void)getCurrentProcessShareableContentWithCompletionHandler:(void (^)(SCShareableContent *, NSError *))completionHandler
{
	completeLater((void (^)(id, NSError *))completionHandler);
}

+ (void)getShareableContentExcludingDesktopWindows:(BOOL)excludeDesktopWindows onScreenWindowsOnly:(BOOL)onScreenWindowsOnly completionHandler:(void (^)(SCShareableContent *, NSError *))completionHandler
{
	completeLater((void (^)(id, NSError *))completionHandler);
}

@end

@implementation SCContentFilter

@synthesize contentRect = _contentRect, pointPixelScale = _pointPixelScale, style = _style, streamType = _streamType,
	includeMenuBar = _includeMenuBar;

- (instancetype)initWithDesktopIndependentWindow:(SCWindow *)window
{
	return [super init];
}

- (instancetype)initWithDisplay:(SCDisplay *)display excludingWindows:(NSArray *)excluded
{
	return [super init];
}

- (instancetype)initWithDisplay:(SCDisplay *)display includingWindows:(NSArray *)included
{
	return [super init];
}

- (instancetype)initWithDisplay:(SCDisplay *)display excludingApplications:(NSArray *)applications exceptingWindows:(NSArray *)exceptingWindows
{
	return [super init];
}

@end

@implementation SCStreamConfiguration
@synthesize width = _width, height = _height, minimumFrameInterval = _minimumFrameInterval, pixelFormat = _pixelFormat,
	scalesToFit = _scalesToFit, sourceRect = _sourceRect, destinationRect = _destinationRect, queueDepth = _queueDepth,
	showsCursor = _showsCursor, capturesAudio = _capturesAudio, sampleRate = _sampleRate, channelCount = _channelCount,
	excludesCurrentProcessAudio = _excludesCurrentProcessAudio, streamName = _streamName, backgroundColor = _backgroundColor,
	colorSpaceName = _colorSpaceName, colorMatrix = _colorMatrix, captureResolution = _captureResolution,
	captureDynamicRange = _captureDynamicRange, preservesAspectRatio = _preservesAspectRatio, shouldBeOpaque = _shouldBeOpaque,
	ignoreShadowsDisplay = _ignoreShadowsDisplay, ignoreShadowsSingleWindow = _ignoreShadowsSingleWindow,
	ignoreGlobalClipDisplay = _ignoreGlobalClipDisplay, ignoreGlobalClipSingleWindow = _ignoreGlobalClipSingleWindow,
	includeChildWindows = _includeChildWindows, captureMicrophone = _captureMicrophone,
	microphoneCaptureDeviceID = _microphoneCaptureDeviceID, showMouseClicks = _showMouseClicks;

- (void)dealloc
{
	[_streamName release];
	[_microphoneCaptureDeviceID release];
	[super dealloc];
}

@end

@implementation SCRecordingOutputConfiguration

@synthesize outputURL = _outputURL, videoCodecType = _videoCodecType, outputFileType = _outputFileType;

- (void)dealloc
{
	[_outputURL release];
	[_videoCodecType release];
	[_outputFileType release];
	[super dealloc];
}

@end

@implementation SCRecordingOutput

- (instancetype)initWithConfiguration:(SCRecordingOutputConfiguration *)recordingOutputConfiguration delegate:(id<SCRecordingOutputDelegate>)delegate
{
	return [super init];
}

@end

@implementation SCStream

- (instancetype)initWithFilter:(SCContentFilter *)contentFilter configuration:(SCStreamConfiguration *)streamConfig delegate:(id<SCStreamDelegate>)delegate
{
	return [super init];
}

- (BOOL)addStreamOutput:(id<SCStreamOutput>)output type:(SCStreamOutputType)type sampleHandlerQueue:(dispatch_queue_t)sampleHandlerQueue error:(NSError **)error
{
	return failWithError(error);
}

- (BOOL)removeStreamOutput:(id<SCStreamOutput>)output type:(SCStreamOutputType)type error:(NSError **)error
{
	return failWithError(error);
}

- (BOOL)addRecordingOutput:(SCRecordingOutput *)recordingOutput error:(NSError **)error
{
	return failWithError(error);
}

- (BOOL)removeRecordingOutput:(SCRecordingOutput *)recordingOutput error:(NSError **)error
{
	return failWithError(error);
}

- (void)updateConfiguration:(SCStreamConfiguration *)streamConfig completionHandler:(void (^)(NSError *))completionHandler
{
	completeErrorLater(completionHandler);
}

- (void)updateContentFilter:(SCContentFilter *)contentFilter completionHandler:(void (^)(NSError *))completionHandler
{
	completeErrorLater(completionHandler);
}

- (void)startCaptureWithCompletionHandler:(void (^)(NSError *))completionHandler
{
	completeErrorLater(completionHandler);
}

- (void)stopCaptureWithCompletionHandler:(void (^)(NSError *))completionHandler
{
	completeErrorLater(completionHandler);
}

@end

@implementation SCScreenshotConfiguration
@synthesize width = _width, height = _height, showsCursor = _showsCursor, sourceRect = _sourceRect,
	destinationRect = _destinationRect, ignoreShadows = _ignoreShadows, ignoreClipping = _ignoreClipping,
	includeChildWindows = _includeChildWindows, displayIntent = _displayIntent, dynamicRange = _dynamicRange,
	contentType = _contentType, fileURL = _fileURL;

- (void)dealloc
{
	[_contentType release];
	[_fileURL release];
	[super dealloc];
}

@end

// Never created by the stub (screenshots always fail), but apps reference its accessors.
@implementation SCScreenshotOutput
@synthesize sdrImage = _sdrImage, hdrImage = _hdrImage, fileURL = _fileURL;
@end

@implementation SCScreenshotManager

+ (void)captureImageWithFilter:(SCContentFilter *)contentFilter configuration:(SCStreamConfiguration *)config completionHandler:(void (^)(CGImageRef, NSError *))completionHandler
{
	completeLater((void (^)(id, NSError *))completionHandler);
}

+ (void)captureSampleBufferWithFilter:(SCContentFilter *)contentFilter configuration:(SCStreamConfiguration *)config completionHandler:(void (^)(void *, NSError *))completionHandler
{
	completeLater((void (^)(id, NSError *))completionHandler);
}

+ (void)captureImageInRect:(CGRect)rect completionHandler:(void (^)(CGImageRef, NSError *))completionHandler
{
	completeLater((void (^)(id, NSError *))completionHandler);
}

+ (void)captureScreenshotWithFilter:(SCContentFilter *)contentFilter configuration:(SCScreenshotConfiguration *)config completionHandler:(void (^)(id, NSError *))completionHandler
{
	completeLater(completionHandler);
}

+ (void)captureScreenshotWithRect:(CGRect)rect configuration:(SCScreenshotConfiguration *)config completionHandler:(void (^)(id, NSError *))completionHandler
{
	completeLater(completionHandler);
}

@end
