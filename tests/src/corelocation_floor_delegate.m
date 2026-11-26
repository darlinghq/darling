// Copyright (C) 2026 Lubos Dolezel
//
// CLLocation's floor (Apple's CLLocation reference, macOS 10.15) is nil when
// the floor is not known, as for a location made with the public
// initializers; it survives copying and archiving as nil. The
// CLLocationManagerDelegate protocol can be imported from its own header, as
// in Apple's SDK.
#import <CoreLocation/CLLocationManagerDelegate.h>
#import <CoreLocation/CoreLocation.h>
#import <Foundation/Foundation.h>
#include <stdio.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

@interface Delegate : NSObject <CLLocationManagerDelegate>
@end

@implementation Delegate
@end

int main(void)
{
	@autoreleasepool {
		CLLocation *location = [[[CLLocation alloc] initWithLatitude:50.08 longitude:14.42] autorelease];
		CHECK(location.floor == nil);
		CHECK([[location copy] floor] == nil);

		NSData *archive = [NSKeyedArchiver archivedDataWithRootObject:location requiringSecureCoding:YES error:NULL];
		CLLocation *decoded = [NSKeyedUnarchiver unarchivedObjectOfClass:[CLLocation class] fromData:archive error:NULL];
		CHECK(decoded != nil && decoded.floor == nil);

		CHECK([CLFloor supportsSecureCoding]);
		CHECK([[[[Delegate alloc] init] autorelease] conformsToProtocol:@protocol(CLLocationManagerDelegate)]);
	}
	printf("%s\n", g_failures ? "FAILED" : "ALL PASSED");
	return g_failures != 0;
}
