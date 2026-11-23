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
	if (![self _allowsPublicWrites])
		return;
	[self _setValue:value handlerQueue:[self _handlerQueue] profileHandler:nil];
}

- (void)_copyStateFromElement:(GCControllerElement *)element
{
	if ([element isKindOfClass:[GCControllerButtonInput class]])
		[self setValue:[(GCControllerButtonInput *)element value]];
}

// Sets the value and calls the handlers on the queue: the value handler first, then the touched
// handler and the pressed handler for the states that changed. The profile handler, if any, runs last.
- (void)_setValue:(float)value handlerQueue:(dispatch_queue_t)queue profileHandler:(void (^)(GCControllerElement *))profileHandler
{
	float clamped = isnan(value) ? 0.0f : fmaxf(0.0f, fminf(1.0f, value));
	BOOL newPressed = (clamped >= GC_PRESS_THRESHOLD);
	BOOL newTouched = (clamped > 0.0f);
	BOOL pressedChanged = (newPressed != _pressed);
	BOOL touchedChanged = (newTouched != _touched);
	BOOL valueChanged = (clamped != _value);

	if (!valueChanged && !pressedChanged && !touchedChanged)
		return;

	_value = clamped;
	_pressed = newPressed;
	_touched = newTouched;
	if (profileHandler)
		[[self _profile] _noteEvent];

	GCControllerButtonValueChangedHandler vh = [_valueChangedHandler copy];
	GCControllerButtonValueChangedHandler ph = [_pressedChangedHandler copy];
	GCControllerButtonTouchedChangedHandler th = [_touchedChangedHandler copy];
	void (^phCopy)(GCControllerElement *) = [profileHandler copy];

	dispatch_queue_t targetQueue = queue ? queue : dispatch_get_main_queue();
	dispatch_async(targetQueue, ^{
		if (vh && valueChanged)
			vh(self, clamped, newPressed);
		if (th && touchedChanged)
			th(self, clamped, newPressed, newTouched);
		if (ph && pressedChanged)
			ph(self, clamped, newPressed);
		if (phCopy)
			phCopy(self);

		[vh release];
		[ph release];
		[th release];
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
