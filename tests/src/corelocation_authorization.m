// Copyright (C) 2026 Lubos Dolezel
//
// CLLocationManager answers the authorization and services queries instead of
// raising: the class and instance authorizationStatus agree, the status is one
// of the documented values, and a process that has not been granted access
// never sees an authorized status while location services are off.
// The program passes on macOS 27, where an unbundled tool that never asked
// for permission gets kCLAuthorizationStatusNotDetermined.
#import <CoreLocation/CoreLocation.h>
#import <Foundation/Foundation.h>
#include <stdio.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

int main(void)
{
	@autoreleasepool {
		CLAuthorizationStatus status = [CLLocationManager authorizationStatus];
		BOOL enabled = [CLLocationManager locationServicesEnabled];

		CHECK(status >= kCLAuthorizationStatusNotDetermined && status <= 4);

		CLLocationManager *manager = [[CLLocationManager alloc] init];
		CHECK(manager != nil);
		CHECK([manager authorizationStatus] == status);

		if (!enabled) {
			CHECK(status != kCLAuthorizationStatusAuthorizedAlways);
			CHECK(status != 4);
		}

		manager.delegate = nil;
		CHECK(manager.delegate == nil);
	}

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
