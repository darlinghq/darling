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
#import <arpa/inet.h>
#import <NetworkExtension/NEPolicySession.h>
#import <network_information.h>

#define REACHABILITY_TEST_HOSTNAME "apple.com"

@interface SCTestReachability : SCTest
@property SCNetworkReachabilityRef target;
@property dispatch_queue_t callbackQ;
@end

@implementation SCTestReachability

+ (NSString *)command
{
	return @"reachability";
}

+ (NSString *)commandDescription
{
	return @"Tests the SCNetworkReachability code path";
}

- (instancetype)initWithOptions:(NSDictionary *)options
{
	self = [super initWithOptions:options];
	if (self) {
		NSMutableDictionary *reachOptions;
		if (self.options[kSCTestReachabilityInterface] != nil) {
			reachOptions = [[NSMutableDictionary alloc] init];
			[reachOptions setObject:self.options[kSCTestReachabilityInterface] forKey:(__bridge NSString *)kSCNetworkReachabilityOptionInterface];
			if (self.options[kSCTestReachabilityHost] != nil) {
				[reachOptions setObject:self.options[kSCTestReachabilityHost] forKey:(__bridge NSString *)kSCNetworkReachabilityOptionNodeName];
			} else if (self.options[kSCTestReachabilityAddress] != nil) {
				NSData *data;
				NSString *addressString = self.options[kSCTestReachabilityAddress];
				struct sockaddr *sa = _SC_string_to_sockaddr(addressString.UTF8String, AF_INET, NULL, 0);
				if (sa == NULL) {
					sa = _SC_string_to_sockaddr(addressString.UTF8String, AF_INET6, NULL, 0);
					if (sa == NULL) {
						SCTestLog("Invalid address");
						ERR_EXIT;
					}
				}
				data = [NSData dataWithBytes:sa length:(sa->sa_family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6)];
				[reachOptions setObject:data forKey:(__bridge NSString *)kSCNetworkReachabilityOptionRemoteAddress];
			}


			_target = SCNetworkReachabilityCreateWithOptions(kCFAllocatorDefault, (__bridge CFDictionaryRef)reachOptions);
		} else if (self.options[kSCTestReachabilityHost]) {
			NSString *host = self.options[kSCTestReachabilityHost];
			_target = SCNetworkReachabilityCreateWithName(kCFAllocatorDefault, host.UTF8String);
		} else if (self.options[kSCTestReachabilityAddress]) {
			NSString *addressString = self.options[kSCTestReachabilityAddress];
			struct sockaddr *sa = _SC_string_to_sockaddr(addressString.UTF8String, AF_INET, NULL, 0);
			if (sa == NULL) {
				sa = _SC_string_to_sockaddr(addressString.UTF8String, AF_INET6, NULL, 0);
				if (sa == NULL) {
					SCTestLog("Invalid address");
					ERR_EXIT;
				}
			}
			_target = SCNetworkReachabilityCreateWithAddress(kCFAllocatorDefault, sa);
		}
	}
	return self;
}

- (void)dealloc
{
	if (self.target != NULL) {
		CFRelease(self.target);
		self.target = NULL;
	}
}

void
myReachabilityCallback(SCNetworkReachabilityRef target, SCNetworkReachabilityFlags flags, void *info)
{
#pragma unused(target)
#pragma unused(info)
	struct tm	tm_now;
	struct timeval	tv_now;
	(void)gettimeofday(&tv_now, NULL);
	(void)localtime_r(&tv_now.tv_sec, &tm_now);
	SCTestLog("%2d:%02d:%02d.%03d Reachability changed: %#x", tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec, tv_now.tv_usec / 1000, flags);
}

- (void)start
{
	if (self.options[kSCTestReachabilityHost] != nil && self.options[kSCTestReachabilityAddress] != nil) {
		SCTestLog("Please specify either a host or address");
		ERR_EXIT;
	}

	if (self.options[kSCTestReachabilityWatch]) {
		self.callbackQ = dispatch_queue_create("SCTestReachability callback queue", NULL);
		SCNetworkReachabilitySetCallback(self.target, myReachabilityCallback, NULL);
		SCNetworkReachabilitySetDispatchQueue(self.target, self.callbackQ);
	} else {
		SCNetworkReachabilityFlags flags;
		SCNetworkReachabilityGetFlags(self.target, &flags);
		SCTestLog("Reachability: %#x", flags);
		[self cleanupAndExitWithErrorCode:0];
	}
}

- (void)cleanupAndExitWithErrorCode:(int)error
{
	[super cleanupAndExitWithErrorCode:error];
}

- (NSString *)primaryInterfaceName
{
	const char *name;
	NSString *nsName;
	nwi_ifstate_t ifstate;
	nwi_state_t nwi;

	nwi = nwi_state_copy();
	ifstate = nwi_state_get_first_ifstate(nwi, AF_INET);
	if (ifstate == NULL) {
		ifstate = nwi_state_get_first_ifstate(nwi, AF_INET6);
		if (ifstate == NULL) {
			// May be we are in airplane mode
			SCTestLog("No primary interface found");
			return nil;
		}
	}

	name = nwi_ifstate_get_ifname(ifstate);
	nsName = [NSString stringWithCString:name encoding:NSUTF8StringEncoding];
	nwi_state_release(nwi);

	return nsName;
}

- (BOOL)setup
{
	return YES;
}

- (BOOL)unitTest
{
	if(![self setup]) {
		return NO;
	}

	BOOL allUnitTestsPassed = YES;

	allUnitTestsPassed &= [self unitTestBasicReachabilityCheck];
	allUnitTestsPassed &= [self unitTestReachabilityWithPolicy];
	allUnitTestsPassed &= [self unitTestScopedReachabilityWithPolicy];

	if(![self tearDown]) {
		return NO;
	}

	return allUnitTestsPassed;
}

