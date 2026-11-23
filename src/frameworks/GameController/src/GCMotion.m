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
#import <GameController/GCMotion.h>
#import <GameController/GCController.h>
#import "GCInputInternal.h"

@implementation GCMotion {
	GCController *_controller;
	GCMotionValueChangedHandler _valueChangedHandler;
	BOOL _sensorsActive;
	BOOL _hasAttitudeAndRotationRate;
	GCAcceleration _gravity;
	GCAcceleration _userAcceleration;
	GCAcceleration _acceleration;
	GCQuaternion _attitude;
	GCRotationRate _rotationRate;
}

@synthesize valueChangedHandler = _valueChangedHandler;

- (instancetype)init
{
	return [self initWithController:nil attitudeAndRotationRate:NO];
}

- (instancetype)initWithController:(GCController *)controller attitudeAndRotationRate:(BOOL)attitude
{
	self = [super init];
	if (!self)
		return nil;
	_controller = controller;
	_hasAttitudeAndRotationRate = attitude;
	// the sensors of a controller created by the application run without being asked to
	_sensorsActive = YES;
	_attitude.w = 1.0;
	return self;
}

- (void)dealloc
{
	[_valueChangedHandler release];
	[super dealloc];
}

- (GCController *)controller { return _controller; }
- (BOOL)sensorsRequireManualActivation { return NO; }
- (BOOL)sensorsActive { return _sensorsActive; }
// Sensors that need no manual activation stay on whatever is asked (observed on macOS 26.7.3).
- (void)setSensorsActive:(BOOL)active { if ([self sensorsRequireManualActivation]) _sensorsActive = active; }
- (BOOL)hasGravityAndUserAcceleration { return NO; }
- (BOOL)hasAttitudeAndRotationRate { return _hasAttitudeAndRotationRate; }
- (BOOL)hasAttitude { return _hasAttitudeAndRotationRate; }
- (BOOL)hasRotationRate { return _hasAttitudeAndRotationRate; }
- (GCAcceleration)gravity { return _gravity; }
- (GCAcceleration)userAcceleration { return _userAcceleration; }
- (GCAcceleration)acceleration { return _acceleration; }
- (GCQuaternion)attitude { return _attitude; }
- (GCRotationRate)rotationRate { return _rotationRate; }

// The setters only store the value; macOS does not call the handler for them either (observed on
// macOS 26.7.3), the handler is for the data a device delivers.
- (void)setGravity:(GCAcceleration)gravity { _gravity = gravity; }
- (void)setUserAcceleration:(GCAcceleration)userAcceleration { _userAcceleration = userAcceleration; }
- (void)setAcceleration:(GCAcceleration)acceleration { _acceleration = acceleration; }
- (void)setAttitude:(GCQuaternion)attitude { _attitude = attitude; }
- (void)setRotationRate:(GCRotationRate)rotationRate { _rotationRate = rotationRate; }

- (void)setStateFromMotion:(GCMotion *)motion
{
	_gravity = motion.gravity;
	_userAcceleration = motion.userAcceleration;
	_acceleration = motion.acceleration;
	_attitude = motion.attitude;
	_rotationRate = motion.rotationRate;
}

@end
#else
#import <GameController/GCMotion.h>
#import <darling-stub-forwarding.h>

@implementation GCMotion

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
