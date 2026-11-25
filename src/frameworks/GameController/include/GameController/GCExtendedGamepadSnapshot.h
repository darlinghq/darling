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

#ifndef _GCExtendedGamepadSnapshot_H_
#define _GCExtendedGamepadSnapshot_H_

#import <Foundation/Foundation.h>
#import <GameController/GCExtendedGamepad.h>

NS_ASSUME_NONNULL_BEGIN

/* A copy of the state of an extended gamepad that the application can store and restore. */
@interface GCExtendedGamepadSnapshot : GCExtendedGamepad

@property (atomic, copy) NSData *snapshotData;

- (nullable instancetype)initWithSnapshotData:(NSData *)data;
- (nullable instancetype)initWithController:(nullable GCController *)controller snapshotData:(NSData *)data;

@end

typedef NS_ENUM(NSInteger, GCExtendedGamepadSnapshotDataVersion) {
	GCExtendedGamepadSnapshotDataVersion1 = 0x0100,
	GCExtendedGamepadSnapshotDataVersion2 = 0x0101
};

extern const GCExtendedGamepadSnapshotDataVersion GCCurrentExtendedGamepadSnapshotDataVersion;

#pragma pack(push, 1)
typedef struct {
	uint16_t version;
	uint16_t size;
	float dpadX;
	float dpadY;
	float buttonA;
	float buttonB;
	float buttonX;
	float buttonY;
	float leftShoulder;
	float rightShoulder;
	float leftThumbstickX;
	float leftThumbstickY;
	float rightThumbstickX;
	float rightThumbstickY;
	float leftTrigger;
	float rightTrigger;
	/* version 2 and later */
	BOOL supportsClickableThumbsticks;
	BOOL leftThumbstickButton;
	BOOL rightThumbstickButton;
} GCExtendedGamepadSnapshotData;
#pragma pack(pop)

extern BOOL GCExtendedGamepadSnapshotDataFromNSData(GCExtendedGamepadSnapshotData * _Nullable snapshotData, NSData * _Nullable data);
extern NSData * _Nullable NSDataFromGCExtendedGamepadSnapshotData(GCExtendedGamepadSnapshotData * _Nullable snapshotData);

typedef struct {
	uint16_t version;
	uint16_t size;
	float dpadX;
	float dpadY;
	float buttonA;
	float buttonB;
	float buttonX;
	float buttonY;
	float leftShoulder;
	float rightShoulder;
	float leftThumbstickX;
	float leftThumbstickY;
	float rightThumbstickX;
	float rightThumbstickY;
	float leftTrigger;
	float rightTrigger;
} GCExtendedGamepadSnapShotDataV100;

extern BOOL GCExtendedGamepadSnapShotDataV100FromNSData(GCExtendedGamepadSnapShotDataV100 * _Nullable snapshotData, NSData * _Nullable data);
extern NSData * _Nullable NSDataFromGCExtendedGamepadSnapShotDataV100(GCExtendedGamepadSnapShotDataV100 * _Nullable snapshotData);

NS_ASSUME_NONNULL_END

#endif
