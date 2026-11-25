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
#import <GameController/GCMicroGamepadSnapshot.h>
#import "GCInputInternal.h"

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

const GCMicroGamepadSnapshotDataVersion GCCurrentMicroGamepadSnapshotDataVersion = GCMicroGamepadSnapshotDataVersion1;

BOOL GCMicroGamepadSnapshotDataFromNSData(GCMicroGamepadSnapshotData *snapshotData, NSData *data)
{
	if (!snapshotData || !GCSnapshotHeaderIsValid(data))
		return NO;
	memset(snapshotData, 0, sizeof(*snapshotData));
	memcpy(snapshotData, [data bytes], MIN([data length], sizeof(*snapshotData)));
	return YES;
}

NSData *NSDataFromGCMicroGamepadSnapshotData(GCMicroGamepadSnapshotData *snapshotData)
{
	return snapshotData ? [NSData dataWithBytes:snapshotData length:sizeof(*snapshotData)] : nil;
}

// The readers of the original data format refuse data of a later version (observed on macOS 26.7.3 for
// the extended gamepad; the other two follow the same rule).
BOOL GCMicroGamepadSnapShotDataV100FromNSData(GCMicroGamepadSnapShotDataV100 *snapshotData, NSData *data)
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

NSData *NSDataFromGCMicroGamepadSnapShotDataV100(GCMicroGamepadSnapShotDataV100 *snapshotData)
{
	return snapshotData ? [NSData dataWithBytes:snapshotData length:sizeof(*snapshotData)] : nil;
}

NSData *GCMicroGamepadStateData(GCMicroGamepad *gamepad)
{
	GCMicroGamepadSnapshotData d;
	memset(&d, 0, sizeof(d));
	d.version = GCMicroGamepadSnapshotDataVersion1;
	d.size = sizeof(d);
	d.dpadX = gamepad.dpad.xAxis.value;
	d.dpadY = gamepad.dpad.yAxis.value;
	d.buttonA = gamepad.buttonA.value;
	d.buttonX = gamepad.buttonX.value;
	return NSDataFromGCMicroGamepadSnapshotData(&d);
}

BOOL GCApplyMicroGamepadStateData(GCMicroGamepad *gamepad, NSData *data)
{
	GCMicroGamepadSnapshotData d;
	if (!GCMicroGamepadSnapshotDataFromNSData(&d, data))
		return NO;
	[gamepad.dpad setValueForXAxis:d.dpadX yAxis:d.dpadY];
	[gamepad.buttonA setValue:d.buttonA];
	[gamepad.buttonX setValue:d.buttonX];
	return YES;
}

@implementation GCMicroGamepadSnapshot

- (instancetype)initWithSnapshotData:(NSData *)data
{
	return [self initWithController:nil snapshotData:data];
}

- (instancetype)initWithController:(GCController *)controller snapshotData:(NSData *)data
{
	GCMicroGamepadSnapshotData d;
	if (!GCMicroGamepadSnapshotDataFromNSData(&d, data)) {
		[self release];
		return nil;
	}
	self = [super initWithController:controller];
	if (!self)
		return nil;
	GCApplyMicroGamepadStateData(self, data);
	return self;
}

- (NSData *)snapshotData
{
	return GCMicroGamepadStateData(self);
}

- (void)setSnapshotData:(NSData *)data
{
	GCApplyMicroGamepadStateData(self, data);
}

@end
#else
#import <GameController/GCMicroGamepadSnapshot.h>
#import <darling-stub-forwarding.h>

@implementation GCMicroGamepadSnapshot

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
