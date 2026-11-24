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

#ifndef _GCMicroGamepadSnapshot_H_
#define _GCMicroGamepadSnapshot_H_

#import <Foundation/Foundation.h>
#import <GameController/GCMicroGamepad.h>

NS_ASSUME_NONNULL_BEGIN

/* A copy of the state of a micro gamepad that the application can store and restore. */
@interface GCMicroGamepadSnapshot : GCMicroGamepad

@property (atomic, copy) NSData *snapshotData;

- (nullable instancetype)initWithSnapshotData:(NSData *)data;
- (nullable instancetype)initWithController:(nullable GCController *)controller snapshotData:(NSData *)data;

@end

typedef NS_ENUM(NSInteger, GCMicroGamepadSnapshotDataVersion) {
	GCMicroGamepadSnapshotDataVersion1 = 0x0100
};

extern const GCMicroGamepadSnapshotDataVersion GCCurrentMicroGamepadSnapshotDataVersion;

#pragma pack(push, 1)
typedef struct {
	uint16_t version;
	uint16_t size;
	float dpadX;
	float dpadY;
	float buttonA;
	float buttonX;
} GCMicroGamepadSnapshotData;
#pragma pack(pop)

extern BOOL GCMicroGamepadSnapshotDataFromNSData(GCMicroGamepadSnapshotData * _Nullable snapshotData, NSData * _Nullable data);
extern NSData * _Nullable NSDataFromGCMicroGamepadSnapshotData(GCMicroGamepadSnapshotData * _Nullable snapshotData);

typedef struct {
	uint16_t version;
	uint16_t size;
	float dpadX;
	float dpadY;
	float buttonA;
	float buttonX;
} GCMicroGamepadSnapShotDataV100;

extern BOOL GCMicroGamepadSnapShotDataV100FromNSData(GCMicroGamepadSnapShotDataV100 * _Nullable snapshotData, NSData * _Nullable data);
extern NSData * _Nullable NSDataFromGCMicroGamepadSnapShotDataV100(GCMicroGamepadSnapShotDataV100 * _Nullable snapshotData);

NS_ASSUME_NONNULL_END

#endif
