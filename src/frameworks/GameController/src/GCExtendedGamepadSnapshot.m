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
#import <GameController/GCExtendedGamepadSnapshot.h>
#import "GCInputInternal.h"

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

const GCExtendedGamepadSnapshotDataVersion GCCurrentExtendedGamepadSnapshotDataVersion = GCExtendedGamepadSnapshotDataVersion2;

BOOL GCExtendedGamepadSnapshotDataFromNSData(GCExtendedGamepadSnapshotData *snapshotData, NSData *data)
{
	if (!snapshotData || !GCSnapshotHeaderIsValid(data))
		return NO;
	memset(snapshotData, 0, sizeof(*snapshotData));
	memcpy(snapshotData, [data bytes], MIN([data length], sizeof(*snapshotData)));
	return YES;
}

NSData *NSDataFromGCExtendedGamepadSnapshotData(GCExtendedGamepadSnapshotData *snapshotData)
{
	return snapshotData ? [NSData dataWithBytes:snapshotData length:sizeof(*snapshotData)] : nil;
}

// The readers of the original data format refuse data of a later version (observed on macOS 26.7.3 for
// the extended gamepad; the other two follow the same rule).
BOOL GCExtendedGamepadSnapShotDataV100FromNSData(GCExtendedGamepadSnapShotDataV100 *snapshotData, NSData *data)
{
	if (!snapshotData || !GCSnapshotHeaderIsValid(data))
		return NO;
	uint16_t version;
	memcpy(&version, [data bytes], sizeof(version));
	if (version > 0x0100)
		return NO;
	memset(snapshotData, 0, sizeof(*snapshotData));
	memcpy(snapshotData, [data bytes], MIN([data length], sizeof(*snapshotData)));
	return YES;
}

NSData *NSDataFromGCExtendedGamepadSnapShotDataV100(GCExtendedGamepadSnapShotDataV100 *snapshotData)
{
	return snapshotData ? [NSData dataWithBytes:snapshotData length:sizeof(*snapshotData)] : nil;
}

NSData *GCExtendedGamepadStateData(GCExtendedGamepad *gamepad)
{
	GCExtendedGamepadSnapshotData d;
	memset(&d, 0, sizeof(d));
	d.version = GCExtendedGamepadSnapshotDataVersion2;
	d.size = sizeof(d);
	d.dpadX = gamepad.dpad.xAxis.value;
	d.dpadY = gamepad.dpad.yAxis.value;
	d.buttonA = gamepad.buttonA.value;
	d.buttonB = gamepad.buttonB.value;
	d.buttonX = gamepad.buttonX.value;
	d.buttonY = gamepad.buttonY.value;
	d.leftShoulder = gamepad.leftShoulder.value;
	d.rightShoulder = gamepad.rightShoulder.value;
	d.leftThumbstickX = gamepad.leftThumbstick.xAxis.value;
	d.leftThumbstickY = gamepad.leftThumbstick.yAxis.value;
	d.rightThumbstickX = gamepad.rightThumbstick.xAxis.value;
	d.rightThumbstickY = gamepad.rightThumbstick.yAxis.value;
	d.leftTrigger = gamepad.leftTrigger.value;
	d.rightTrigger = gamepad.rightTrigger.value;
	d.supportsClickableThumbsticks = (gamepad.leftThumbstickButton != nil && gamepad.rightThumbstickButton != nil);
	d.leftThumbstickButton = gamepad.leftThumbstickButton.isPressed;
	d.rightThumbstickButton = gamepad.rightThumbstickButton.isPressed;
	return NSDataFromGCExtendedGamepadSnapshotData(&d);
}

BOOL GCApplyExtendedGamepadStateData(GCExtendedGamepad *gamepad, NSData *data)
{
	GCExtendedGamepadSnapshotData d;
	if (!GCExtendedGamepadSnapshotDataFromNSData(&d, data))
		return NO;
	[gamepad.dpad setValueForXAxis:d.dpadX yAxis:d.dpadY];
	[gamepad.buttonA setValue:d.buttonA];
	[gamepad.buttonB setValue:d.buttonB];
	[gamepad.buttonX setValue:d.buttonX];
	[gamepad.buttonY setValue:d.buttonY];
	[gamepad.leftShoulder setValue:d.leftShoulder];
	[gamepad.rightShoulder setValue:d.rightShoulder];
	[gamepad.leftThumbstick setValueForXAxis:d.leftThumbstickX yAxis:d.leftThumbstickY];
	[gamepad.rightThumbstick setValueForXAxis:d.rightThumbstickX yAxis:d.rightThumbstickY];
	[gamepad.leftTrigger setValue:d.leftTrigger];
	[gamepad.rightTrigger setValue:d.rightTrigger];
	if (d.version >= GCExtendedGamepadSnapshotDataVersion2 && d.supportsClickableThumbsticks) {
		[gamepad.leftThumbstickButton setValue:d.leftThumbstickButton ? 1.0f : 0.0f];
		[gamepad.rightThumbstickButton setValue:d.rightThumbstickButton ? 1.0f : 0.0f];
	}
	return YES;
}

@implementation GCExtendedGamepadSnapshot

- (instancetype)initWithSnapshotData:(NSData *)data
{
	return [self initWithController:nil snapshotData:data];
}

- (instancetype)initWithController:(GCController *)controller snapshotData:(NSData *)data
{
	GCExtendedGamepadSnapshotData d;
	if (!GCExtendedGamepadSnapshotDataFromNSData(&d, data)) {
		[self release];
		return nil;
	}
	self = [super initWithController:controller];
	if (!self)
		return nil;
	GCApplyExtendedGamepadStateData(self, data);
	return self;
}

- (NSData *)snapshotData
{
	return GCExtendedGamepadStateData(self);
}

- (void)setSnapshotData:(NSData *)data
{
	GCApplyExtendedGamepadStateData(self, data);
}

@end
#else
#import <GameController/GCExtendedGamepadSnapshot.h>
#import <darling-stub-forwarding.h>

@implementation GCExtendedGamepadSnapshot

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
