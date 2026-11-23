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
#import <GameController/GCDeviceBattery.h>

@implementation GCDeviceBattery {
	float _batteryLevel;
	GCDeviceBatteryState _batteryState;
}

- (instancetype)initWithLevel:(float)level batteryState:(GCDeviceBatteryState)state
{
	self = [super init];
	if (!self)
		return nil;
	_batteryLevel = level;
	_batteryState = state;
	return self;
}

- (float)batteryLevel
{
	return _batteryLevel;
}

- (GCDeviceBatteryState)batteryState
{
	return _batteryState;
}

@end

#endif /* __LP64__ */
