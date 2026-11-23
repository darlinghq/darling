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

#ifndef _GCController_H_
#define _GCController_H_

#import <Foundation/Foundation.h>
#import <GameController/GCDevice.h>
#import <GameController/GCProductCategories.h>
#import <IOKit/hid/IOHIDBase.h>

@class GCExtendedGamepad;
@class GCMicroGamepad;
@class GCPhysicalInputProfile;
@class GCDeviceLight;
@class GCDeviceBattery;
@class GCDeviceHaptics;

NS_ASSUME_NONNULL_BEGIN

extern NSString *const GCControllerDidConnectNotification;
extern NSString *const GCControllerDidDisconnectNotification;
extern NSString *const GCControllerDidBecomeCurrentNotification;
extern NSString *const GCControllerDidStopBeingCurrentNotification;

typedef NS_ENUM(NSInteger, GCControllerPlayerIndex) {
	GCControllerPlayerIndexUnset = -1,
	GCControllerPlayerIndex1 = 0,
	GCControllerPlayerIndex2 = 1,
	GCControllerPlayerIndex3 = 2,
	GCControllerPlayerIndex4 = 3,
};

@interface GCController : NSObject <GCDevice>

+ (NSArray<GCController *> *)controllers;
+ (nullable GCController *)current;
+ (BOOL)supportsHIDDevice:(IOHIDDeviceRef)device;

+ (void)startWirelessControllerDiscoveryWithCompletionHandler:(nullable void (^)(void))completionHandler;
+ (void)stopWirelessControllerDiscovery;

@property (nonatomic, strong, null_resettable) dispatch_queue_t handlerQueue;
@property (nonatomic, readonly, copy, nullable) NSString *vendorName;
@property (nonatomic, readonly) NSString *productCategory;
@property (nonatomic) GCControllerPlayerIndex playerIndex;

@property (nonatomic, strong, readonly, nullable) GCExtendedGamepad *extendedGamepad;
@property (nonatomic, strong, readonly, nullable) GCMicroGamepad *microGamepad;
@property (nonatomic, strong, readonly, nullable) GCPhysicalInputProfile *physicalInputProfile;

@property (nonatomic, copy, readonly, nullable) GCDeviceBattery *battery;
@property (nonatomic, retain, readonly, nullable) GCDeviceLight *light;
@property (nonatomic, retain, readonly, nullable) GCDeviceHaptics *haptics;

@property (readonly, getter=isAttachedToDevice) BOOL attachedToDevice;

@end

NS_ASSUME_NONNULL_END

#endif
