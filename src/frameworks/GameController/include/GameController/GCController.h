/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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

#include <Foundation/Foundation.h>

/* Games observe controllers appearing and disappearing through these. Darling has no
 * controller hardware, so nothing posts them, but the symbols must exist: dyld refuses
 * to launch a binary that imports an undefined symbol, and games reference both. */
extern NSString *const GCControllerDidConnectNotification;
extern NSString *const GCControllerDidDisconnectNotification;

@class GCExtendedGamepad;
@class GCMicroGamepad;
@class GCPhysicalInputProfile;

typedef NS_ENUM(NSInteger, GCControllerPlayerIndex) {
	GCControllerPlayerIndexUnset = -1,
	GCControllerPlayerIndex1 = 0,
	GCControllerPlayerIndex2 = 1,
	GCControllerPlayerIndex3 = 2,
	GCControllerPlayerIndex4 = 3,
};

@interface GCController : NSObject

/* Games enumerate controllers at startup and then read the attached gamepad. No
 * controller hardware exists here, so these answer honestly: an empty array and nil.
 * What matters is that they exist - a selector miss on GCController is an uncaught
 * NSException, which kills the app before any of its own code runs. */
+ (NSArray *)controllers;
+ (NSArray *)extendedGamepads;

- (GCExtendedGamepad *)extendedGamepad;
- (GCMicroGamepad *)microGamepad;
- (GCPhysicalInputProfile *)physicalInputProfile;

@property(readonly, copy, nullable) NSString *vendorName;
@property(readonly, copy) NSString *productCategory;
@property(nonatomic) GCControllerPlayerIndex playerIndex;

@end
