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

#import <CoreLocation/CLLocation.h>
#import <CoreLocation/CLErrorDomain.h>
#import <darling-stub-forwarding.h>
#include <float.h>
#include <math.h>

const CLLocationDistance kCLDistanceFilterNone = -1.0;
const CLLocationAccuracy kCLLocationAccuracyBestForNavigation = -2.0;
const CLLocationAccuracy kCLLocationAccuracyBest = -1.0;
const CLLocationAccuracy kCLLocationAccuracyNearestTenMeters = 10.0;
const CLLocationAccuracy kCLLocationAccuracyHundredMeters = 100.0;
const CLLocationAccuracy kCLLocationAccuracyKilometer = 1000.0;
const CLLocationAccuracy kCLLocationAccuracyThreeKilometers = 3000.0;
const CLLocationAccuracy kCLLocationAccuracyReduced = 6380000.0;
const CLLocationDistance CLLocationDistanceMax = DBL_MAX;
const NSTimeInterval CLTimeIntervalMax = DBL_MAX;
const CLLocationCoordinate2D kCLLocationCoordinate2DInvalid = { -180.0, -180.0 };

NSString *const kCLErrorDomain = @"kCLErrorDomain";

#define WGS84_A 6378137.0
#define WGS84_F (1.0 / 298.257223563)
#define WGS84_B ((1.0 - WGS84_F) * WGS84_A)
#define DEG_TO_RAD (M_PI / 180.0)

static double haversineDistance(double lat1, double lon1, double lat2, double lon2)
{
    double dlat = (lat2 - lat1) * DEG_TO_RAD;
    double dlon = (lon2 - lon1) * DEG_TO_RAD;
    double phi1 = lat1 * DEG_TO_RAD;
    double phi2 = lat2 * DEG_TO_RAD;

    double a = sin(dlat / 2.0) * sin(dlat / 2.0) +
               cos(phi1) * cos(phi2) * sin(dlon / 2.0) * sin(dlon / 2.0);
    double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
    return 6371009.0 * c;
}

static double vincentyDistance(double lat1, double lon1, double lat2, double lon2)
{
    if (lat1 == lat2 && lon1 == lon2)
        return 0.0;

    double phi1 = lat1 * DEG_TO_RAD;
    double lambda1 = lon1 * DEG_TO_RAD;
    double phi2 = lat2 * DEG_TO_RAD;
    double lambda2 = lon2 * DEG_TO_RAD;

    double U1 = atan((1.0 - WGS84_F) * tan(phi1));
    double U2 = atan((1.0 - WGS84_F) * tan(phi2));
    double L = lambda2 - lambda1;
    double Lambda = L;

    double sinU1 = sin(U1), cosU1 = cos(U1);
    double sinU2 = sin(U2), cosU2 = cos(U2);

    double sinSigma = 0.0, cosSigma = 0.0, sigma = 0.0;
    double sinAlpha = 0.0, cos2Alpha = 0.0, cos2SigmaM = 0.0;

    int converged = 0;
    for (int iter = 0; iter < 100; iter++)
    {
        double sinLambda = sin(Lambda), cosLambda = cos(Lambda);
        sinSigma = sqrt((cosU2 * sinLambda) * (cosU2 * sinLambda) +
                        (cosU1 * sinU2 - sinU1 * cosU2 * cosLambda) *
                        (cosU1 * sinU2 - sinU1 * cosU2 * cosLambda));
        if (sinSigma == 0.0)
            return 0.0;

        cosSigma = sinU1 * sinU2 + cosU1 * cosU2 * cosLambda;
        sigma = atan2(sinSigma, cosSigma);

        sinAlpha = (cosU1 * cosU2 * sinLambda) / sinSigma;
        cos2Alpha = 1.0 - sinAlpha * sinAlpha;
        cos2SigmaM = (cos2Alpha != 0.0) ? (cosSigma - 2.0 * sinU1 * sinU2 / cos2Alpha) : 0.0;

        double C = (WGS84_F / 16.0) * cos2Alpha * (4.0 + WGS84_F * (4.0 - 3.0 * cos2Alpha));
        double LambdaPrev = Lambda;
        Lambda = L + (1.0 - C) * WGS84_F * sinAlpha *
                 (sigma + C * sinSigma * (cos2SigmaM + C * cosSigma * (-1.0 + 2.0 * cos2SigmaM * cos2SigmaM)));

        if (fabs(Lambda - LambdaPrev) < 1e-12)
        {
            converged = 1;
            break;
        }
    }

    if (!converged)
        return haversineDistance(lat1, lon1, lat2, lon2);

    double u2 = cos2Alpha * (WGS84_A * WGS84_A - WGS84_B * WGS84_B) / (WGS84_B * WGS84_B);
    double A = 1.0 + (u2 / 16384.0) * (4096.0 + u2 * (-768.0 + u2 * (320.0 - 175.0 * u2)));
    double B = (u2 / 1024.0) * (256.0 + u2 * (-128.0 + u2 * (74.0 - 47.0 * u2)));
    double deltaSigma = B * sinSigma * (cos2SigmaM + (B / 4.0) * (cosSigma * (-1.0 + 2.0 * cos2SigmaM * cos2SigmaM) -
                        (B / 6.0) * cos2SigmaM * (-3.0 + 4.0 * sinSigma * sinSigma) *
                        (-3.0 + 4.0 * cos2SigmaM * cos2SigmaM)));

    return WGS84_B * A * (sigma - deltaSigma);
}

