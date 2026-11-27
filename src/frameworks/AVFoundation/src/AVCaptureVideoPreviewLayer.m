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

#import <AVFoundation/AVCaptureVideoPreviewLayer.h>
#import <AVFoundation/AVCaptureVideoDataOutput.h>
#import <AVFoundation/AVAnimation.h>
#import <CoreVideo/CoreVideo.h>
#import <CoreGraphics/CoreGraphics.h>
#import <QuartzCore/CATransaction.h>
#import <objc/runtime.h>

@interface DarlingVideoPreviewLayerState : NSObject
{
@public
	AVCaptureSession *m_session;
	AVCaptureVideoDataOutput *m_output;
	dispatch_queue_t m_queue;
	NSString *m_videoGravity;
}
@end

@implementation DarlingVideoPreviewLayerState
@end

static char kDarlingVideoPreviewLayerStateKey;

static DarlingVideoPreviewLayerState *DarlingVideoPreviewLayerStateForLayer(AVCaptureVideoPreviewLayer *layer)
{
	DarlingVideoPreviewLayerState *state = objc_getAssociatedObject(layer, &kDarlingVideoPreviewLayerStateKey);
	if (!state)
	{
		state = [[DarlingVideoPreviewLayerState alloc] init];
		objc_setAssociatedObject(layer, &kDarlingVideoPreviewLayerStateKey, state,
			OBJC_ASSOCIATION_RETAIN_NONATOMIC);
		[state release];
	}
	return state;
}

@interface AVCaptureVideoPreviewLayer () <AVCaptureVideoDataOutputSampleBufferDelegate>
@end

static CGBitmapInfo previewBitmapInfoForPixelFormat(OSType format)
{
	switch (format)
	{
		case kCVPixelFormatType_32BGRA: return kCGBitmapByteOrder32Little | kCGImageAlphaNoneSkipFirst;
		case kCVPixelFormatType_32ARGB: return kCGBitmapByteOrder32Big | kCGImageAlphaNoneSkipFirst;
		case kCVPixelFormatType_32RGBA: return kCGBitmapByteOrder32Big | kCGImageAlphaNoneSkipLast;
		case kCVPixelFormatType_32ABGR: return kCGBitmapByteOrder32Little | kCGImageAlphaNoneSkipLast;
		default: return 0;
	}
}

static CGImageRef createPreviewImageFromPixelBuffer(CVPixelBufferRef buffer)
{
	CGBitmapInfo info = previewBitmapInfoForPixelFormat(CVPixelBufferGetPixelFormatType(buffer));
	if (info == 0 || CVPixelBufferLockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly) != kCVReturnSuccess)
		return NULL;
	size_t width = CVPixelBufferGetWidth(buffer);
	size_t height = CVPixelBufferGetHeight(buffer);
	size_t rowBytes = CVPixelBufferGetBytesPerRow(buffer);
	CFDataRef data = CFDataCreate(NULL, CVPixelBufferGetBaseAddress(buffer), rowBytes * height);
	CVPixelBufferUnlockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly);
	if (data == NULL)
		return NULL;
	CGDataProviderRef provider = CGDataProviderCreateWithCFData(data);
	CFRelease(data);
	CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
	CGImageRef image = CGImageCreate(width, height, 8, 32, rowBytes, space, info, provider, NULL, false, kCGRenderingIntentDefault);
	CGColorSpaceRelease(space);
	CGDataProviderRelease(provider);
	return image;
}

@implementation AVCaptureVideoPreviewLayer

+ (instancetype)layerWithSession:(AVCaptureSession *)session
{
	return [[[self alloc] initWithSession: session] autorelease];
}

- (instancetype)initWithSession:(AVCaptureSession *)session
{
	self = [super init];
	if (self)
	{
		DarlingVideoPreviewLayerState *state = DarlingVideoPreviewLayerStateForLayer(self);
		state->m_videoGravity = [AVLayerVideoGravityResizeAspect copy];
		self.contentsGravity = kCAGravityResizeAspect;
		state->m_queue = dispatch_queue_create("org.darlinghq.avfoundation.preview", DISPATCH_QUEUE_SERIAL);
		[self setSession: session];
	}
	return self;
}

- (void)dealloc
{
	[self setSession: nil];
	DarlingVideoPreviewLayerState *state = objc_getAssociatedObject(self, &kDarlingVideoPreviewLayerStateKey);
	if (state)
	{
		[state->m_videoGravity release];
		if (state->m_queue)
			dispatch_release(state->m_queue);
	}
	objc_setAssociatedObject(self, &kDarlingVideoPreviewLayerStateKey, nil,
		OBJC_ASSOCIATION_RETAIN_NONATOMIC);
	[super dealloc];
}

- (AVCaptureSession *)session
{
	return DarlingVideoPreviewLayerStateForLayer(self)->m_session;
}

- (void)setSession:(AVCaptureSession *)session
{
	DarlingVideoPreviewLayerState *state = DarlingVideoPreviewLayerStateForLayer(self);
	if (session == state->m_session)
		return;

	if (state->m_session && state->m_output)
	{
		[state->m_session removeOutput: state->m_output];
		[state->m_output setSampleBufferDelegate: nil queue: NULL];
		[state->m_output release];
		state->m_output = nil;
	}

	[state->m_session autorelease];
	state->m_session = [session retain];

	if (state->m_session)
	{
		state->m_output = [[AVCaptureVideoDataOutput alloc] init];
		[state->m_output setSampleBufferDelegate: self queue: state->m_queue];
		if ([state->m_session canAddOutput: state->m_output])
			[state->m_session addOutput: state->m_output];
	}
}

/* Connections are created when the session starts, so read them from the
 * output instead of caching a possibly nil connection here. */
- (AVCaptureConnection *)connection
{
	return [[DarlingVideoPreviewLayerStateForLayer(self)->m_output connections] firstObject];
}

- (NSString *)videoGravity
{
	return DarlingVideoPreviewLayerStateForLayer(self)->m_videoGravity;
}

- (void)setVideoGravity:(NSString *)videoGravity
{
	DarlingVideoPreviewLayerState *state = DarlingVideoPreviewLayerStateForLayer(self);
	if (!videoGravity || [videoGravity isEqualToString: state->m_videoGravity])
		return;
	[state->m_videoGravity release];
	state->m_videoGravity = [videoGravity copy];
	if ([state->m_videoGravity isEqualToString: AVLayerVideoGravityResize])
		self.contentsGravity = kCAGravityResize;
	else if ([state->m_videoGravity isEqualToString: AVLayerVideoGravityResizeAspectFill])
		self.contentsGravity = kCAGravityResizeAspectFill;
	else
		self.contentsGravity = kCAGravityResizeAspect;
}

- (void)captureOutput:(AVCaptureOutput *)captureOutput didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer fromConnection:(AVCaptureConnection *)connection
{
	(void) captureOutput; (void) connection;
	CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
	if (!imageBuffer)
		return;
	CGImageRef image = createPreviewImageFromPixelBuffer(imageBuffer);
	if (!image)
		return;
	AVCaptureVideoPreviewLayer *layer = [self retain];
	dispatch_async(dispatch_get_main_queue(), ^{
		[CATransaction begin];
		[CATransaction setDisableActions: YES];
		layer.contents = (id) image;
		[CATransaction commit];
		CGImageRelease(image);
		[layer release];
	});
}

@end
