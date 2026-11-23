/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel
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
#import <GameController/GCControllerButtonInput.h>
#import "GCInputInternal.h"
#import <math.h>

@implementation GCControllerButtonInput {
	float _value;
	BOOL _pressed;
	BOOL _touched;
	GCControllerButtonValueChangedHandler _valueChangedHandler;
	GCControllerButtonValueChangedHandler _pressedChangedHandler;
	GCControllerButtonTouchedChangedHandler _touchedChangedHandler;
}

@synthesize valueChangedHandler = _valueChangedHandler;
@synthesize pressedChangedHandler = _pressedChangedHandler;
@synthesize touchedChangedHandler = _touchedChangedHandler;

- (void)dealloc
{
	[_valueChangedHandler release];
	[_pressedChangedHandler release];
	[_touchedChangedHandler release];
	[super dealloc];
}

- (float)value
{
	return _value;
}

- (BOOL)isPressed
{
	return _pressed;
}

- (BOOL)isTouched
{
	return _touched;
}

- (void)setValue:(float)value
{
	_value = fmaxf(0.0f, fminf(1.0f, value));
	_pressed = (_value > 0.0f);
	_touched = (_value > 0.0f);
}

- (void)_setValue:(float)value handlerQueue:(dispatch_queue_t)queue profileHandler:(void (^)(GCControllerElement *))profileHandler
{
	float clamped = fmaxf(0.0f, fminf(1.0f, value));
	BOOL newPressed = (clamped > 0.0f);
	BOOL pressedChanged = (newPressed != _pressed);
	BOOL valueChanged = (clamped != _value);

	if (!valueChanged && !pressedChanged)
		return;

	_value = clamped;
	_pressed = newPressed;
	_touched = (clamped > 0.0f);

	GCControllerButtonValueChangedHandler vh = [_valueChangedHandler copy];
	GCControllerButtonValueChangedHandler ph = [_pressedChangedHandler copy];
	void (^phCopy)(GCControllerElement *) = [profileHandler copy];

	dispatch_queue_t targetQueue = queue ? queue : dispatch_get_main_queue();
	dispatch_async(targetQueue, ^{
		if (vh)
			vh(self, clamped, newPressed);
		if (pressedChanged && ph)
			ph(self, clamped, newPressed);
		if (phCopy)
			phCopy(self);

		[vh release];
		[ph release];
		[phCopy release];
	});
}

@end

#else
#import <GameController/GCControllerButtonInput.h>
#import <darling-stub-forwarding.h>

@implementation GCControllerButtonInput

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

@end

#endif /* __LP64__ */