@implementation CLLocation

@synthesize coordinate = _coordinate;
@synthesize altitude = _altitude;
@synthesize horizontalAccuracy = _horizontalAccuracy;
@synthesize verticalAccuracy = _verticalAccuracy;
@synthesize course = _course;
@synthesize speed = _speed;
@synthesize timestamp = _timestamp;

- (instancetype)initWithLatitude:(CLLocationDegrees)latitude longitude:(CLLocationDegrees)longitude
{
    return [self initWithCoordinate:CLLocationCoordinate2DMake(latitude, longitude)
                           altitude:0.0
                 horizontalAccuracy:0.0
                   verticalAccuracy:-1.0
                             course:-1.0
                              speed:-1.0
                          timestamp:[NSDate date]];
}

- (instancetype)initWithCoordinate:(CLLocationCoordinate2D)coordinate
                          altitude:(CLLocationDistance)altitude
                horizontalAccuracy:(CLLocationAccuracy)hAccuracy
                  verticalAccuracy:(CLLocationAccuracy)vAccuracy
                         timestamp:(NSDate *)timestamp
{
    return [self initWithCoordinate:coordinate
                           altitude:altitude
                 horizontalAccuracy:hAccuracy
                   verticalAccuracy:vAccuracy
                             course:-1.0
                              speed:-1.0
                          timestamp:timestamp];
}

- (instancetype)initWithCoordinate:(CLLocationCoordinate2D)coordinate
                          altitude:(CLLocationDistance)altitude
                horizontalAccuracy:(CLLocationAccuracy)hAccuracy
                  verticalAccuracy:(CLLocationAccuracy)vAccuracy
                            course:(CLLocationDirection)course
                             speed:(CLLocationSpeed)speed
                         timestamp:(NSDate *)timestamp
{
    self = [super init];
    if (self)
    {
        _coordinate = coordinate;
        _altitude = altitude;
        _horizontalAccuracy = hAccuracy;
        _verticalAccuracy = vAccuracy;
        _course = course;
        _speed = speed;
        _timestamp = timestamp ? [timestamp copy] : [[NSDate alloc] init];
    }
    return self;
}

- (void)dealloc
{
    [_timestamp release];
    [super dealloc];
}

- (CLLocationDistance)distanceFromLocation:(const CLLocation *)location
{
    if (!location)
        return -1.0;

    return vincentyDistance(_coordinate.latitude, _coordinate.longitude,
                            location.coordinate.latitude, location.coordinate.longitude);
}

- (id)copyWithZone:(NSZone *)zone
{
    return [self retain];
}

- (BOOL)isEqual:(id)object
{
    if (self == object)
        return YES;
    if (![object isKindOfClass:[CLLocation class]])
        return NO;

    CLLocation *other = (CLLocation *)object;
    return _coordinate.latitude == other.coordinate.latitude &&
           _coordinate.longitude == other.coordinate.longitude &&
           _altitude == other.altitude &&
           _horizontalAccuracy == other.horizontalAccuracy &&
           _verticalAccuracy == other.verticalAccuracy &&
           _course == other.course &&
           _speed == other.speed &&
           [_timestamp isEqualToDate:other.timestamp];
}

- (NSUInteger)hash
{
    return (NSUInteger)(_coordinate.latitude * 1000.0) ^
           (NSUInteger)(_coordinate.longitude * 1000.0) ^
           [_timestamp hash];
}

