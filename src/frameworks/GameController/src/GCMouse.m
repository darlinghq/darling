/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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

// The implementation is 64-bit only, like GameController on macOS (its 32-bit
// Objective-C ABI cannot hold instance variables declared in the @implementation
// and the framework is x86_64 and arm64 only there); the 32-bit slice keeps the stub.
#ifdef __LP64__
#import <GameController/GCMouse.h>
#import <GameController/GCProductCategories.h>
#import <GameController/_GCControllerManager.h>
#import "GCInputInternal.h"
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hid/IOHIDUsageTables.h>

NSString *const GCMouseDidConnectNotification = @"GCMouseDidConnectNotification";
NSString *const GCMouseDidDisconnectNotification = @"GCMouseDidDisconnectNotification";
NSString *const GCMouseDidBecomeCurrentNotification = @"GCMouseDidBecomeCurrentNotification";
NSString *const GCMouseDidStopBeingCurrentNotification = @"GCMouseDidStopBeingCurrentNotification";

@implementation GCMouseInput {
	GCDeviceButtonInput *_leftButton;
	GCDeviceButtonInput *_rightButton;
	GCDeviceButtonInput *_middleButton;
	NSArray<GCDeviceButtonInput *> *_auxiliaryButtons;
	GCDeviceCursor *_scroll;
}

@synthesize mouseMovedHandler = _mouseMovedHandler;

- (instancetype)init
{
	return [self initWithMouse:nil auxiliaryButtonCount:0];
}

- (instancetype)initWithMouse:(GCMouse *)mouse auxiliaryButtonCount:(NSUInteger)auxiliaryCount
{
	self = [super initWithDevice:mouse];
	if (!self)
		return nil;
	_leftButton = [[GCDeviceButtonInput alloc] initWithCollection:nil isAnalog:NO aliases:[NSSet setWithObject:@"Left Button"]];
	// the other buttons exist only for a mouse that has them: a profile without a mouse has none
	if (mouse) {
		_rightButton = [[GCDeviceButtonInput alloc] initWithCollection:nil isAnalog:NO aliases:[NSSet setWithObject:@"Right Button"]];
		_middleButton = [[GCDeviceButtonInput alloc] initWithCollection:nil isAnalog:NO aliases:[NSSet setWithObject:@"Middle Button"]];
	}
	NSMutableArray *aux = [NSMutableArray array];
	for (NSUInteger i = 0; i < auxiliaryCount; i++)
		[aux addObject:[[[GCDeviceButtonInput alloc] initWithCollection:nil isAnalog:NO aliases:[NSSet set]] autorelease]];
	_auxiliaryButtons = auxiliaryCount ? [aux copy] : nil;
	_scroll = [[GCDeviceCursor alloc] initWithCollection:nil isAnalog:YES aliases:[NSSet setWithObject:@"Scroll"]];
	// "Left Button" and "Scroll" are the names macOS lists for a mouse profile; the names of the
	// right and middle button follow the same pattern (inference, not observed on a device).
	[self _registerButton:_leftButton key:@"Left Button"];
	if (mouse) {
		[self _registerButton:_rightButton key:@"Right Button"];
		[self _registerButton:_middleButton key:@"Middle Button"];
	}
	[self _registerDpad:_scroll key:@"Scroll"];
	return self;
}

- (void)dealloc
{
	[_mouseMovedHandler release];
	[_leftButton release];
	[_rightButton release];
	[_middleButton release];
	[_auxiliaryButtons release];
	[_scroll release];
	[super dealloc];
}

- (GCDeviceButtonInput *)leftButton { return _leftButton; }
- (GCDeviceButtonInput *)rightButton { return _rightButton; }
- (GCDeviceButtonInput *)middleButton { return _middleButton; }
- (NSArray *)auxiliaryButtons { return _auxiliaryButtons; }
- (GCDeviceCursor *)scroll { return _scroll; }

@end

@implementation GCMouse {
	NSString *_vendorName;
	GCMouseInput *_mouseInput;
	dispatch_queue_t _handlerQueue;
	float _pendingDeltaX;
	float _pendingDeltaY;
	BOOL _movePending;
}

- (instancetype)init
{
	return [self initWithHIDDevice:NULL];
}

- (instancetype)initWithHIDDevice:(IOHIDDeviceRef)device
{
	self = [super init];
	if (!self)
		return nil;

	_handlerQueue = dispatch_get_main_queue();

	NSString *name = @"Mouse";
	NSUInteger auxiliary = 0;
	if (device) {
		CFTypeRef prop = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductKey));
		if (prop && CFGetTypeID(prop) == CFStringGetTypeID())
			name = [NSString stringWithString:(__bridge NSString *)prop];

		// buttons 1 .. 3 are the left, right and middle buttons, the others are auxiliary
		NSDictionary *match = @{ (id)CFSTR(kIOHIDElementUsagePageKey): @(kHIDPage_Button) };
		CFArrayRef buttons = IOHIDDeviceCopyMatchingElements(device, (__bridge CFDictionaryRef)match, kIOHIDOptionsTypeNone);
		if (buttons) {
			for (CFIndex i = 0; i < CFArrayGetCount(buttons); i++) {
				IOHIDElementRef e = (IOHIDElementRef)CFArrayGetValueAtIndex(buttons, i);
				if (IOHIDElementGetUsage(e) > 3 + auxiliary)
					auxiliary = IOHIDElementGetUsage(e) - 3;
			}
			CFRelease(buttons);
		}
	}
	_vendorName = [name copy];
	_mouseInput = [[GCMouseInput alloc] initWithMouse:self auxiliaryButtonCount:auxiliary];

	return self;
}

