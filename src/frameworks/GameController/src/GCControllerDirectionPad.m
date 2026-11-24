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
#import <GameController/GCControllerDirectionPad.h>
#import "GCInputInternal.h"
#import <math.h>

@implementation GCControllerDirectionPad {
	GCControllerAxisInput *_xAxis;
	GCControllerAxisInput *_yAxis;
	GCControllerButtonInput *_up;
	GCControllerButtonInput *_down;
	GCControllerButtonInput *_left;
	GCControllerButtonInput *_right;
	GCControllerDirectionPadValueChangedHandler _valueChangedHandler;
}

@synthesize valueChangedHandler = _valueChangedHandler;

- (instancetype)init
{
	return [self initWithCollection:nil isAnalog:YES aliases:[NSSet set]];
}

- (instancetype)initWithCollection:(GCControllerElement *)collection isAnalog:(BOOL)analog aliases:(NSSet<NSString *> *)aliases
{
	self = [super initWithCollection:collection isAnalog:analog aliases:aliases];
	if (!self)
		return nil;
	// the children of a pad are analog whatever the pad is
	_xAxis = [[GCControllerAxisInput alloc] initWithCollection:self isAnalog:YES aliases:[NSSet set]];
	_yAxis = [[GCControllerAxisInput alloc] initWithCollection:self isAnalog:YES aliases:[NSSet set]];
	_up = [[GCControllerButtonInput alloc] initWithCollection:self isAnalog:YES aliases:[NSSet set]];
	_down = [[GCControllerButtonInput alloc] initWithCollection:self isAnalog:YES aliases:[NSSet set]];
	_left = [[GCControllerButtonInput alloc] initWithCollection:self isAnalog:YES aliases:[NSSet set]];
	_right = [[GCControllerButtonInput alloc] initWithCollection:self isAnalog:YES aliases:[NSSet set]];
	return self;
}

- (void)dealloc
{
	[_xAxis release];
	[_yAxis release];
	[_up release];
	[_down release];
	[_left release];
	[_right release];
	[_valueChangedHandler release];
	[super dealloc];
}

- (GCControllerAxisInput *)xAxis { return _xAxis; }
- (GCControllerAxisInput *)yAxis { return _yAxis; }
- (GCControllerButtonInput *)up { return _up; }
- (GCControllerButtonInput *)down { return _down; }
- (GCControllerButtonInput *)left { return _left; }
- (GCControllerButtonInput *)right { return _right; }

- (void)_assignAliasesForKey:(NSString *)key
{
	[self _setAliases:[NSSet setWithObject:key]];
	[_xAxis _setAliases:[NSSet setWithObject:[key stringByAppendingString:@" X Axis"]]];
	[_yAxis _setAliases:[NSSet setWithObject:[key stringByAppendingString:@" Y Axis"]]];
	[_up _setAliases:[NSSet setWithObject:[key stringByAppendingString:@" Up"]]];
	[_down _setAliases:[NSSet setWithObject:[key stringByAppendingString:@" Down"]]];
	[_left _setAliases:[NSSet setWithObject:[key stringByAppendingString:@" Left"]]];
	[_right _setAliases:[NSSet setWithObject:[key stringByAppendingString:@" Right"]]];
}

- (void)setValueForXAxis:(float)xAxis yAxis:(float)yAxis
{
	if (![self _allowsPublicWrites])
		return;
	[self _setXAxis:xAxis yAxis:yAxis handlerQueue:[self _handlerQueue] profileHandler:nil];
}

- (void)_copyStateFromElement:(GCControllerElement *)element
{
	if ([element isKindOfClass:[GCControllerDirectionPad class]])
		[self setValueForXAxis:[[(GCControllerDirectionPad *)element xAxis] value] yAxis:[[(GCControllerDirectionPad *)element yAxis] value]];
}

- (void)_setXAxis:(float)x yAxis:(float)y handlerQueue:(dispatch_queue_t)queue profileHandler:(void (^)(GCControllerElement *))profileHandler
{
	float cx = isnan(x) ? 0.0f : fmaxf(-1.0f, fminf(1.0f, x));
	float cy = isnan(y) ? 0.0f : fmaxf(-1.0f, fminf(1.0f, y));
	if (cx == _xAxis.value && cy == _yAxis.value)
		return;
	if (profileHandler)
		[[self _profile] _noteEvent];
	// every element of the pad reports its own change through its handlers
	[_xAxis _setValue:cx handlerQueue:queue profileHandler:nil];
	[_yAxis _setValue:cy handlerQueue:queue profileHandler:nil];
	[_up _setValue:(cy > 0.0f) ? cy : 0.0f handlerQueue:queue profileHandler:nil];
	[_down _setValue:(cy < 0.0f) ? -cy : 0.0f handlerQueue:queue profileHandler:nil];
	[_right _setValue:(cx > 0.0f) ? cx : 0.0f handlerQueue:queue profileHandler:nil];
	[_left _setValue:(cx < 0.0f) ? -cx : 0.0f handlerQueue:queue profileHandler:nil];

	GCControllerDirectionPadValueChangedHandler vh = [_valueChangedHandler copy];
	void (^phCopy)(GCControllerElement *) = [profileHandler copy];
	dispatch_queue_t targetQueue = queue ? queue : dispatch_get_main_queue();
	dispatch_async(targetQueue, ^{
		if (vh)
			vh(self, cx, cy);
		if (phCopy)
			phCopy(self);
		[vh release];
		[phCopy release];
	});
}

@end
#else
#import <GameController/GCControllerDirectionPad.h>
#import <darling-stub-forwarding.h>

@implementation GCControllerDirectionPad

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