- (NSString *)description
{
    char latSign = (_coordinate.latitude >= 0) ? '+' : '-';
    char lonSign = (_coordinate.longitude >= 0) ? '+' : '-';
    return [NSString stringWithFormat:@"<%c%.8f,%c%.8f> +/- %.2fm (speed %.2f mps / course %.2f) @ %@",
            latSign, fabs(_coordinate.latitude),
            lonSign, fabs(_coordinate.longitude),
            _horizontalAccuracy, _speed, _course, _timestamp];
}

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    // The key names and the representation of the timestamp (seconds since the
    // reference date, as a double) are those of the archives macOS writes. The
    // "Raw" keys carry the same values since no filtering is applied here.
    [coder encodeDouble:_coordinate.latitude forKey:@"kCLLocationCodingKeyCoordinateLatitude"];
    [coder encodeDouble:_coordinate.longitude forKey:@"kCLLocationCodingKeyCoordinateLongitude"];
    [coder encodeDouble:_coordinate.latitude forKey:@"kCLLocationCodingKeyRawCoordinateLatitude"];
    [coder encodeDouble:_coordinate.longitude forKey:@"kCLLocationCodingKeyRawCoordinateLongitude"];
    [coder encodeDouble:_altitude forKey:@"kCLLocationCodingKeyAltitude"];
    [coder encodeDouble:_altitude forKey:@"kCLLocationCodingKeyRawAltitude"];
    [coder encodeDouble:_horizontalAccuracy forKey:@"kCLLocationCodingKeyHorizontalAccuracy"];
    [coder encodeDouble:_horizontalAccuracy forKey:@"kCLLocationCodingKeyRawHorizontalAccuracy"];
    [coder encodeDouble:_verticalAccuracy forKey:@"kCLLocationCodingKeyVerticalAccuracy"];
    [coder encodeDouble:_verticalAccuracy forKey:@"kCLLocationCodingKeyRawVerticalAccuracy"];
    [coder encodeDouble:_course forKey:@"kCLLocationCodingKeyCourse"];
    [coder encodeDouble:_course forKey:@"kCLLocationCodingKeyRawCourse"];
    [coder encodeDouble:_speed forKey:@"kCLLocationCodingKeySpeed"];
    [coder encodeDouble:[_timestamp timeIntervalSinceReferenceDate] forKey:@"kCLLocationCodingKeyTimestamp"];
}

static double decodeDoubleWithFallback(NSCoder *coder, NSString *key, NSString *rawKey, double defaultValue)
{
    if ([coder containsValueForKey:key])
        return [coder decodeDoubleForKey:key];
    if ([coder containsValueForKey:rawKey])
        return [coder decodeDoubleForKey:rawKey];
    return defaultValue;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super init];
    if (self)
    {
        _coordinate.latitude = decodeDoubleWithFallback(coder, @"kCLLocationCodingKeyCoordinateLatitude",
                                                        @"kCLLocationCodingKeyRawCoordinateLatitude", 0.0);
        _coordinate.longitude = decodeDoubleWithFallback(coder, @"kCLLocationCodingKeyCoordinateLongitude",
                                                         @"kCLLocationCodingKeyRawCoordinateLongitude", 0.0);
        _altitude = decodeDoubleWithFallback(coder, @"kCLLocationCodingKeyAltitude",
                                             @"kCLLocationCodingKeyRawAltitude", 0.0);
        _horizontalAccuracy = decodeDoubleWithFallback(coder, @"kCLLocationCodingKeyHorizontalAccuracy",
                                                       @"kCLLocationCodingKeyRawHorizontalAccuracy", -1.0);
        _verticalAccuracy = decodeDoubleWithFallback(coder, @"kCLLocationCodingKeyVerticalAccuracy",
                                                     @"kCLLocationCodingKeyRawVerticalAccuracy", -1.0);
        _course = decodeDoubleWithFallback(coder, @"kCLLocationCodingKeyCourse",
                                           @"kCLLocationCodingKeyRawCourse", -1.0);
        _speed = [coder containsValueForKey:@"kCLLocationCodingKeySpeed"]
                 ? [coder decodeDoubleForKey:@"kCLLocationCodingKeySpeed"] : -1.0;
        _timestamp = [coder containsValueForKey:@"kCLLocationCodingKeyTimestamp"]
                     ? [[NSDate alloc] initWithTimeIntervalSinceReferenceDate:[coder decodeDoubleForKey:@"kCLLocationCodingKeyTimestamp"]]
                     : [[NSDate alloc] init];
    }
    return self;
}

- (CLLocationDistance)getDistanceFrom:(const CLLocation *)location
{
    return [self distanceFromLocation:location];
}

// Methods of the public API that are not implemented yet stay no-ops.
- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

@end
