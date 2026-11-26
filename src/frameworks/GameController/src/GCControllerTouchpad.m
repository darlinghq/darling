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
#import <GameController/GCControllerTouchpad.h>
#import "GCInputInternal.h"

@implementation GCControllerTouchpad {
	GCControllerButtonInput *_button;
	GCControllerDirectionPad *_touchSurface;
	GCControllerTouchpadHandler _touchDown;
	GCControllerTouchpadHandler _touchMoved;
	GCControllerTouchpadHandler _touchUp;
	GCTouchState _touchState;
	BOOL _reportsAbsoluteTouchSurfaceValues;
}

@synthesize touchDown = _touchDown;
@synthesize touchMoved = _touchMoved;
@synthesize touchUp = _touchUp;
@synthesize reportsAbsoluteTouchSurfaceValues = _reportsAbsoluteTouchSurfaceValues;

- (instancetype)initWithCollection:(GCControllerElement *)collection isAnalog:(BOOL)analog aliases:(NSSet<NSString *> *)aliases
{
	self = [super initWithCollection:collection isAnalog:analog aliases:aliases];
	if (!self)
		return nil;
	_button = [[GCControllerButtonInput alloc] initWithCollection:self isAnalog:YES aliases:[NSSet set]];
	_touchSurface = [[GCControllerDirectionPad alloc] initWithCollection:self isAnalog:YES aliases:[NSSet set]];
	return self;
}

- (void)dealloc
{
	[_button release];
	[_touchSurface release];
	[_touchDown release];
	[_touchMoved release];
	[_touchUp release];
	[super dealloc];
}

- (GCControllerButtonInput *)button { return _button; }
- (GCControllerDirectionPad *)touchSurface { return _touchSurface; }
- (GCTouchState)touchState { return _touchState; }

- (void)setValueForXAxis:(float)xAxis yAxis:(float)yAxis touchDown:(BOOL)touchDown buttonValue:(float)buttonValue
{
	if (![self _allowsPublicWrites])
		return;
	GCTouchState previous = _touchState;
	GCTouchState state = !touchDown ? GCTouchStateUp : (previous == GCTouchStateUp ? GCTouchStateDown : GCTouchStateMoving);
	_touchState = state;
	[_touchSurface setValueForXAxis:xAxis yAxis:yAxis];
	[_button setValue:buttonValue];
	GCControllerTouchpadHandler handler = state == GCTouchStateDown ? _touchDown : (state == GCTouchStateMoving ? _touchMoved : (previous != GCTouchStateUp ? _touchUp : nil));
	if (!handler)
		return;
	handler = [handler copy];
	float x = _touchSurface.xAxis.value, y = _touchSurface.yAxis.value, b = _button.value;
	BOOL pressed = _button.isPressed;
	dispatch_async([self _handlerQueue], ^{
		handler(self, x, y, b, pressed);
		[handler release];
	});
}

- (void)_copyStateFromElement:(GCControllerElement *)element
{
	if (![element isKindOfClass:[GCControllerTouchpad class]])
		return;
	GCControllerTouchpad *other = (GCControllerTouchpad *)element;
	[self setValueForXAxis:other.touchSurface.xAxis.value yAxis:other.touchSurface.yAxis.value
		touchDown:(other.touchState != GCTouchStateUp) buttonValue:other.button.value];
}

@end
#else
#import <GameController/GCControllerTouchpad.h>
#import <darling-stub-forwarding.h>

@implementation GCControllerTouchpad

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