- (void)dealloc
{
	[_vendorName release];
	[_mouseInput release];
	[super dealloc];
}

+ (NSArray<GCMouse *> *)mice
{
	return [[_GCControllerManager sharedInstance] mice];
}

+ (GCMouse *)current
{
	return [[_GCControllerManager sharedInstance] currentMouse];
}

- (GCMouseInput *)mouseInput
{
	return _mouseInput;
}

- (GCPhysicalInputProfile *)physicalInputProfile
{
	return _mouseInput;
}

- (NSString *)vendorName
{
	return _vendorName;
}

- (NSString *)productCategory
{
	return GCProductCategoryMouse;
}

- (BOOL)isAttachedToDevice
{
	return YES;
}

- (dispatch_queue_t)handlerQueue
{
	@synchronized (self) {
		return _handlerQueue;
	}
}

- (void)setHandlerQueue:(dispatch_queue_t)queue
{
	@synchronized (self) {
		_handlerQueue = queue ? queue : dispatch_get_main_queue();
	}
}

// Sends the movement of one input report to the handler on the handler queue, as one call.
- (void)_addMotionX:(float)dx y:(float)dy
{
	dispatch_queue_t queue;
	@synchronized (self) {
		_pendingDeltaX += dx;
		_pendingDeltaY += dy;
		if (_movePending)
			return;
		_movePending = YES;
		queue = _handlerQueue;
	}
	dispatch_async(queue, ^{
		float x, y;
		@synchronized (self) {
			x = _pendingDeltaX;
			y = _pendingDeltaY;
			_pendingDeltaX = _pendingDeltaY = 0;
			_movePending = NO;
		}
		GCMouseMoved handler = [[_mouseInput mouseMovedHandler] copy];
		if (handler)
			handler(_mouseInput, x, y);
		[handler release];
	});
}

- (void)handleHIDValue:(IOHIDValueRef)value
{
	IOHIDElementRef element = IOHIDValueGetElement(value);
	uint32_t page = IOHIDElementGetUsagePage(element);
	uint32_t usage = IOHIDElementGetUsage(element);
	CFIndex v = IOHIDValueGetIntegerValue(value);
	dispatch_queue_t queue = [self handlerQueue];
	GCMouseInput *input = _mouseInput;

	if (page == kHIDPage_Button) {
		GCDeviceButtonInput *button = nil;
		if (usage == 1)
			button = [input leftButton];
		else if (usage == 2)
			button = [input rightButton];
		else if (usage == 3)
			button = [input middleButton];
		else if (usage > 3 && usage - 4 < [[input auxiliaryButtons] count])
			button = [input auxiliaryButtons][usage - 4];
		if (!button)
			return;
		[button _setValue:(v != 0 ? 1.0f : 0.0f) handlerQueue:queue profileHandler:^(GCControllerElement *e) {
			void (^handler)(GCPhysicalInputProfile *, GCControllerElement *) = [input valueDidChangeHandler];
			if (handler)
				handler(input, e);
		}];
	} else if (page == kHIDPage_GenericDesktop && usage == kHIDUsage_GD_X) {
		[self _addMotionX:(float)v y:0.0f];
	} else if (page == kHIDPage_GenericDesktop && usage == kHIDUsage_GD_Y) {
		// Deltas are passed on as the HID reports them (positive Y is down); the orientation macOS uses
		// is not documented and has not been confirmed on a Mac with a mouse.
		[self _addMotionX:0.0f y:(float)v];
	}
}

@end

#else
#import <GameController/GCMouse.h>

NSString *const GCMouseDidConnectNotification = @"GCMouseDidConnectNotification";
NSString *const GCMouseDidDisconnectNotification = @"GCMouseDidDisconnectNotification";

@implementation GCMouseInput

@synthesize mouseMovedHandler = _mouseMovedHandler;

- (void)dealloc
{
	[_mouseMovedHandler release];
	[super dealloc];
}

- (GCDeviceButtonInput *)leftButton
{
	return nil;
}

- (GCDeviceButtonInput *)rightButton
{
	return nil;
}

- (GCDeviceButtonInput *)middleButton
{
	return nil;
}

- (NSArray *)auxiliaryButtons
{
	return nil;
}

- (GCDeviceDirectionPad *)scroll
{
	return nil;
}

@end

@implementation GCMouse

+ (NSArray<GCMouse *> *)mice
{
	return @[];
}

+ (GCMouse *)current
{
	return nil;
}

- (GCMouseInput *)mouseInput
{
	return nil;
}

- (NSString *)vendorName
{
	return nil;
}

- (NSString *)productCategory
{
	return @"Mouse";
}

- (BOOL)isAttachedToDevice
{
	return NO;
}

- (dispatch_queue_t)handlerQueue
{
	return dispatch_get_main_queue();
}

- (void)setHandlerQueue:(dispatch_queue_t)queue
{
}

@end

#endif /* __LP64__ */
