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

#import <AVFoundation/AVCaptureVideoDataOutput.h>
#import "camera/DarlingCameraBackend.h"
#import <objc/runtime.h>
#import <darling-stub-forwarding.h>

@interface DarlingVideoDataOutputState : NSObject {
@public
	id<AVCaptureVideoDataOutputSampleBufferDelegate> delegate;
	dispatch_queue_t delegateQueue;
	NSDictionary *videoSettings;
	BOOL alwaysDiscardsLateVideoFrames;
}
@end

@implementation DarlingVideoDataOutputState

- (void)dealloc
{
	objc_storeWeak((id *) &delegate, nil);
	if (delegateQueue)
		dispatch_release(delegateQueue);
	[videoSettings release];
	[super dealloc];
}

@end

static char kDarlingVideoDataOutputStateKey;
#define STATE(self_) ((DarlingVideoDataOutputState *)objc_getAssociatedObject(self_, &kDarlingVideoDataOutputStateKey))

@implementation AVCaptureVideoDataOutput

- (instancetype)init
{
	self = [super init];
	if (self)
	{
		DarlingVideoDataOutputState *state = [[DarlingVideoDataOutputState alloc] init];
		objc_setAssociatedObject(self, &kDarlingVideoDataOutputStateKey, state,
			OBJC_ASSOCIATION_RETAIN_NONATOMIC);
		[state release];
		STATE(self)->alwaysDiscardsLateVideoFrames = YES;
	}
	return self;
}

- (void)dealloc
{
	[super dealloc];
}

- (id<AVCaptureVideoDataOutputSampleBufferDelegate>)sampleBufferDelegate
{
	return (id<AVCaptureVideoDataOutputSampleBufferDelegate>) objc_loadWeak((id *) &STATE(self)->delegate);
}

- (dispatch_queue_t)sampleBufferDelegateQueue
{
	return STATE(self)->delegateQueue;
}

- (void)setSampleBufferDelegate:(id<AVCaptureVideoDataOutputSampleBufferDelegate>)delegate
	queue:(dispatch_queue_t)queue
{
	// The delegate is not retained by AVCaptureVideoDataOutput; a weak slot
	// keeps the documented non-owning behavior without a dangling pointer.
	objc_storeWeak((id *) &STATE(self)->delegate, delegate);
	if (queue != STATE(self)->delegateQueue)
	{
		if (queue)
			dispatch_retain(queue);
		if (STATE(self)->delegateQueue)
			dispatch_release(STATE(self)->delegateQueue);
		STATE(self)->delegateQueue = queue;
	}
}

- (void)setSampleBufferDelegate:(id<AVCaptureVideoDataOutputSampleBufferDelegate>)delegate
{
	[self setSampleBufferDelegate: delegate queue: STATE(self)->delegateQueue];
}

- (NSDictionary *)videoSettings { return STATE(self)->videoSettings; }
- (void)setVideoSettings:(NSDictionary *)settings
{
	if (settings != STATE(self)->videoSettings)
	{
		[STATE(self)->videoSettings release];
		STATE(self)->videoSettings = [settings copy];
	}
}

- (BOOL)isAlwaysDiscardsLateVideoFrames { return STATE(self)->alwaysDiscardsLateVideoFrames; }
- (BOOL)alwaysDiscardsLateVideoFrames { return [self isAlwaysDiscardsLateVideoFrames]; }
- (void)setAlwaysDiscardsLateVideoFrames:(BOOL)discard
{
	STATE(self)->alwaysDiscardsLateVideoFrames = discard;
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
