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

#import <AVFoundation/AVCaptureScreenInput.h>
#import "screen/DarlingScreenCaptureBackend.h"
#import <darling-stub-forwarding.h>
#import <objc/runtime.h>

@interface DarlingScreenInputState : NSObject {
@public
	CGDirectDisplayID _displayID;
	CMTime _minFrameDuration;
	CGRect _cropRect;
	CGFloat _scaleFactor;
	BOOL _capturesCursor;
	BOOL _capturesMouseClicks;
	BOOL _removesDuplicateFrames;
}
@end

@implementation DarlingScreenInputState
@end

static char kDarlingScreenInputStateKey;
#define STATE(self_) ((DarlingScreenInputState *)objc_getAssociatedObject(self_, &kDarlingScreenInputStateKey))

@implementation AVCaptureScreenInput

@dynamic minFrameDuration, cropRect, scaleFactor, capturesCursor,
	capturesMouseClicks, removesDuplicateFrames;

- (CMTime)minFrameDuration { return STATE(self)->_minFrameDuration; }
- (void)setMinFrameDuration:(CMTime)value { STATE(self)->_minFrameDuration = value; }

- (CGRect)cropRect { return STATE(self)->_cropRect; }
- (void)setCropRect:(CGRect)value { STATE(self)->_cropRect = value; }

- (CGFloat)scaleFactor { return STATE(self)->_scaleFactor; }
- (void)setScaleFactor:(CGFloat)value { STATE(self)->_scaleFactor = value; }

- (BOOL)capturesCursor { return STATE(self)->_capturesCursor; }
- (void)setCapturesCursor:(BOOL)value { STATE(self)->_capturesCursor = value; }

- (BOOL)capturesMouseClicks { return STATE(self)->_capturesMouseClicks; }
- (void)setCapturesMouseClicks:(BOOL)value { STATE(self)->_capturesMouseClicks = value; }

- (BOOL)removesDuplicateFrames { return STATE(self)->_removesDuplicateFrames; }
- (void)setRemovesDuplicateFrames:(BOOL)value { STATE(self)->_removesDuplicateFrames = value; }

- (instancetype)init
{
	return [self initWithDisplayID: CGMainDisplayID()];
}

- (instancetype)initWithDisplayID:(CGDirectDisplayID)displayID
{
	self = [super init];
	if (self)
	{
		if (CGRectIsEmpty(CGDisplayBounds(displayID)))
		{
			[self release];
			return nil;
		}
		DarlingScreenInputState *state = [[DarlingScreenInputState alloc] init];
		objc_setAssociatedObject(self, &kDarlingScreenInputStateKey, state,
			OBJC_ASSOCIATION_RETAIN_NONATOMIC);
		[state release];
		STATE(self)->_displayID = displayID;
		STATE(self)->_minFrameDuration = kCMTimeInvalid;
		STATE(self)->_cropRect = CGRectZero;
		STATE(self)->_scaleFactor = 1;
		STATE(self)->_capturesCursor = YES;
		STATE(self)->_capturesMouseClicks = NO;
		STATE(self)->_removesDuplicateFrames = NO;
		objc_setAssociatedObject(self, &kDarlingCameraPortsKey,
			[NSArray arrayWithObject: [AVCaptureInputPort darling_videoPortForScreenInput: self]],
			OBJC_ASSOCIATION_RETAIN_NONATOMIC);
	}
	return self;
}

- (CGDirectDisplayID)darling_displayID
{
	return STATE(self)->_displayID;
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
