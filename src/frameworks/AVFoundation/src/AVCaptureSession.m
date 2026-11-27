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

#import <AVFoundation/AVCaptureSession.h>
#import "camera/DarlingCameraBackend.h"
#import "screen/DarlingScreenCaptureBackend.h"
#import <darling-stub-forwarding.h>
#import <objc/runtime.h>

@interface DarlingCaptureSessionState : NSObject {
@public
	NSMutableArray *inputs;
	NSMutableArray *outputs;
	NSMutableArray *engines;
	AVCaptureSessionPreset sessionPreset;
	BOOL running;
}
@end

@implementation DarlingCaptureSessionState

- (void)dealloc
{
	[inputs release];
	[outputs release];
	[engines release];
	[sessionPreset release];
	[super dealloc];
}

@end

static char kDarlingCaptureSessionStateKey;
#define STATE(self_) ((DarlingCaptureSessionState *)objc_getAssociatedObject(self_, &kDarlingCaptureSessionStateKey))

@implementation AVCaptureSession

- (instancetype)init
{
	self = [super init];
	if (self)
	{
		DarlingCaptureSessionState *state = [[DarlingCaptureSessionState alloc] init];
		objc_setAssociatedObject(self, &kDarlingCaptureSessionStateKey, state,
			OBJC_ASSOCIATION_RETAIN_NONATOMIC);
		[state release];
		STATE(self)->inputs = [[NSMutableArray alloc] init];
		STATE(self)->outputs = [[NSMutableArray alloc] init];
		STATE(self)->engines = [[NSMutableArray alloc] init];
		STATE(self)->sessionPreset = [AVCaptureSessionPresetHigh copy];
	}
	return self;
}

- (void)dealloc
{
	[self stopRunning];
	[super dealloc];
}

- (NSArray<AVCaptureInput *> *)inputs { return STATE(self)->inputs; }
- (NSArray<AVCaptureOutput *> *)outputs { return STATE(self)->outputs; }
- (BOOL)isRunning { return STATE(self)->running; }
- (AVCaptureSessionPreset)sessionPreset { return STATE(self)->sessionPreset; }

- (void)setSessionPreset:(AVCaptureSessionPreset)sessionPreset
{
	if (!sessionPreset || STATE(self)->running)
		return;
	if (![sessionPreset isEqualToString: STATE(self)->sessionPreset])
	{
		[STATE(self)->sessionPreset release];
		STATE(self)->sessionPreset = [sessionPreset copy];
	}
}

- (BOOL)canAddInput:(AVCaptureInput *)input
{
	if (input == nil || [STATE(self)->inputs containsObject: input])
		return NO;
	for (AVCaptureInput *existing in STATE(self)->inputs) {
		if ([existing isKindOfClass: [AVCaptureDeviceInput class]] && [input isKindOfClass: [AVCaptureDeviceInput class]]) {
			AVCaptureDevice *d1 = [(AVCaptureDeviceInput *) existing device];
			AVCaptureDevice *d2 = [(AVCaptureDeviceInput *) input device];
			if ([d1 hasMediaType: AVMediaTypeVideo] && [d2 hasMediaType: AVMediaTypeVideo])
				return NO;
		}
	}
	return YES;
}

- (BOOL)canAddOutput:(AVCaptureOutput *)output
{
	return output != nil && ![STATE(self)->outputs containsObject: output];
}

- (void)addInput:(AVCaptureInput *)input
{
	if ([self canAddInput: input])
		[STATE(self)->inputs addObject: input];
}

- (void)addOutput:(AVCaptureOutput *)output
{
	if ([self canAddOutput: output]) {
		[STATE(self)->outputs addObject: output];
		if (STATE(self)->running)
			[self startRunning];
	}
}

- (void)removeInput:(AVCaptureInput *)input
{
	[STATE(self)->inputs removeObject: input];
}

- (void)removeOutput:(AVCaptureOutput *)output
{
	[STATE(self)->outputs removeObject: output];
}

- (BOOL)canSetSessionPreset:(AVCaptureSessionPreset)preset
{
	return preset != nil;
}

- (void)beginConfiguration {}
- (void)commitConfiguration {}

