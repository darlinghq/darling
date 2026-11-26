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

#ifndef _GCPhysicalInputProfile_H_
#define _GCPhysicalInputProfile_H_

#import <Foundation/Foundation.h>
#import <GameController/GCControllerAxisInput.h>
#import <GameController/GCControllerButtonInput.h>
#import <GameController/GCControllerDirectionPad.h>

@protocol GCDevice;
@class GCControllerElement;
@class GCControllerTouchpad;

typedef GCControllerElement GCDeviceElement;
typedef GCControllerButtonInput GCDeviceButtonInput;
typedef GCControllerAxisInput GCDeviceAxisInput;
typedef GCControllerDirectionPad GCDeviceDirectionPad;
typedef GCControllerTouchpad GCDeviceTouchpad;

NS_ASSUME_NONNULL_BEGIN

@interface GCPhysicalInputProfile : NSObject

@property (nonatomic, readonly, weak, nullable) id<GCDevice> device;
@property (atomic, readonly) NSTimeInterval lastEventTimestamp;
@property (nonatomic, readonly) BOOL hasRemappedElements;

@property (nonatomic, copy, nullable) void (^valueDidChangeHandler)(__kindof GCPhysicalInputProfile *profile, GCControllerElement *element);

@property (nonatomic, readonly, strong) NSDictionary<NSString *, GCDeviceElement *> *elements;
@property (nonatomic, readonly, strong) NSDictionary<NSString *, GCDeviceButtonInput *> *buttons;
@property (nonatomic, readonly, strong) NSDictionary<NSString *, GCDeviceAxisInput *> *axes;
@property (nonatomic, readonly, strong) NSDictionary<NSString *, GCDeviceDirectionPad *> *dpads;

@property (nonatomic, readonly, strong) NSDictionary<NSString *, GCDeviceTouchpad *> *touchpads;
@property (nonatomic, readonly, strong) NSSet<GCDeviceElement *> *allElements;
@property (nonatomic, readonly, strong) NSSet<GCDeviceButtonInput *> *allButtons;
@property (nonatomic, readonly, strong) NSSet<GCDeviceAxisInput *> *allAxes;
@property (nonatomic, readonly, strong) NSSet<GCDeviceDirectionPad *> *allDpads;
@property (nonatomic, readonly, strong) NSSet<GCDeviceTouchpad *> *allTouchpads;

- (nullable __kindof GCDeviceElement *)objectForKeyedSubscript:(NSString *)key;

/* A copy of the current state in a new, writable profile of the same class that belongs to no device. */
- (instancetype)capture;

/* Copies the state of another profile into this one, unless this profile belongs to a real device. */
- (void)setStateFromPhysicalInput:(GCPhysicalInputProfile *)physicalInput;

/* The element alias a physical input name maps to, and the names mapped to an alias (identity here:
 * there is no user remapping). */
- (NSString *)mappedElementAliasForPhysicalInputName:(NSString *)inputName;
- (NSSet<NSString *> *)mappedPhysicalInputNamesForElementAlias:(NSString *)elementAlias;

@end

NS_ASSUME_NONNULL_END

#endif
