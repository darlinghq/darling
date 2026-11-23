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

#ifndef _CoreLocation_CLLocationManager_H_
#define _CoreLocation_CLLocationManager_H_

#import <Foundation/Foundation.h>
#import <CoreLocation/CLLocation.h>
#import <CoreLocation/CLError.h>

@class CLLocationManager;

typedef NS_ENUM(int, CLAuthorizationStatus) {
	kCLAuthorizationStatusNotDetermined = 0,
	kCLAuthorizationStatusRestricted,
	kCLAuthorizationStatusDenied,
	kCLAuthorizationStatusAuthorizedAlways,
	kCLAuthorizationStatusAuthorizedWhenInUse,
	kCLAuthorizationStatusAuthorized = kCLAuthorizationStatusAuthorizedAlways,
};

typedef NS_ENUM(NSInteger, CLAccuracyAuthorization) {
	CLAccuracyAuthorizationFullAccuracy = 0,
	CLAccuracyAuthorizationReducedAccuracy = 1,
};

@protocol CLLocationManagerDelegate <NSObject>
@optional
- (void)locationManager:(CLLocationManager *)manager didUpdateToLocation:(CLLocation *)newLocation fromLocation:(CLLocation *)oldLocation;
- (void)locationManager:(CLLocationManager *)manager didUpdateLocations:(NSArray<CLLocation *> *)locations;
- (void)locationManager:(CLLocationManager *)manager didFailWithError:(NSError *)error;
- (void)locationManager:(CLLocationManager *)manager didChangeAuthorizationStatus:(CLAuthorizationStatus)status;
- (void)locationManagerDidChangeAuthorization:(CLLocationManager *)manager;
@end

@interface CLLocationManager : NSObject
{
@private
	id<CLLocationManagerDelegate> _delegate;
	CLLocationDistance _distanceFilter;
	CLLocationAccuracy _desiredAccuracy;
	CLAuthorizationStatus _authorizationStatus;
	CLAccuracyAuthorization _accuracyAuthorization;
	id _clientToken;
}

// Not retained, like other Cocoa delegates.
@property (nonatomic, assign) id<CLLocationManagerDelegate> delegate;
@property (nonatomic, assign) CLLocationDistance distanceFilter;
@property (nonatomic, assign) CLLocationAccuracy desiredAccuracy;
@property (readonly, nonatomic, copy) CLLocation *location;

@property (nonatomic, readonly) CLAuthorizationStatus authorizationStatus;
@property (nonatomic, readonly) CLAccuracyAuthorization accuracyAuthorization;
@property (readonly, nonatomic) BOOL locationServicesEnabled;

+ (CLAuthorizationStatus)authorizationStatus;
+ (BOOL)locationServicesEnabled;

- (void)requestWhenInUseAuthorization;
- (void)requestAlwaysAuthorization;

- (void)startUpdatingLocation;
- (void)stopUpdatingLocation;
- (void)requestLocation;

@end

#endif
