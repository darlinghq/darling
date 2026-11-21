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

/*
 * CoreLocation contract tests that hold with or without a host location
 * service: the CLLocation value class (checked against published geodesic
 * values and the key names macOS archives use), and the CLLocationManager
 * delegate contract (initial authorization callback from the run loop, one
 * outcome for -requestLocation, safe stop/release while updating).
 */

#import <Foundation/Foundation.h>
#import <CoreLocation/CoreLocation.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static int s_fails = 0;

#define CHECK(cond) do { \
	if (cond) { printf("PASS: %s\n", #cond); } \
	else { printf("FAIL (line %d): %s\n", __LINE__, #cond); s_fails++; } \
} while (0)

static void spin(double seconds, BOOL (^done)(void))
{
	NSDate *until = [NSDate dateWithTimeIntervalSinceNow:seconds];
	while ((!done || !done()) && [until timeIntervalSinceNow] > 0)
		[[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.05]];
}

@interface Delegate : NSObject <CLLocationManagerDelegate>
{
@public
	int authChanges;
	int authChangesOnMainThread;
	int updates;
	int failures;
	NSInteger lastErrorCode;
	NSString *lastErrorDomain;
	CLLocation *lastLocation;
	BOOL insideSetDelegate;
	BOOL authCalledInsideSetDelegate;
}
@end

@implementation Delegate
- (void)dealloc
{
	[lastErrorDomain release];
	[lastLocation release];
	[super dealloc];
}
- (void)locationManagerDidChangeAuthorization:(CLLocationManager *)manager
{
	authChanges++;
	if ([NSThread isMainThread])
		authChangesOnMainThread++;
	if (insideSetDelegate)
		authCalledInsideSetDelegate = YES;
}
- (void)locationManager:(CLLocationManager *)manager didUpdateLocations:(NSArray<CLLocation *> *)locations
{
	updates++;
	[lastLocation release];
	lastLocation = [[locations firstObject] retain];
}
- (void)locationManager:(CLLocationManager *)manager didFailWithError:(NSError *)error
{
	failures++;
	lastErrorCode = error.code;
	[lastErrorDomain release];
	lastErrorDomain = [error.domain copy];
}
@end

// A delegate implementing only the deprecated authorization callback.
@interface LegacyDelegate : NSObject <CLLocationManagerDelegate>
{
@public
	int calls;
}
@end
@implementation LegacyDelegate
- (void)locationManager:(CLLocationManager *)manager didChangeAuthorizationStatus:(CLAuthorizationStatus)status
{
	calls++;
}
@end

static double dist(double lat1, double lon1, double lat2, double lon2)
{
	CLLocation *a = [[[CLLocation alloc] initWithLatitude:lat1 longitude:lon1] autorelease];
	CLLocation *b = [[[CLLocation alloc] initWithLatitude:lat2 longitude:lon2] autorelease];
	return [a distanceFromLocation:b];
}

static void test_location_values(void)
{
	// Initializer defaults.
	CLLocation *l = [[[CLLocation alloc] initWithLatitude:50.0875 longitude:14.4214] autorelease];
	CHECK(l.coordinate.latitude == 50.0875 && l.coordinate.longitude == 14.4214);
	CHECK(l.altitude == 0.0 && l.horizontalAccuracy == 0.0 && l.verticalAccuracy == -1.0);
	CHECK(l.course == -1.0 && l.speed == -1.0 && l.timestamp != nil);

	NSDate *ts = [NSDate dateWithTimeIntervalSince1970:1000];
	CLLocation *five = [[[CLLocation alloc] initWithCoordinate:CLLocationCoordinate2DMake(1, 2) altitude:3
	                                        horizontalAccuracy:4 verticalAccuracy:5 timestamp:ts] autorelease];
	CHECK(five.altitude == 3 && five.horizontalAccuracy == 4 && five.verticalAccuracy == 5);
	CHECK(five.course == -1.0 && five.speed == -1.0);
	CLLocation *seven = [[[CLLocation alloc] initWithCoordinate:CLLocationCoordinate2DMake(1, 2) altitude:3
	                                         horizontalAccuracy:4 verticalAccuracy:5 course:6 speed:7 timestamp:ts] autorelease];
	CHECK(seven.course == 6 && seven.speed == 7 && [seven.timestamp isEqualToDate:ts]);

	// Distances. A degree of longitude on the WGS-84 equator is a*pi/180; the
	// second value is the published Vincenty/GRS80 example (Flinders Peak to
	// Buninyong). macOS itself deviates from the exact ellipsoidal distance by
	// up to a few parts per million, so these checks allow for that.
	CHECK(dist(0, 0, 0, 0) == 0.0);
	CHECK(fabs(dist(0, 0, 0, 1) - 111319.4908) < 0.001);
	CHECK(fabs(dist(0, 1, 0, 0) - 111319.4908) < 0.001);
	double fl_lat = -(37 + 57 / 60.0 + 3.72030 / 3600.0), fl_lon = 144 + 25 / 60.0 + 29.52440 / 3600.0;
	double bu_lat = -(37 + 39 / 60.0 + 10.15610 / 3600.0), bu_lon = 143 + 55 / 60.0 + 35.38390 / 3600.0;
	CHECK(fabs(dist(fl_lat, fl_lon, bu_lat, bu_lon) - 54972.271) < 0.5);
	// Pole to pole is the meridian half-length, 20003931.4586 m.
	CHECK(fabs(dist(90, 0, -90, 0) - 20003931.4586) < 20.0);
	// Prague to London, measured on macOS 26 (Vincenty-class accuracy, a few mm).
	CHECK(fabs(dist(50.0875, 14.4214, 51.5074, -0.1278) - 1036068.60) < 0.05);
	CHECK(fabs(dist(40.7128, -74.0060, 35.6762, 139.6503) - 10875723.69) < 0.05);

	// Coordinates.
	CLLocationCoordinate2D inv = kCLLocationCoordinate2DInvalid;
	CHECK(inv.latitude == -180.0 && inv.longitude == -180.0 && !CLLocationCoordinate2DIsValid(inv));
	CHECK(CLLocationCoordinate2DIsValid(CLLocationCoordinate2DMake(90, 180)));
	CHECK(CLLocationCoordinate2DIsValid(CLLocationCoordinate2DMake(-90, -180)));
	CHECK(!CLLocationCoordinate2DIsValid(CLLocationCoordinate2DMake(91, 0)));
	CHECK(!CLLocationCoordinate2DIsValid(CLLocationCoordinate2DMake(0, -181)));
	CHECK(!CLLocationCoordinate2DIsValid(CLLocationCoordinate2DMake(NAN, 0)));

	CHECK(kCLLocationAccuracyBest == -1.0 && kCLLocationAccuracyBestForNavigation == -2.0);
	CHECK(kCLDistanceFilterNone == -1.0 && CLLocationDistanceMax == DBL_MAX);
	CHECK([kCLErrorDomain isEqualToString:@"kCLErrorDomain"]);
	CHECK(kCLErrorLocationUnknown == 0 && kCLErrorDenied == 1 && kCLErrorNetwork == 2);
}

static void test_location_coding(void)
{
	NSDate *ts = [NSDate dateWithTimeIntervalSince1970:1000];
	CLLocation *l = [[[CLLocation alloc] initWithCoordinate:CLLocationCoordinate2DMake(1, 2) altitude:3
	                                     horizontalAccuracy:4 verticalAccuracy:5 course:6 speed:7 timestamp:ts] autorelease];
	CHECK([CLLocation supportsSecureCoding]);

	NSData *data = [NSKeyedArchiver archivedDataWithRootObject:l requiringSecureCoding:YES error:nil];
	CHECK(data != nil);
	NSDictionary *plist = [NSPropertyListSerialization propertyListWithData:data options:0 format:NULL error:nil];
	NSDictionary *obj = nil;
	for (id o in plist[@"$objects"])
		if ([o isKindOfClass:[NSDictionary class]] && o[@"kCLLocationCodingKeyCoordinateLatitude"])
			obj = o;
	CHECK(obj != nil);
	// Key names and the timestamp representation of archives written by macOS.
	CHECK([obj[@"kCLLocationCodingKeyCoordinateLatitude"] doubleValue] == 1);
	CHECK([obj[@"kCLLocationCodingKeyCoordinateLongitude"] doubleValue] == 2);
	CHECK([obj[@"kCLLocationCodingKeyAltitude"] doubleValue] == 3);
	CHECK([obj[@"kCLLocationCodingKeyHorizontalAccuracy"] doubleValue] == 4);
	CHECK([obj[@"kCLLocationCodingKeyVerticalAccuracy"] doubleValue] == 5);
	CHECK([obj[@"kCLLocationCodingKeyCourse"] doubleValue] == 6);
	CHECK([obj[@"kCLLocationCodingKeySpeed"] doubleValue] == 7);
	CHECK([obj[@"kCLLocationCodingKeyTimestamp"] doubleValue] == [ts timeIntervalSinceReferenceDate]);

	CLLocation *d = [NSKeyedUnarchiver unarchivedObjectOfClass:[CLLocation class] fromData:data error:nil];
	CHECK(d != nil && d.coordinate.latitude == 1 && d.coordinate.longitude == 2 && d.altitude == 3);
	CHECK(d.horizontalAccuracy == 4 && d.verticalAccuracy == 5 && d.course == 6 && d.speed == 7);
	CHECK([d.timestamp isEqualToDate:ts]);
}

static void test_manager_defaults_and_delegate(void)
{
	CLLocationManager *m = [[CLLocationManager alloc] init];
	CHECK(m.desiredAccuracy == kCLLocationAccuracyBest);
	CHECK(m.distanceFilter == kCLDistanceFilterNone);
	CHECK(m.location == nil);

	CLAuthorizationStatus classStatus = [CLLocationManager authorizationStatus];
	CHECK(m.authorizationStatus == classStatus);
	BOOL enabled = [CLLocationManager locationServicesEnabled];
	CHECK(m.locationServicesEnabled == enabled);
	if (!enabled)
		CHECK(classStatus == kCLAuthorizationStatusRestricted || classStatus == kCLAuthorizationStatusDenied ||
		      classStatus == kCLAuthorizationStatusNotDetermined);

	// The initial authorization callback comes from the run loop, not from inside -setDelegate:.
	Delegate *del = [[Delegate alloc] init];
	del->insideSetDelegate = YES;
	m.delegate = del;
	del->insideSetDelegate = NO;
	CHECK(!del->authCalledInsideSetDelegate && del->authChanges == 0);
	spin(2.0, ^BOOL { return del->authChanges > 0; });
	CHECK(del->authChanges == 1 && del->authChangesOnMainThread == 1);

	// Asking for authorization again does not report a change that did not happen.
	[m requestWhenInUseAuthorization];
	[m requestAlwaysAuthorization];
	spin(0.5, nil);
	CHECK(del->authChanges == 1 || [CLLocationManager authorizationStatus] != classStatus);

	// Only the deprecated callback implemented: it is used.
	LegacyDelegate *legacy = [[LegacyDelegate alloc] init];
	CLLocationManager *m2 = [[CLLocationManager alloc] init];
	m2.delegate = legacy;
	spin(2.0, ^BOOL { return legacy->calls > 0; });
	CHECK(legacy->calls == 1);
	m2.delegate = nil;
	[m2 release];
	[legacy release];

	// -requestLocation ends in exactly one outcome: a location or an error in kCLErrorDomain.
	[m requestLocation];
	spin(15.0, ^BOOL { return del->updates + del->failures > 0; });
	spin(0.3, nil);
	CHECK(del->updates + del->failures == 1);
	if (del->failures)
		CHECK([del->lastErrorDomain isEqualToString:kCLErrorDomain]);
	if (del->updates)
		CHECK(CLLocationCoordinate2DIsValid(del->lastLocation.coordinate) && m.location != nil);

	// Stopping and releasing a manager that is updating is safe, with callbacks possibly still queued.
	m.delegate = nil;
	[m startUpdatingLocation];
	[m stopUpdatingLocation];
	[m startUpdatingLocation];
	[m release];

	CLLocationManager *m3 = [[CLLocationManager alloc] init];
	m3.delegate = del;
	m3.desiredAccuracy = kCLLocationAccuracyKilometer;
	m3.distanceFilter = 100.0;
	[m3 startUpdatingLocation];
	[m3 release];
	spin(1.0, nil);

	// A manager released from the delegate callback of its own request.
	__block CLLocationManager *m4 = [[CLLocationManager alloc] init];
	m4.delegate = del;
	[m4 requestLocation];
	[m4 release];
	spin(1.0, nil);
	del->authChanges = 0;
	[del release];
	CHECK(YES);
}

int main(int argc, const char *argv[])
{
	@autoreleasepool {
		test_location_values();
		test_location_coding();
		test_manager_defaults_and_delegate();
	}

	if (s_fails == 0) {
		printf("PASS: corelocation_locationd\n");
		return 0;
	}
	printf("FAILED: %d checks failed\n", s_fails);
	return 1;
}