- (BOOL)unitTestBasicReachabilityCheck
{
	SCNetworkReachabilityFlags flags;
	SCTestReachability *test;

	test = [[SCTestReachability alloc] initWithOptions:self.options];
	if ([test primaryInterfaceName] == nil) {
		return YES;
	}

	if (test.target == NULL) {
		NSDictionary *options = @{kSCTestReachabilityHost:@REACHABILITY_TEST_HOSTNAME};
		test = [[SCTestReachability alloc] initWithOptions:options];
	}


	SCNetworkReachabilityGetFlags(test.target, &flags);
	if (flags == 0) {
		SCTestLog("Reachability reported not reachable");
		return NO;
	}

	SCTestLog("Verified that basic reachability check succeeds");
	return YES;
}

- (BOOL)unitTestReachabilityWithPolicy
{
	NEPolicyCondition *condition1;
	NEPolicyCondition *condition2;
	SCNetworkReachabilityFlags flags;
	BOOL ok;
	NEPolicy *policy;
	NSUInteger policyID;
	NEPolicyResult *result;
	NEPolicySession *session;
	SCTestReachability *test;

	test = [[SCTestReachability alloc] initWithOptions:self.options];
	if ([test primaryInterfaceName] == nil) {
		return YES;
	}

	if (test.target == NULL) {
		NSDictionary *options = @{kSCTestReachabilityHost:@REACHABILITY_TEST_HOSTNAME};
		test = [[SCTestReachability alloc] initWithOptions:options];
	}

	SCNetworkReachabilityGetFlags(test.target, &flags);
	if (flags == 0) {
		SCTestLog("Reachability reported not reachable");
		return NO;
	}

	session = [[NEPolicySession alloc] init];
	if (session == nil) {
		SCTestLog("Failed to create NEPolicySession");
		return NO;
	}

	result = [NEPolicyResult drop];
	condition1 = [NEPolicyCondition allInterfaces];
	condition2 = [NEPolicyCondition effectivePID:getpid()];
	policy = [[NEPolicy alloc ] initWithOrder:10 result:result conditions:@[condition1, condition2]];
	policyID = [session addPolicy:policy];
	if (policyID == 0) {
		SCTestLog("Failed to add policy");
		return NO;
	}

	ok = [session apply];
	if (!ok) {
		SCTestLog("Failed to apply policy");
		return NO;
	}

	SCNetworkReachabilityGetFlags(test.target, &flags);
	if (flags != 0) {
		SCTestLog("Reachability reported as reachable, in presence of a drop policy");
		return NO;
	}

	ok = [session removeAllPolicies];
	if (!ok) {
		SCTestLog("Failed to remove policies from session");
		return NO;
	}

	ok = [session apply];
	if (!ok) {
		SCTestLog("Failed to apply policy");
		return NO;
	}

	SCNetworkReachabilityGetFlags(test.target, &flags);
	if (flags == 0) {
		SCTestLog("Reachability reported as not reachable, in absence of a drop policy");
		return NO;
	}

	SCTestLog("Verified that SCNetworkReachability reports reachability corresponding to the NECP Policies");
	return YES;
}

- (BOOL)unitTestScopedReachabilityWithPolicy
{
	NEPolicyCondition *condition1;
	NEPolicyCondition *condition2;
	SCNetworkReachabilityFlags flags;
	BOOL ok;
	NEPolicy *policy;
	NSUInteger policyID;
	NSString *primaryInterface;
	NEPolicyResult *result;
	NEPolicySession *session;
	SCTestReachability *test;

	test = [[SCTestReachability alloc] initWithOptions:self.options];
	primaryInterface = [test primaryInterfaceName];
	if (primaryInterface == nil) {
		return YES;
	}

	if (test.target == NULL) {
		NSDictionary *options = @{kSCTestReachabilityHost:@REACHABILITY_TEST_HOSTNAME,
					  kSCTestReachabilityInterface:primaryInterface};
		test = [[SCTestReachability alloc] initWithOptions:options];
	}

	SCNetworkReachabilityGetFlags(test.target, &flags);
	if (flags == 0) {
		SCTestLog("Reachability reported not reachable");
		return NO;
	}

	session = [[NEPolicySession alloc] init];
	if (session == nil) {
		SCTestLog("Failed to create NEPolicySession");
		return NO;
	}

	result = [NEPolicyResult drop];
	condition1 = [NEPolicyCondition scopedInterface:primaryInterface];
	condition2 = [NEPolicyCondition effectivePID:getpid()];
	policy = [[NEPolicy alloc ] initWithOrder:10 result:result conditions:@[condition1, condition2]];
	policyID = [session addPolicy:policy];
	if (policyID == 0) {
		SCTestLog("Failed to add policy");
		return NO;
	}

	ok = [session apply];
	if (!ok) {
		SCTestLog("Failed to apply policy");
		return NO;
	}

	SCNetworkReachabilityGetFlags(test.target, &flags);
	if (flags != 0) {
		SCTestLog("Reachability reported as reachable, in presence of a drop policy");
		return NO;
	}

	ok = [session removeAllPolicies];
	if (!ok) {
		SCTestLog("Failed to remove policies from session");
		return NO;
	}

	ok = [session apply];
	if (!ok) {
		SCTestLog("Failed to apply policy");
		return NO;
	}

	SCNetworkReachabilityGetFlags(test.target, &flags);
	if (flags == 0) {
		SCTestLog("Reachability reported as not reachable, in absence of a drop policy");
		return NO;
	}

	SCTestLog("Verified that SCNetworkReachability for scoped queries report reachability corresponding to the NECP Policies");
	return YES;
}

- (BOOL)tearDown
{
	return YES;
}

@end
