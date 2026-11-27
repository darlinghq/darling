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

#import "DarlingScreenCaptureBackend.h"

#import <CoreMedia/CoreMedia.h>
#import <CoreVideo/CoreVideo.h>
#import <IOSurface/IOSurfaceRef.h>
#import <objc/runtime.h>

#include <stdatomic.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

@implementation AVCaptureInputPort (DarlingScreen)

+ (AVCaptureInputPort *)darling_videoPortForScreenInput:(AVCaptureScreenInput *)input
{
	AVCaptureInputPort *port = [[self alloc] init];

	(void) input;
	objc_setAssociatedObject(port, &kDarlingCameraMediaTypeKey, AVMediaTypeVideo,
		OBJC_ASSOCIATION_RETAIN_NONATOMIC);
	return [port autorelease];
}

@end

@interface DarlingScreenCaptureEngine ()
- (void)captureSurface:(IOSurfaceRef)surface
	displayTime:(uint64_t)displayTime
	droppedFrames:(size_t)droppedFrames;
- (void)deliverSample:(CMSampleBufferRef)sample dropped:(BOOL)dropped;
- (void)finishDelayedSample;
@end

static OSType DarlingScreenOutputFormat(NSDictionary *videoSettings)
{
	NSNumber *number = videoSettings[(NSString *) kCVPixelBufferPixelFormatTypeKey];
	OSType requested = number && [number isKindOfClass: [NSNumber class]]
		? (OSType) [number unsignedIntValue] : kCVPixelFormatType_32BGRA;

	switch (requested)
	{
		case kCVPixelFormatType_32BGRA:
		case kCVPixelFormatType_32RGBA:
		case kCVPixelFormatType_32ARGB:
		case kCVPixelFormatType_32ABGR:
			return requested;
		default:
			return kCVPixelFormatType_32BGRA;
	}
}

static uint64_t DarlingScreenFrameHash(const uint8_t *data, size_t size)
{
	uint64_t hash = UINT64_C(1469598103934665603);
	size_t i;

	for (i = 0; i < size; i++)
	{
		hash ^= data[i];
		hash *= UINT64_C(1099511628211);
	}
	return hash;
}

static int64_t DarlingScreenIntervalNs(CMTime duration)
{
	Float64 seconds = CMTimeGetSeconds(duration);

	if (!(seconds > 0))
		return 0;
	return (int64_t) (seconds * 1000000000.0);
}

/* cropRect uses a documented bottom-left origin. CGDisplayStream source
 * rectangles use the top-left CoreGraphics coordinate space. */
static CGRect DarlingScreenSourceRect(CGRect displayBounds, CGRect cropRect)
{
	CGRect source;

	if (CGRectIsEmpty(cropRect))
		return displayBounds;
	source = CGRectMake(displayBounds.origin.x + cropRect.origin.x,
		displayBounds.origin.y + displayBounds.size.height
			- cropRect.origin.y - cropRect.size.height,
		cropRect.size.width, cropRect.size.height);
	source = CGRectIntersection(source, displayBounds);
	return CGRectIsNull(source) ? CGRectZero : source;
}

static BOOL DarlingScreenCopySurface(const uint8_t *base, uint32_t stride,
	uint32_t width, uint32_t height, uint8_t *destination, uint32_t destinationStride)
{
	uint32_t y;

	if (!base || !destination || !width || !height
		|| stride < width * 4 || destinationStride < width * 4)
		return NO;
	for (y = 0; y < height; y++)
		memcpy(destination + (size_t) y * destinationStride,
			base + (size_t) y * stride, (size_t) width * 4);
	return YES;
}

