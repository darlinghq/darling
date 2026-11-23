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
#import <GameController/GCControllerAxisInput.h>
#import "GCInputInternal.h"
#import <math.h>

@implementation GCControllerAxisInput {
	float _value;
	GCControllerAxisValueChangedHandler _valueChangedHandler;
}

@synthesize valueChangedHandler = _valueChangedHandler;

- (instancetype)init
{
	return [self initWithCollection:nil isAnalog:NO aliases:[NSSet set]];
}

- (void)dealloc
{
	[_valueChangedHandler release];
	[super dealloc];
}

- (float)value
{
	return _value;
}

- (void)setValue:(float)value
{
	if (![self _allowsPublicWrites])
		return;
	[self _setValue:value handlerQueue:[self _handlerQueue] profileHandler:nil];
}

- (void)_copyStateFromElement:(GCControllerElement *)element
{
	if ([element isKindOfClass:[GCControllerAxisInput class]])
		[self setValue:[(GCControllerAxisInput *)element value]];
}

- (void)_setValue:(float)value handlerQueue:(dispatch_queue_t)queue profileHandler:(void (^)(GCControllerElement *))profileHandler
{
	float clamped = isnan(value) ? 0.0f : fmaxf(-1.0f, fminf(1.0f, value));
	if (clamped == _value)
		return;
	_value = clamped;
	if (profileHandler)
		[[self _profile] _noteEvent];
	GCControllerAxisValueChangedHandler vh = [_valueChangedHandler copy];
	void (^phCopy)(GCControllerElement *) = [profileHandler copy];
	dispatch_queue_t targetQueue = queue ? queue : dispatch_get_main_queue();
	dispatch_async(targetQueue, ^{
		if (vh)
			vh(self, clamped);
		if (phCopy)
			phCopy(self);
		[vh release];
		[phCopy release];
	});
}

@end
#else
#import <GameController/GCControllerAxisInput.h>
#import <darling-stub-forwarding.h>

@implementation GCControllerAxisInput

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
