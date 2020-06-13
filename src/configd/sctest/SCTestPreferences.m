/*
 * Copyright (c) 2016, 2017 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#import "SCTest.h"
#import "SCTestUtils.h"

@interface SCTestPreferences : SCTest
@property SCPreferencesRef prefs;
@end

@implementation SCTestPreferences

+ (NSString *)command
{
	return @"preferences";
}

+ (NSString *)commandDescription
{
	return @"Tests the SCPreferences code path";
}

- (instancetype)initWithOptions:(NSDictionary *)options
{
	self = [super initWithOptions:options];
	if (self) {
		_prefs = SCPreferencesCreate(kCFAllocatorDefault, CFSTR("SCTest"), NULL);
	}
	return self;
}

- (void)dealloc
{
	if (self.prefs != NULL) {
		CFRelease(self.prefs);
		self.prefs = NULL;
	}
}

- (void)start
{
	if (self.options[kSCTestPreferencesServiceList]) {
		NSDictionary *services = (__bridge NSDictionary *)SCPreferencesGetValue(self.prefs, kSCPrefNetworkServices);
		if (services != nil) {
			[self printNetworkServicesFromDict:services];
		} else {
			SCTestLog("No services present!");
		}
	}

	if (self.options[kSCTestPreferencesServiceOrder]) {
		SCNetworkSetRef set = SCNetworkSetCopyCurrent(self.prefs);
		NSArray *serviceID = (__bridge NSArray *)SCNetworkSetGetServiceOrder(set);
		NSDictionary *services = (__bridge NSDictionary *)SCPreferencesGetValue(self.prefs, kSCPrefNetworkServices);
		int counter = 1;
		SCTestLog("Network service order");
		for (NSString *key in serviceID) {
			NSDictionary *dict = [services objectForKey:key];
			SCTestLog("\n%d: %@\n\tUserDefinedName: %@", counter++, key, [dict objectForKey:(__bridge NSString *)kSCPropNetServiceUserDefinedName]);
		}
		CFRelease(set);
	}

	[self cleanupAndExitWithErrorCode:0];
}

- (void)printNetworkServicesFromDict:(NSDictionary *)serviceDict
{
	int counter = 1;
	SCTestLog("Network Services");
	for (NSString *key in serviceDict) {
		NSDictionary *dict = [serviceDict objectForKey:key];
		SCTestLog("\n%d: %@\n\tUserDefinedName: %@", counter++, key, [dict objectForKey:(__bridge NSString *)kSCPropNetServiceUserDefinedName]);
	}
}

- (BOOL)unitTest
{
	BOOL allUnitTestsPassed = YES;
	allUnitTestsPassed &= [self unitTestNetworkServicesSanity];
	allUnitTestsPassed &= [self unitTestPreferencesAPI];
	allUnitTestsPassed &= [self unitTestPreferencesSession];
	return  allUnitTestsPassed;

}

- (BOOL)unitTestNetworkServicesSanity
{
	// We verify that every service has a unique name, an interface, an IPv4 config method and and IPv6 config method.
	NSArray *sets;
	NSDictionary *services;
	SCTestPreferences *test;

	test = [[SCTestPreferences alloc] initWithOptions:self.options];

	sets = (__bridge_transfer NSArray *)SCNetworkSetCopyAll(test.prefs);
	if (sets == nil || [sets count] == 0) {
		SCTestLog("No sets present!");
		return NO;
	}

	services = (__bridge NSDictionary *)SCPreferencesGetValue(test.prefs, kSCPrefNetworkServices);
	if (services == nil || [services count] == 0) {
		SCTestLog("No services present!");
		return NO;
	}

	for (id setPtr in sets) {
		SCNetworkSetRef set = (__bridge SCNetworkSetRef)setPtr;
		NSArray *serviceArray = nil;
		NSMutableArray *serviceNameArray = nil;
		NSString *setID;

		setID = (__bridge NSString *)SCNetworkSetGetSetID(set);

		serviceArray = (__bridge_transfer NSArray *)SCNetworkSetCopyServices(set);
		if (serviceArray == nil) {
			SCTestLog("No services in set %@!", setID);
			continue;
		}

		serviceNameArray = [[NSMutableArray alloc] init];
		for (id servicePTR in serviceArray) {
			NSDictionary *serviceDict;
			NSDictionary *ipv4Dict;
			NSDictionary *ipv6Dict;
			NSDictionary *ipv4ProtocolConfig;
			NSDictionary *ipv6ProtocolConfig;
			NSString *serviceName;
			NSString *serviceID;
			NSString *interfaceType;
			SCNetworkServiceRef service;
			SCNetworkInterfaceRef interface;
			SCNetworkProtocolRef ipv4Protocol;
			SCNetworkProtocolRef ipv6Protocol;


			service = (__bridge SCNetworkServiceRef)servicePTR;
			serviceID = (__bridge NSString *)SCNetworkServiceGetServiceID(service);

			serviceDict = [services objectForKey:serviceID];
			if (![serviceDict isKindOfClass:[NSDictionary class]]) {
				SCTestLog("Service is not a dictionary");
				return NO;
			}

			serviceName = (__bridge NSString *)SCNetworkServiceGetName(service);
			if (serviceName != nil) {
				// Check if the name is unique
				BOOL namePresent = [serviceNameArray containsObject:serviceName];
				if (!namePresent) {
					[serviceNameArray addObject:serviceName];
				} else {
					SCTestLog("Duplicate services with name %@ exist", serviceName);
					return NO;
				}
			} else {
				SCTestLog("Service ID %@ does not have a name", serviceID);
				return NO;
			}

			interface = SCNetworkServiceGetInterface(service);
			if (interface == nil) {
				SCTestLog("Service %@ does not have an interface", serviceName);
				return NO;
			}

			interfaceType = (__bridge NSString *)SCNetworkInterfaceGetInterfaceType(interface);
			if (interfaceType == nil || [interfaceType length] == 0) {
				SCTestLog("Service %@ does not have an interface type", serviceName);
				return NO;
			}
#if TARGET_OS_IPHONE
			if ([interfaceType containsString:@"CommCenter"]) {
				// CommCenter services typically do not have an ipv4/v6 data OR config method. Skip such services.
				continue;
			}
#endif // TARGET_OS_IPHONE
			ipv4Protocol = SCNetworkServiceCopyProtocol(service, kSCNetworkProtocolTypeIPv4);
			ipv6Protocol = SCNetworkServiceCopyProtocol(service, kSCNetworkProtocolTypeIPv6);

			if (ipv4Protocol != NULL) {
				ipv4ProtocolConfig = (__bridge NSDictionary *)SCNetworkProtocolGetConfiguration(ipv4Protocol);
				if (ipv4ProtocolConfig != nil) {
					ipv4Dict = [ipv4ProtocolConfig copy];
				}
				CFRelease(ipv4Protocol);
			}

			if (ipv6Protocol != NULL) {
				ipv6ProtocolConfig = (__bridge NSDictionary *)SCNetworkProtocolGetConfiguration(ipv6Protocol);
				if (ipv6ProtocolConfig != nil) {
					ipv6Dict = [ipv6ProtocolConfig copy];
				}
				CFRelease(ipv6Protocol);
			}

			// Check that we have at least one IP config method
			if (ipv4Dict == nil && ipv6Dict == nil) {
				SCTestLog("Service %@ does not have an IP dictionary", serviceName);
				return NO;
			}

			if ([ipv4Dict objectForKey:(__bridge NSString *)kSCPropNetIPv4ConfigMethod] == nil &&
			    [ipv6Dict objectForKey:(__bridge NSString *)kSCPropNetIPv6ConfigMethod] == nil) {
				SCTestLog("Service %@ does not have an IP Config Method", serviceName);
				return NO;
			}
		}
	}

	SCTestLog("Verified that the Network Services have valid configurations");

	return YES;
}

- (BOOL)unitTestPreferencesSession
{
	SCPreferencesRef prefs;
	prefs = SCPreferencesCreate(kCFAllocatorDefault, CFSTR("SCTest"), NULL);
	if (prefs == NULL) {
		SCTestLog("Failed to create SCPreferences. Error: %s", SCErrorString(SCError()));
		return NO;
	} else {
		CFRelease(prefs);
	}

	prefs = SCPreferencesCreateWithOptions(kCFAllocatorDefault, CFSTR("SCTest"), NULL, kSCPreferencesUseEntitlementAuthorization, NULL);
	if (prefs == NULL) {
		SCTestLog("Failed to create SCPreferences w/options. Error: %s", SCErrorString(SCError()));
		return NO;
	} else {
		CFRelease(prefs);
	}

	prefs = SCPreferencesCreateWithAuthorization(kCFAllocatorDefault, CFSTR("SCTest"), NULL, kSCPreferencesUseEntitlementAuthorization);
	if (prefs == NULL) {
		SCTestLog("Failed to create SCPreferences w/options. Error: %s", SCErrorString(SCError()));
		return NO;
	} else {
		CFRelease(prefs);
	}

	SCTestLog("Verified that the preferences session can be created");
	return YES;
}

- (BOOL)unitTestPreferencesAPI
{
	BOOL ok = NO;
	int iterations = 100;
	NSDictionary *prefsOptions;
	NSMutableArray *keys;
	NSMutableArray *values;
	SCTestPreferences *test;
	NSArray *keyList;

	test = [[SCTestPreferences alloc] initWithOptions:self.options];
	if (test.prefs != NULL) {
		CFRelease(test.prefs);
		test.prefs = NULL;
	}

	prefsOptions = @{(__bridge NSString *)kSCPreferencesOptionRemoveWhenEmpty:(__bridge NSNumber *)kCFBooleanTrue};
	test.prefs = SCPreferencesCreateWithOptions(kCFAllocatorDefault,
							CFSTR("SCTest"),
							CFSTR("SCTestPreferences.plist"),
							kSCPreferencesUseEntitlementAuthorization,
							(__bridge CFDictionaryRef)prefsOptions);
	if (test.prefs == NULL) {
		SCTestLog("Failed to create a preferences session. Error: %s", SCErrorString(SCError()));
		return NO;
	}

	keys = [[NSMutableArray alloc] init];
	values = [[NSMutableArray alloc] init];
	for (int i = 0; i < iterations; i++) {
		NSUUID *uuidKey = [NSUUID UUID];
		NSUUID *uuidValue = [NSUUID UUID];

		ok = SCPreferencesLock(test.prefs, true);
		if (!ok) {
			SCTestLog("Failed to get preferences lock. Error: %s", SCErrorString(SCError()));
			return NO;
		}

		ok = SCPreferencesSetValue(test.prefs, (__bridge CFStringRef)uuidKey.UUIDString, (__bridge CFStringRef)uuidValue.UUIDString);
		if (!ok) {
			SCTestLog("Failed to set preferences value. Error: %s", SCErrorString(SCError()));
			return NO;
		}

		ok = SCPreferencesUnlock(test.prefs);
		if (!ok) {
			SCTestLog("Failed to release preferences lock. Error: %s", SCErrorString(SCError()));
			return NO;
		}

		[keys addObject:uuidKey.UUIDString];
		[values addObject:uuidValue.UUIDString];
	}

	ok = SCPreferencesCommitChanges(test.prefs);
	if (!ok) {
		SCTestLog("Failed to commit preferences. Error: %s", SCErrorString(SCError()));
		return NO;
	}

	CFRelease(test.prefs);
	test.prefs = SCPreferencesCreateWithOptions(kCFAllocatorDefault,
							CFSTR("SCTest"),
							CFSTR("SCTestPreferences.plist"),
							kSCPreferencesUseEntitlementAuthorization,
							(__bridge CFDictionaryRef)prefsOptions);
	if (test.prefs == NULL) {
		SCTestLog("Failed to create a preferences session. Error: %s", SCErrorString(SCError()));
		return NO;
	}

	keyList = (__bridge_transfer NSArray *)SCPreferencesCopyKeyList(test.prefs);
	if ([keyList count] < [keys count]) {
		SCTestLog("Failed to copy all keys from preferences. Error: %s", SCErrorString(SCError()));
		return NO;
	}

	for (NSString *key in keys) {
		NSString *valueString = (__bridge NSString *)SCPreferencesGetValue(test.prefs, (__bridge CFStringRef)key);
		if (!valueString) {
			SCTestLog("Failed to get value from preferences. Error: %s", SCErrorString(SCError()));
			return NO;
		}

		BOOL ok = [values containsObject:valueString];
		if (!ok) {
			SCTestLog("Incorrect value fetched from preferences");
			return NO;
		}
	}

	ok = SCPreferencesRemoveAllValues(test.prefs);
	if (!ok) {
		SCTestLog("Failed to remove values  preferences. Error: %s", SCErrorString(SCError()));
		return NO;
	}

	ok = SCPreferencesCommitChanges(test.prefs);
	if (!ok) {
		SCTestLog("Failed to commit preferences. Error: %s", SCErrorString(SCError()));
		return NO;
	}

	CFRelease(test.prefs);
	test.prefs = SCPreferencesCreateWithOptions(kCFAllocatorDefault,
							CFSTR("SCTest"),
							CFSTR("SCTestPreferences.plist"),
							kSCPreferencesUseEntitlementAuthorization,
							(__bridge CFDictionaryRef)prefsOptions);
	if (test.prefs == NULL) {
		SCTestLog("Failed to create a preferences session. Error: %s", SCErrorString(SCError()));
		return NO;
	}

	keyList = (__bridge_transfer NSArray *)SCPreferencesCopyKeyList(test.prefs);
	if ([keyList count] > 0) {
		SCTestLog("Failed to remove all keys from preferences. Error: %s", SCErrorString(SCError()));
		return NO;
	}

	SCTestLog("Verified that SCPreferences APIs behave as expected");
	return ok;
}

- (void)cleanupAndExitWithErrorCode:(int)error
{
	[super cleanupAndExitWithErrorCode:error];
}

@end
