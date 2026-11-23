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

typedef GCControllerElement GCDeviceElement;
typedef GCControllerButtonInput GCDeviceButtonInput;
typedef GCControllerAxisInput GCDeviceAxisInput;
typedef GCControllerDirectionPad GCDeviceDirectionPad;

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

@property (nonatomic, readonly, strong) NSSet<GCDeviceElement *> *allElements;
@property (nonatomic, readonly, strong) NSSet<GCDeviceButtonInput *> *allButtons;
@property (nonatomic, readonly, strong) NSSet<GCDeviceAxisInput *> *allAxes;
@property (nonatomic, readonly, strong) NSSet<GCDeviceDirectionPad *> *allDpads;

- (nullable __kindof GCDeviceElement *)objectForKeyedSubscript:(NSString *)key;

@end

NS_ASSUME_NONNULL_END

#endif