static CMSampleBufferRef DarlingScreenCreateSampleBuffer(IOSurfaceRef surface,
	int64_t ptsNs, int64_t durationNs, OSType pixelFormat, uint64_t *hashOut)
{
	size_t width = (size_t) IOSurfaceGetWidth(surface);
	size_t height = (size_t) IOSurfaceGetHeight(surface);
	CVPixelBufferRef pixelBuffer = NULL;
	CMVideoFormatDescriptionRef description = NULL;
	CMSampleBufferRef sample = NULL;
	CMSampleTimingInfo timing;
	uint8_t *surfaceBase;
	uint32_t surfaceStride, destinationStride;
	uint8_t *destination;

	if (hashOut)
		*hashOut = 0;
	if (!width || !height || IOSurfaceGetPixelFormat(surface) != pixelFormat)
		return NULL;
	if (CVPixelBufferCreate(NULL, width, height, pixelFormat, NULL, &pixelBuffer)
			!= kCVReturnSuccess)
		return NULL;
	if (CVPixelBufferLockBaseAddress(pixelBuffer, 0) != kCVReturnSuccess)
	{
		CVPixelBufferRelease(pixelBuffer);
		return NULL;
	}
	if (IOSurfaceLock(surface, kIOSurfaceLockReadOnly, NULL) != 0)
	{
		CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
		CVPixelBufferRelease(pixelBuffer);
		return NULL;
	}

	surfaceBase = (uint8_t *) IOSurfaceGetBaseAddress(surface);
	surfaceStride = (uint32_t) IOSurfaceGetBytesPerRow(surface);
	destination = (uint8_t *) CVPixelBufferGetBaseAddress(pixelBuffer);
	destinationStride = (uint32_t) CVPixelBufferGetBytesPerRow(pixelBuffer);
	DarlingScreenCopySurface(surfaceBase, surfaceStride, (uint32_t) width,
		(uint32_t) height, destination, destinationStride);
	if (hashOut)
		*hashOut = DarlingScreenFrameHash(destination,
			(size_t) destinationStride * height);
	IOSurfaceUnlock(surface, kIOSurfaceLockReadOnly, NULL);
	CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);

	if (CMVideoFormatDescriptionCreateForImageBuffer(NULL, pixelBuffer, &description) == noErr)
	{
		memset(&timing, 0, sizeof(timing));
		if (ptsNs >= 0)
			timing.presentationTimeStamp = CMTimeMake(ptsNs, 1000000000);
		else
			timing.presentationTimeStamp = CMClockGetTime(CMClockGetHostTimeClock());
		if (durationNs <= 0)
			durationNs = 33333333;
		timing.duration = CMTimeMake(durationNs, 1000000000);
		CMSampleBufferCreateReadyWithImageBuffer(NULL, pixelBuffer, description, &timing, &sample);
	}
	if (description)
		CFRelease(description);
	CVPixelBufferRelease(pixelBuffer);
	return sample;
}

@implementation DarlingScreenCaptureEngine

- (instancetype)initWithOutput:(AVCaptureVideoDataOutput *)output
	connection:(AVCaptureConnection *)connection
	videoSettings:(NSDictionary *)videoSettings
	alwaysDiscardsLateVideoFrames:(BOOL)alwaysDiscardsLateVideoFrames
	screenInput:(AVCaptureScreenInput *)screenInput
{
	self = [super init];
	if (!self)
		return nil;

	_output = [output retain];
	_connection = [connection retain];
	_videoSettings = [videoSettings copy];
	_alwaysDiscardsLateVideoFrames = alwaysDiscardsLateVideoFrames;
	_screenInput = [screenInput retain];
	atomic_store(&_running, false);
	atomic_store(&_inFlight, 0);
	_lastPtsNs = -1;
	return self;
}

- (void)dealloc
{
	[self stop];
	[_output release];
	[_connection release];
	[_videoSettings release];
	[_screenInput release];
	[super dealloc];
}

- (void)start
{
	CGRect bounds = CGDisplayBounds([_screenInput darling_displayID]);
	CGRect source;
	CGSize outputSize;
	CGFloat scale = [_screenInput scaleFactor];
	CFMutableDictionaryRef properties;
	CGDisplayStreamRef stream;
	DarlingScreenCaptureEngine *engine = self;
	double minimumFrameTime;

	if (_displayStream || CGRectIsEmpty(bounds))
		return;
	source = DarlingScreenSourceRect(bounds, [_screenInput cropRect]);
	if (CGRectIsEmpty(source))
		return;
	if (!(scale > 0))
		scale = 1;
	outputSize = CGSizeMake(source.size.width * scale, source.size.height * scale);
	if (outputSize.width <= 0 || outputSize.height <= 0
		|| outputSize.width > 16384 || outputSize.height > 16384)
		return;

	_captureQueue = dispatch_queue_create("com.darling.avfoundation.screen-capture",
		DISPATCH_QUEUE_SERIAL);
	if (!_captureQueue)
		return;

	properties = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks);
	if (properties)
	{
		int64_t queueDepth = _alwaysDiscardsLateVideoFrames ? 1 : 3;
		CFNumberRef number;

		CFDictionarySetValue(properties, kCGDisplayStreamShowCursor,
			[_screenInput capturesCursor] ? kCFBooleanTrue : kCFBooleanFalse);
		number = CFNumberCreate(NULL, kCFNumberSInt64Type, &queueDepth);
		if (number)
		{
			CFDictionarySetValue(properties, kCGDisplayStreamQueueDepth, number);
			CFRelease(number);
		}
		minimumFrameTime = CMTimeGetSeconds([_screenInput minFrameDuration]);
		if (minimumFrameTime > 0)
		{
			number = CFNumberCreate(NULL, kCFNumberDoubleType, &minimumFrameTime);
			if (number)
			{
				CFDictionarySetValue(properties, kCGDisplayStreamMinimumFrameTime, number);
				CFRelease(number);
			}
		}
		CFDictionarySetValue(properties, kCGDisplayStreamSourceRect,
			[NSValue valueWithRect: source]);
	}

	stream = CGDisplayStreamCreateWithDispatchQueue([_screenInput darling_displayID],
		(size_t) outputSize.width, (size_t) outputSize.height,
		DarlingScreenOutputFormat(_videoSettings), properties, _captureQueue,
		^(CGDisplayStreamFrameStatus status, uint64_t displayTime,
			IOSurfaceRef surface, CGDisplayStreamUpdateRef update)
		{
			if (status == kCGDisplayStreamFrameStatusStopped)
				return;
			if (status != kCGDisplayStreamFrameStatusFrameComplete || surface == NULL)
				return;
			@autoreleasepool
			{
				[engine captureSurface: surface displayTime: displayTime
					droppedFrames: update ? CGDisplayStreamUpdateGetDropCount(update) : 0];
			}
		});
	if (properties)
		CFRelease(properties);
	if (!stream)
	{
		dispatch_release(_captureQueue);
		_captureQueue = NULL;
		return;
	}

	atomic_store(&_running, true);
	if (CGDisplayStreamStart(stream) != kCGErrorSuccess)
	{
		atomic_store(&_running, false);
		CFRelease(stream);
		dispatch_release(_captureQueue);
		_captureQueue = NULL;
		return;
	}
	_displayStream = stream;
}

