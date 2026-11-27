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
#import "../camera/DarlingCameraFrame.h"

#import <CoreMedia/CoreMedia.h>
#import <CoreVideo/CoreVideo.h>
#import <objc/runtime.h>

#include <stdatomic.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#import <IOSurface/IOSurfaceRef.h>

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
- (void)captureSurface:(IOSurfaceRef)surface displayTime:(uint64_t)displayTime;
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
		case kCVPixelFormatType_24RGB:
		case kCVPixelFormatType_24BGR:
		case kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange:
		case kCVPixelFormatType_420YpCbCr8BiPlanarFullRange:
		case kCVPixelFormatType_420YpCbCr8Planar:
			return requested;
		default:
			return kCVPixelFormatType_32BGRA;
	}
}

static enum darling_camera_output_layout DarlingScreenOutputLayout(OSType format)
{
	switch (format)
	{
		case kCVPixelFormatType_32RGBA: return DARLING_CAMERA_OUTPUT_RGBA;
		case kCVPixelFormatType_32ARGB: return DARLING_CAMERA_OUTPUT_ARGB;
		case kCVPixelFormatType_32ABGR: return DARLING_CAMERA_OUTPUT_ABGR;
		case kCVPixelFormatType_24RGB: return DARLING_CAMERA_OUTPUT_RGB24;
		case kCVPixelFormatType_24BGR: return DARLING_CAMERA_OUTPUT_BGR24;
		case kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange: return DARLING_CAMERA_OUTPUT_NV12;
		case kCVPixelFormatType_420YpCbCr8BiPlanarFullRange: return DARLING_CAMERA_OUTPUT_NV12_FULL;
		case kCVPixelFormatType_420YpCbCr8Planar: return DARLING_CAMERA_OUTPUT_I420;
		default: return DARLING_CAMERA_OUTPUT_BGRA;
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

/* cropRect uses the documented bottom-left origin and is relative to the
 * display's own coordinate space; CGDisplayStream rectangles use a top-left
 * origin. An empty crop rectangle means the whole display. */
static CGRect DarlingScreenSourceRect(CGRect displayBounds, CGRect cropRect)
{
	CGRect local = CGRectMake(0, 0, displayBounds.size.width, displayBounds.size.height);

	if (CGRectIsEmpty(cropRect))
		return local;
	cropRect.origin.y = local.size.height - (cropRect.origin.y + cropRect.size.height);
	return CGRectIntersection(cropRect, local);
}

static CMSampleBufferRef DarlingScreenCreateSampleBuffer(
	const uint8_t *bgra, uint32_t bgraStride, uint32_t width, uint32_t height,
	int64_t ptsNs, int64_t durationNs, OSType pixelFormat)
{
	CVPixelBufferRef pixelBuffer = NULL;
	CMVideoFormatDescriptionRef description = NULL;
	CMSampleBufferRef sample = NULL;
	CMSampleTimingInfo timing;
	const uint8_t *srcPlanes[4] = { bgra, NULL, NULL, NULL };
	uint32_t srcStrides[4] = { bgraStride, 0, 0, 0 };
	const uint8_t *dstPlanes[4] = { NULL, NULL, NULL, NULL };
	uint32_t dstStrides[4] = { 0, 0, 0, 0 };
	size_t planeCount, i;

	if (CVPixelBufferCreate(NULL, width, height, pixelFormat, NULL, &pixelBuffer)
			!= kCVReturnSuccess)
		return NULL;
	if (CVPixelBufferLockBaseAddress(pixelBuffer, 0) != kCVReturnSuccess)
	{
		CVPixelBufferRelease(pixelBuffer);
		return NULL;
	}

	planeCount = CVPixelBufferGetPlaneCount(pixelBuffer);
	if (planeCount == 0)
	{
		dstPlanes[0] = CVPixelBufferGetBaseAddress(pixelBuffer);
		dstStrides[0] = (uint32_t) CVPixelBufferGetBytesPerRow(pixelBuffer);
	}
	else if (planeCount <= 4)
	{
		for (i = 0; i < planeCount; i++)
		{
			dstPlanes[i] = CVPixelBufferGetBaseAddressOfPlane(pixelBuffer, i);
			dstStrides[i] = (uint32_t) CVPixelBufferGetBytesPerRowOfPlane(pixelBuffer, i);
		}
	}
	if (dstPlanes[0])
		DarlingCameraConvertFrame(srcPlanes, srcStrides, DARLING_CAMERA_PIXEL_BGRX,
			dstPlanes, dstStrides, DarlingScreenOutputLayout(pixelFormat), width, height);
	CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);

	if (CMVideoFormatDescriptionCreateForImageBuffer(NULL, pixelBuffer, &description) == noErr)
	{
		memset(&timing, 0, sizeof(timing));
		if (ptsNs >= 0)
			timing.presentationTimeStamp = CMTimeMake(ptsNs, 1000000000);
		else
			timing.presentationTimeStamp = CMTimeConvertScale(
				CMClockGetTime(CMClockGetHostTimeClock()), 1000000000,
				kCMTimeRoundingMethod_RoundHalfAwayFromZero);
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
	CGDirectDisplayID display = [_screenInput darling_displayID];
	CGRect bounds = CGDisplayBounds(display);
	CGRect source = DarlingScreenSourceRect(bounds, [_screenInput cropRect]);
	CGFloat scale = [_screenInput scaleFactor];
	CMTime minDuration = [_screenInput minFrameDuration];
	CGDisplayStreamRef displayStream;
	NSMutableDictionary *properties;
	CFDictionaryRef sourceDictionary;
	dispatch_queue_t queue;
	size_t width, height;

	if (_displayStream || CGRectIsEmpty(bounds) || CGRectIsEmpty(source))
		return;
	if (!(scale > 0))
		scale = 1;
	if (source.size.width * scale < 1 || source.size.height * scale < 1
			|| source.size.width * scale > 16384 || source.size.height * scale > 16384)
		return;
	width = (size_t) (source.size.width * scale);
	height = (size_t) (source.size.height * scale);

	properties = [NSMutableDictionary dictionary];
	/* The display stream takes the source rectangle in global display coordinates. */
	sourceDictionary = CGRectCreateDictionaryRepresentation(
		CGRectOffset(source, bounds.origin.x, bounds.origin.y));
	[properties setObject: (id) sourceDictionary forKey: (id) kCGDisplayStreamSourceRect];
	CFRelease(sourceDictionary);
	[properties setObject: [NSNumber numberWithBool: [_screenInput capturesCursor]]
		forKey: (id) kCGDisplayStreamShowCursor];
	if (CMTimeGetSeconds(minDuration) > 0)
		[properties setObject: [NSNumber numberWithDouble: CMTimeGetSeconds(minDuration)]
			forKey: (id) kCGDisplayStreamMinimumFrameTime];

	queue = dispatch_queue_create("com.apple.AVFoundation.screen-capture", DISPATCH_QUEUE_SERIAL);
	/* The handler keeps the engine alive until the stream reports Stopped. */
	displayStream = CGDisplayStreamCreateWithDispatchQueue(display, width, height,
		kCVPixelFormatType_32BGRA, (CFDictionaryRef) properties, queue,
		^(CGDisplayStreamFrameStatus status, uint64_t displayTime, IOSurfaceRef surface,
		  CGDisplayStreamUpdateRef update)
		{
			(void) update;
			if (status == kCGDisplayStreamFrameStatusFrameComplete && surface)
			{
				@autoreleasepool
				{
					[self captureSurface: surface displayTime: displayTime];
				}
			}
		});
	dispatch_release(queue);
	if (!displayStream)
		return;
	if (CGDisplayStreamStart(displayStream) != kCGErrorSuccess)
	{
		CGDisplayStreamStop(displayStream);
		CFRelease(displayStream);
		return;
	}
	_displayStream = displayStream;
	atomic_store(&_running, true);
}

- (void)stop
{
	CGDisplayStreamRef displayStream = _displayStream;

	atomic_store(&_running, false);
	_displayStream = NULL;
	if (displayStream)
	{
		CGDisplayStreamStop(displayStream);
		CFRelease(displayStream);
	}
}

- (void)captureSurface:(IOSurfaceRef)surface displayTime:(uint64_t)displayTime
{
	uint32_t width = (uint32_t) IOSurfaceGetWidth(surface);
	uint32_t height = (uint32_t) IOSurfaceGetHeight(surface);
	CMTime minDuration = [_screenInput minFrameDuration];
	int64_t durationNs = CMTimeGetSeconds(minDuration) > 0
		? (int64_t) (CMTimeGetSeconds(minDuration) * 1000000000.0) : 33333333;
	uint64_t hash;
	CMSampleBufferRef sample;
	OSType pixelFormat;

	if (!atomic_load(&_running) || ![_connection isEnabled] || width == 0 || height == 0)
		return;
	if (IOSurfaceLock(surface, kIOSurfaceLockReadOnly, NULL) != 0)
		return;
	hash = DarlingScreenFrameHash((const uint8_t *) IOSurfaceGetBaseAddress(surface),
		IOSurfaceGetBytesPerRow(surface) * height);
	if (![_screenInput removesDuplicateFrames] || !_hasLastFrameHash || hash != _lastFrameHash)
	{
		_lastFrameHash = hash;
		_hasLastFrameHash = YES;
		pixelFormat = DarlingScreenOutputFormat(_videoSettings);
		sample = DarlingScreenCreateSampleBuffer(
			(const uint8_t *) IOSurfaceGetBaseAddress(surface),
			(uint32_t) IOSurfaceGetBytesPerRow(surface), width, height,
			(int64_t) displayTime, durationNs, pixelFormat);
	}
	else
		sample = NULL;
	IOSurfaceUnlock(surface, kIOSurfaceLockReadOnly, NULL);
	if (!sample)
		return;

	if (atomic_load(&_inFlight) > 0 && _alwaysDiscardsLateVideoFrames)
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