- (void)startRunning
{
	if (STATE(self)->running)
		return;

	AVCaptureDevice *videoDevice = nil;
	AVCaptureScreenInput *screenInput = nil;
	for (AVCaptureInput *input in STATE(self)->inputs)
	{
		if ([input isKindOfClass: [AVCaptureScreenInput class]])
		{
			if (!screenInput)
				screenInput = (AVCaptureScreenInput *) input;
		}
		else if ([input isKindOfClass: [AVCaptureDeviceInput class]])
		{
			AVCaptureDevice *device = [(AVCaptureDeviceInput *) input device];
			if ([device hasMediaType: AVMediaTypeVideo] && !videoDevice)
				videoDevice = device;
		}
	}

	if (!videoDevice && !screenInput)
		return;

	/* The preset selects the capture resolution; videoSettings may refine it
	 * with explicit kCVPixelBufferWidth/HeightKey values. Screen inputs derive
	 * their resolution from cropRect and scaleFactor instead. */
	AVCaptureDeviceFormat *format = nil;
	if (videoDevice)
	{
		format = [videoDevice darling_formatForPreset: STATE(self)->sessionPreset];
		for (AVCaptureOutput *output in STATE(self)->outputs)
		{
			if ([output isKindOfClass: [AVCaptureVideoDataOutput class]])
			{
				AVCaptureDeviceFormat *settingsFormat = [videoDevice
					darling_formatForVideoSettings: [(AVCaptureVideoDataOutput *) output videoSettings]];
				if (settingsFormat)
				{
					format = settingsFormat;
					break;
				}
			}
		}
		if (format)
			[videoDevice setActiveFormat: format];
		else
			format = [videoDevice activeFormat];
		if (!format && !screenInput)
			return;
	}

	STATE(self)->running = YES;
	[[NSNotificationCenter defaultCenter] postNotificationName: AVCaptureSessionDidStartRunningNotification object: self];

	for (AVCaptureOutput *output in STATE(self)->outputs)
	{
		if (![output isKindOfClass: [AVCaptureVideoDataOutput class]])
			continue;
		if ([[output connections] count] == 0)
		{
			NSMutableArray *ports = [NSMutableArray array];
			for (AVCaptureInput *input in STATE(self)->inputs)
				[ports addObjectsFromArray: [input ports]];
			AVCaptureConnection *connection = [AVCaptureConnection darling_connectionWithInputPorts: ports
				output: output];
			[output darling_addConnection: connection];
		}
		for (AVCaptureConnection *connection in [output connections])
		{
			AVCaptureVideoDataOutput *videoOutput = (AVCaptureVideoDataOutput *) output;
			if (screenInput)
			{
				DarlingScreenCaptureEngine *engine = [[DarlingScreenCaptureEngine alloc]
					initWithOutput: videoOutput
					connection: connection
					videoSettings: [videoOutput videoSettings]
					alwaysDiscardsLateVideoFrames: [videoOutput isAlwaysDiscardsLateVideoFrames]
					screenInput: screenInput];
				[engine start];
				[STATE(self)->engines addObject: engine];
				[engine release];
			}
			else if (format)
			{
				DarlingCameraCaptureEngine *engine = [[DarlingCameraCaptureEngine alloc]
					initWithOutput: videoOutput
					connection: connection
					delegate: [videoOutput sampleBufferDelegate]
					queue: [videoOutput sampleBufferDelegateQueue]
					format: format
					videoSettings: [videoOutput videoSettings]
					alwaysDiscardsLateVideoFrames: [videoOutput isAlwaysDiscardsLateVideoFrames]];
				[engine start];
				[STATE(self)->engines addObject: engine];
				[engine release];
			}
		}
	}
}

- (void)stopRunning
{
	for (id engine in STATE(self)->engines)
	{
		if ([engine isKindOfClass: [DarlingCameraCaptureEngine class]]
				|| [engine isKindOfClass: [DarlingScreenCaptureEngine class]])
			[engine stop];
	}
	[STATE(self)->engines removeAllObjects];
	if (STATE(self)->running) {
		STATE(self)->running = NO;
		[[NSNotificationCenter defaultCenter] postNotificationName: AVCaptureSessionDidStopRunningNotification object: self];
	} else {
		STATE(self)->running = NO;
	}
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation {
    DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation {
    DarlingStubForwardInvocation(self, anInvocation);
}

@end
