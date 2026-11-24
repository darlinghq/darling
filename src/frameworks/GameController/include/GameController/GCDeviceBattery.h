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

#ifndef _GCDeviceBattery_H_
#define _GCDeviceBattery_H_

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSInteger, GCDeviceBatteryState) {
	GCDeviceBatteryStateUnknown = -1,
	GCDeviceBatteryStateDischarging = 0,
	GCDeviceBatteryStateCharging = 1,
	GCDeviceBatteryStateFull = 2,
};

@interface GCDeviceBattery : NSObject

@property (nonatomic, readonly) float batteryLevel;
@property (nonatomic, readonly) GCDeviceBatteryState batteryState;

- (instancetype)initWithLevel:(float)level batteryState:(GCDeviceBatteryState)state;

@end

NS_ASSUME_NONNULL_END

#endif
