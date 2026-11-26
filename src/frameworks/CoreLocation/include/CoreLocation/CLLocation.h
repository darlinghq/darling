/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel
 Copyright (C) 2026 Darling developers

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

#ifndef _CoreLocation_CLLocation_H_
#define _CoreLocation_CLLocation_H_

#import <Foundation/Foundation.h>

typedef double CLLocationAccuracy;
typedef double CLLocationDegrees;
typedef double CLLocationDistance;
typedef double CLLocationDirection;
typedef double CLLocationSpeed;
typedef double CLLocationDirectionAccuracy;
typedef double CLLocationSpeedAccuracy;

typedef struct CLLocationCoordinate2D
{
    CLLocationDegrees latitude;
    CLLocationDegrees longitude;
} CLLocationCoordinate2D;

#ifdef __cplusplus
extern "C" {
#endif

static inline CLLocationCoordinate2D CLLocationCoordinate2DMake(CLLocationDegrees latitude, CLLocationDegrees longitude)
{
    CLLocationCoordinate2D coordinate;
    coordinate.latitude = latitude;
    coordinate.longitude = longitude;
    return coordinate;
}

// YES when the latitude is within -90...90 and the longitude within -180...180 (both ends included).
extern BOOL CLLocationCoordinate2DIsValid(CLLocationCoordinate2D coord);

extern const CLLocationDistance kCLDistanceFilterNone;
extern const CLLocationDistance CLLocationDistanceMax;
extern const NSTimeInterval CLTimeIntervalMax;
extern const CLLocationCoordinate2D kCLLocationCoordinate2DInvalid;

extern const CLLocationAccuracy kCLLocationAccuracyBestForNavigation;
extern const CLLocationAccuracy kCLLocationAccuracyBest;
extern const CLLocationAccuracy kCLLocationAccuracyNearestTenMeters;
extern const CLLocationAccuracy kCLLocationAccuracyHundredMeters;
extern const CLLocationAccuracy kCLLocationAccuracyKilometer;
extern const CLLocationAccuracy kCLLocationAccuracyThreeKilometers;
extern const CLLocationAccuracy kCLLocationAccuracyReduced;

#ifdef __cplusplus
}
#endif

// The floor of a building a location is on (macOS 10.15).
@interface CLFloor : NSObject <NSCopying, NSSecureCoding>
{
@private
    NSInteger _level;
}
// The floor's number, 0 for the ground floor, negative below it.
@property(readonly, nonatomic) NSInteger level;
@end

@interface CLLocation : NSObject <NSCopying, NSSecureCoding>
{
@private
    CLLocationCoordinate2D _coordinate;
    CLLocationDistance _altitude;
    CLLocationAccuracy _horizontalAccuracy;
    CLLocationAccuracy _verticalAccuracy;
    CLLocationDirection _course;
    CLLocationSpeed _speed;
    NSDate *_timestamp;
}

- (instancetype)initWithLatitude:(CLLocationDegrees)latitude longitude:(CLLocationDegrees)longitude;
- (instancetype)initWithCoordinate:(CLLocationCoordinate2D)coordinate
                          altitude:(CLLocationDistance)altitude
                horizontalAccuracy:(CLLocationAccuracy)hAccuracy
                  verticalAccuracy:(CLLocationAccuracy)vAccuracy
                         timestamp:(NSDate *)timestamp;
- (instancetype)initWithCoordinate:(CLLocationCoordinate2D)coordinate
                          altitude:(CLLocationDistance)altitude
                horizontalAccuracy:(CLLocationAccuracy)hAccuracy
                  verticalAccuracy:(CLLocationAccuracy)vAccuracy
                            course:(CLLocationDirection)course
                             speed:(CLLocationSpeed)speed
                         timestamp:(NSDate *)timestamp;

@property(readonly, nonatomic) CLLocationCoordinate2D coordinate;
@property(readonly, nonatomic) CLLocationDistance altitude;
@property(readonly, nonatomic) CLLocationAccuracy horizontalAccuracy;
@property(readonly, nonatomic) CLLocationAccuracy verticalAccuracy;
@property(readonly, nonatomic) CLLocationDirection course;
@property(readonly, nonatomic) CLLocationSpeed speed;
@property(readonly, nonatomic, copy) NSDate *timestamp;
// The floor of the building the location is on, or nil if that is unknown
// (it always is: Darling has no indoor positioning).
@property(readonly, nonatomic, copy) CLFloor *floor;

- (CLLocationDistance)distanceFromLocation:(const CLLocation *)location;
- (CLLocationDistance)getDistanceFrom:(const CLLocation *)location;

@end

#endif
