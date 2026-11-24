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

#ifndef _GCGamepadSnapshot_H_
#define _GCGamepadSnapshot_H_

#import <Foundation/Foundation.h>
#import <GameController/GCGamepad.h>

NS_ASSUME_NONNULL_BEGIN

@interface GCGamepadSnapshot : GCGamepad

@property (atomic, copy) NSData *snapshotData;

- (nullable instancetype)initWithSnapshotData:(NSData *)data;
- (nullable instancetype)initWithController:(nullable GCController *)controller snapshotData:(NSData *)data;

@end

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
} GCGamepadSnapShotDataV100;

extern BOOL GCGamepadSnapShotDataV100FromNSData(GCGamepadSnapShotDataV100 * _Nullable snapshotData, NSData * _Nullable data);
extern NSData * _Nullable NSDataFromGCGamepadSnapShotDataV100(GCGamepadSnapShotDataV100 * _Nullable snapshotData);

NS_ASSUME_NONNULL_END

#endif