- (void)stop
{
	CGDisplayStreamRef stream;

	atomic_store(&_running, false);
	stream = _displayStream;
	_displayStream = NULL;
	if (stream)
	{
		CGDisplayStreamStop(stream);
		CFRelease(stream);
	}
	if (_captureQueue)
	{
		dispatch_release(_captureQueue);
		_captureQueue = NULL;
	}
}

- (void)captureSurface:(IOSurfaceRef)surface
	displayTime:(uint64_t)displayTime
	droppedFrames:(size_t)droppedFrames
{
	int64_t minIntervalNs = DarlingScreenIntervalNs([_screenInput minFrameDuration]);
	int64_t durationNs = minIntervalNs > 0 ? minIntervalNs : 33333333;
	int64_t ptsNs = displayTime <= (uint64_t) INT64_MAX ? (int64_t) displayTime : -1;
	uint64_t hash = 0;
	CMSampleBufferRef sample;
	OSType pixelFormat = DarlingScreenOutputFormat(_videoSettings);

	if (!atomic_load(&_running) || ![_connection isEnabled])
		return;
	if (minIntervalNs > 0 && _lastPtsNs >= 0 && ptsNs >= _lastPtsNs
			&& ptsNs - _lastPtsNs < minIntervalNs)
		return;

	sample = DarlingScreenCreateSampleBuffer(surface, ptsNs, durationNs, pixelFormat, &hash);
	if (!sample)
		return;

	if ([_screenInput removesDuplicateFrames] && _hasLastFrameHash && hash == _lastFrameHash)
	{
		CFRelease(sample);
		return;
	}

	_lastFrameHash = hash;
	_hasLastFrameHash = YES;
	_lastPtsNs = ptsNs;

	if (droppedFrames > 0 || (atomic_load(&_inFlight) > 0
			&& _alwaysDiscardsLateVideoFrames))
		[self deliverSample: sample dropped: YES];
	else
		[self deliverSample: sample dropped: NO];
}

- (void)deliverSample:(CMSampleBufferRef)sample dropped:(BOOL)dropped
{
	id delegate = [_output sampleBufferDelegate];
	SEL selector = dropped
		? @selector(captureOutput:didDropSampleBuffer:fromConnection:)
		: @selector(captureOutput:didOutputSampleBuffer:fromConnection:);

	if (!delegate || ![delegate respondsToSelector: selector])
	{
		CFRelease(sample);
		return;
	}

	if ([_output sampleBufferDelegateQueue])
	{
		AVCaptureOutput *output = [_output retain];
		AVCaptureConnection *connection = [_connection retain];
		id strongDelegate = [delegate retain];
		DarlingScreenCaptureEngine *strongEngine = [self retain];

		atomic_fetch_add(&_inFlight, 1);
		CFRetain(sample);
		dispatch_async([_output sampleBufferDelegateQueue], ^{
			if (dropped)
				[strongDelegate captureOutput: output didDropSampleBuffer: sample
					fromConnection: connection];
			else
				[strongDelegate captureOutput: output didOutputSampleBuffer: sample
					fromConnection: connection];
			CFRelease(sample);
			[strongDelegate release];
			[output release];
			[connection release];
			[strongEngine finishDelayedSample];
			[strongEngine release];
		});
		CFRelease(sample);
	}
	else
	{
		atomic_fetch_add(&_inFlight, 1);
		if (dropped)
			[delegate captureOutput: _output didDropSampleBuffer: sample
				fromConnection: _connection];
		else
			[delegate captureOutput: _output didOutputSampleBuffer: sample
				fromConnection: _connection];
		CFRelease(sample);
		atomic_fetch_sub(&_inFlight, 1);
	}
}

- (void)finishDelayedSample
{
	atomic_fetch_sub(&_inFlight, 1);
}

@end
