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

#ifndef _GCMotion_H_
#define _GCMotion_H_

#import <Foundation/Foundation.h>

@class GCController;

NS_ASSUME_NONNULL_BEGIN

typedef struct {
	double x, y, z;
} GCAcceleration;

typedef struct {
	double x, y, z;
} GCRotationRate;

typedef struct {
	double pitch, yaw, roll;
} GCEulerAngles;

typedef struct GCQuaternion {
	double x, y, z, w;
} GCQuaternion;

/* The motion sensors of a controller. */
@interface GCMotion : NSObject

@property (nonatomic, readonly, weak) GCController *controller;

typedef void (^GCMotionValueChangedHandler)(GCMotion *motion);

/* Called after the motion data changed. */
@property (nonatomic, copy, nullable) GCMotionValueChangedHandler valueChangedHandler;

/* Whether the application has to switch the sensors on (sensorsActive) before they report. */
@property (nonatomic, readonly) BOOL sensorsRequireManualActivation;
@property (nonatomic) BOOL sensorsActive;

@property (nonatomic, readonly) BOOL hasGravityAndUserAcceleration;
@property (nonatomic, readonly) GCAcceleration gravity;
@property (nonatomic, readonly) GCAcceleration userAcceleration;
@property (nonatomic, readonly) GCAcceleration acceleration;

@property (nonatomic, readonly) BOOL hasAttitudeAndRotationRate;
@property (nonatomic, readonly) BOOL hasAttitude;
@property (nonatomic, readonly) BOOL hasRotationRate;
@property (nonatomic, readonly) GCQuaternion attitude;
@property (nonatomic, readonly) GCRotationRate rotationRate;

/* Write the motion data of a controller created by the application. */
- (void)setGravity:(GCAcceleration)gravity;
- (void)setUserAcceleration:(GCAcceleration)userAcceleration;
- (void)setAcceleration:(GCAcceleration)acceleration;
- (void)setAttitude:(GCQuaternion)attitude;
- (void)setRotationRate:(GCRotationRate)rotationRate;
- (void)setStateFromMotion:(GCMotion *)motion;

@end

NS_ASSUME_NONNULL_END

#endif
