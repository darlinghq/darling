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
#import <GameController/GCKeyboard.h>
#import <GameController/GCKeyboardInput.h>
#import <GameController/GCProductCategories.h>
#import <GameController/_GCControllerManager.h>
#import "GCInputInternal.h"
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hid/IOHIDUsageTables.h>

NSString *const GCKeyboardDidConnectNotification = @"GCKeyboardDidConnectNotification";
NSString *const GCKeyboardDidDisconnectNotification = @"GCKeyboardDidDisconnectNotification";

@implementation GCKeyboardInput {
	NSMutableDictionary<NSNumber *, GCDeviceButtonInput *> *_buttonsByKeyCode;
	GCKeyboardValueChangedHandler _keyChangedHandler;
}

@synthesize keyChangedHandler = _keyChangedHandler;

- (instancetype)initWithKeyboard:(GCKeyboard *)keyboard
{
	self = [super initWithDevice:keyboard];
	if (!self)
		return nil;
	_buttonsByKeyCode = [[NSMutableDictionary alloc] init];
	return self;
}

- (void)dealloc
{
	[_buttonsByKeyCode release];
	[_keyChangedHandler release];
	[super dealloc];
}

- (BOOL)isAnyKeyPressed
{
	@synchronized (self) {
		for (GCDeviceButtonInput *btn in [_buttonsByKeyCode allValues]) {
			if (btn.isPressed)
				return YES;
		}
	}
	return NO;
}

- (GCDeviceButtonInput *)buttonForKeyCode:(GCKeyCode)code
{
	NSNumber *key = @(code);
	@synchronized (self) {
		GCDeviceButtonInput *btn = _buttonsByKeyCode[key];
		if (!btn) {
			btn = [[[GCDeviceButtonInput alloc] init] autorelease];
			_buttonsByKeyCode[key] = btn;
		}
		return btn;
	}
}

@end

@implementation GCKeyboard {
	GCKeyboardInput *_keyboardInput;
	dispatch_queue_t _handlerQueue;
}

- (instancetype)init
{
	self = [super init];
	if (!self)
		return nil;
	_handlerQueue = dispatch_get_main_queue();
	_keyboardInput = [[GCKeyboardInput alloc] initWithKeyboard:self];
	return self;
}

- (void)dealloc
{
	[_keyboardInput release];
	[super dealloc];
}

+ (GCKeyboard *)coalescedKeyboard
{
	return [[_GCControllerManager sharedInstance] coalescedKeyboard];
}

- (GCKeyboardInput *)keyboardInput
{
	return _keyboardInput;
}

- (GCPhysicalInputProfile *)physicalInputProfile
{
	return _keyboardInput;
}

- (NSString *)vendorName
{
	return @"Keyboard";
}

- (NSString *)productCategory
{
	return GCProductCategoryKeyboard;
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

// A key code is the usage of the key on the Keyboard/Keypad page. Usages below 4 are the
// error states of the report (roll over, POST fail, undefined), not keys.
- (void)handleHIDValue:(IOHIDValueRef)value
{
	IOHIDElementRef element = IOHIDValueGetElement(value);
	if (IOHIDElementGetUsagePage(element) != kHIDPage_KeyboardOrKeypad)
		return;
	uint32_t usage = IOHIDElementGetUsage(element);
	if (usage < kHIDUsage_KeyboardA || usage > kHIDUsage_KeyboardRightGUI)
		return;

	BOOL pressed = IOHIDValueGetIntegerValue(value) != 0;
	GCKeyboardInput *input = _keyboardInput;
	GCDeviceButtonInput *button = [input buttonForKeyCode:(GCKeyCode)usage];
	[button _setValue:(pressed ? 1.0f : 0.0f) handlerQueue:[self handlerQueue] profileHandler:^(GCControllerElement *e) {
		GCKeyboardValueChangedHandler keyHandler = [[input keyChangedHandler] copy];
		if (keyHandler)
			keyHandler(input, (GCDeviceButtonInput *)e, (GCKeyCode)usage, pressed);
		[keyHandler release];
		void (^handler)(GCPhysicalInputProfile *, GCControllerElement *) = [input valueDidChangeHandler];
		if (handler)
			handler(input, e);
	}];
}

@end

#endif /* __LP64__ */
