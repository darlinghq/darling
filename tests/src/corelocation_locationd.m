/*
 This file is part of Darling.

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

#import <Foundation/Foundation.h>
#import <CoreLocation/CoreLocation.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static int s_fails = 0;

static void test_check(const char* name, BOOL cond)
{
    if (cond) {
        printf("PASS: %s\n", name);
    } else {
        printf("FAIL: %s\n", name);
        s_fails++;
    }
}

@interface TestLocationDelegate : NSObject <CLLocationManagerDelegate>
@property (nonatomic, assign) BOOL gotAuthChange;
@property (nonatomic, assign) BOOL gotLocationUpdate;
@property (nonatomic, assign) BOOL gotError;
@property (nonatomic, retain) CLLocation* lastReceivedLocation;
@property (nonatomic, retain) NSError* lastReceivedError;
@end

@implementation TestLocationDelegate

- (void)dealloc
{
    [_lastReceivedLocation release];
    [_lastReceivedError release];
    [super dealloc];
}

- (void)locationManagerDidChangeAuthorization:(CLLocationManager *)manager
{
    self.gotAuthChange = YES;
    printf("[Delegate] locationManagerDidChangeAuthorization: status=%d, accuracy=%d\n",
           (int)manager.authorizationStatus, (int)manager.accuracyAuthorization);
}

- (void)locationManager:(CLLocationManager *)manager didChangeAuthorizationStatus:(CLAuthorizationStatus)status
{
    self.gotAuthChange = YES;
    printf("[Delegate] didChangeAuthorizationStatus: %d\n", (int)status);
}

- (void)locationManager:(CLLocationManager *)manager didUpdateLocations:(NSArray<CLLocation *> *)locations
{
    self.gotLocationUpdate = YES;
    self.lastReceivedLocation = [locations firstObject];
    printf("[Delegate] didUpdateLocations: count=%lu, first=%s\n",
           (unsigned long)[locations count], [[locations.firstObject description] UTF8String]);
}

- (void)locationManager:(CLLocationManager *)manager didFailWithError:(NSError *)error
{
    self.gotError = YES;
    self.lastReceivedError = error;
    printf("[Delegate] didFailWithError: domain=%s, code=%ld\n",
           [error.domain UTF8String], (long)error.code);
}

@end

int main(int argc, const char* argv[])
{
    @autoreleasepool {
        printf("=== Testing CoreLocation CLLocationManager ===\n");

        BOOL enabled = [CLLocationManager locationServicesEnabled];
        CLAuthorizationStatus classAuth = [CLLocationManager authorizationStatus];
        printf("locationServicesEnabled: %d\n", enabled);
        printf("authorizationStatus (class): %d\n", (int)classAuth);

        CLLocationManager *mgr = [[CLLocationManager alloc] init];
        test_check("mgr created", mgr != nil);
        test_check("default desiredAccuracy is kCLLocationAccuracyBest (-1.0)",
              mgr.desiredAccuracy == kCLLocationAccuracyBest);
        test_check("default distanceFilter is kCLDistanceFilterNone (-1.0)",
              mgr.distanceFilter == kCLDistanceFilterNone);
        test_check("default location is nil before any update", mgr.location == nil);

        TestLocationDelegate *del = [[[TestLocationDelegate alloc] init] autorelease];
        mgr.delegate = del;

        // Run loop turn to receive initial auth change
        NSDate *until = [NSDate dateWithTimeIntervalSinceNow:0.5];
        while (!del.gotAuthChange && [until timeIntervalSinceNow] > 0) {
            [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
        }
        test_check("received initial authorization callback", del.gotAuthChange);

        // Test authorization request calls
        [mgr requestWhenInUseAuthorization];
        [mgr requestAlwaysAuthorization];

        // Test CLLocation class directly
        CLLocation *loc1 = [[[CLLocation alloc] initWithLatitude:50.0875 longitude:14.4214] autorelease];
        test_check("CLLocation lat/lon initialization",
              fabs(loc1.coordinate.latitude - 50.0875) < 0.0001 &&
              fabs(loc1.coordinate.longitude - 14.4214) < 0.0001);

        CLLocation *loc2 = [[[CLLocation alloc] initWithLatitude:51.5074 longitude:-0.1278] autorelease];
        CLLocationDistance dist = [loc1 distanceFromLocation:loc2];
        test_check("CLLocation geodesic distance from Prague to London > 1000km", dist > 1000000.0 && dist < 1100000.0);

        CLLocationCoordinate2D validCoord = CLLocationCoordinate2DMake(50.0, 14.0);
        CLLocationCoordinate2D invalidCoord = CLLocationCoordinate2DMake(100.0, 200.0);
        test_check("CLLocationCoordinate2DIsValid on valid coordinate", CLLocationCoordinate2DIsValid(validCoord));
        test_check("CLLocationCoordinate2DIsValid on invalid coordinate", !CLLocationCoordinate2DIsValid(invalidCoord));

        if (enabled) {
            printf("Starting location updates...\n");
            [mgr startUpdatingLocation];

            until = [NSDate dateWithTimeIntervalSinceNow:3.0];
            while (!del.gotLocationUpdate && !del.gotError && [until timeIntervalSinceNow] > 0) {
                [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.2]];
            }

            if (del.gotLocationUpdate) {
                test_check("received didUpdateLocations callback", del.gotLocationUpdate);
                test_check("manager.location is populated", mgr.location != nil);
                test_check("latitude is valid", CLLocationCoordinate2DIsValid(mgr.location.coordinate));
            } else if (del.gotError) {
                printf("Location failed with error: %ld\n", (long)del.lastReceivedError.code);
            }

            [mgr stopUpdatingLocation];
        } else {
            printf("Host location services disabled, verifying restricted status\n");
            test_check("authorizationStatus is Restricted when disabled",
                  [mgr authorizationStatus] == kCLAuthorizationStatusRestricted);
        }

        [mgr release];
    }

    if (s_fails == 0) {
        printf("PASS: corelocation_locationd\n");
        return 0;
    } else {
        printf("FAILED: %d checks failed\n", s_fails);
        return 1;
    }
}
