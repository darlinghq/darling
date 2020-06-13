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

@interface SCTestDynamicStore : SCTest
@property SCDynamicStoreRef store;
@property dispatch_semaphore_t sem;
@property int counter;
@end

@implementation SCTestDynamicStore

+ (NSString *)command
{
	return @"dynamic_store";
}

+ (NSString *)commandDescription
{
	return @"Tests the SCDynamicStore code path";
}

- (instancetype)initWithOptions:(NSDictionary *)options
{
	self = [super initWithOptions:options];
	if (self) {
		_store = SCDynamicStoreCreate(kCFAllocatorDefault,
						  CFSTR("SCTest"),
						  NULL,
						  NULL);
		if (_store == NULL) {
			SCTestLog("Could not create session");
			ERR_EXIT;
		}
	}
	return self;
}

- (void)dealloc
{
	if (self.store != NULL) {
		CFRelease(self.store);
		self.store = NULL;
	}
}

- (void)start
{
	CFStringRef key;
	if (self.options[kSCTestDynamicStoreOptionDNS]) {
		key = SCDynamicStoreKeyCreateNetworkGlobalEntity(kCFAllocatorDefault, kSCDynamicStoreDomainState, kSCEntNetDNS);
		CFPropertyListRef value = SCDynamicStoreCopyValue(self.store, key);
		SCTestLog("%@ : %@", key, value);
		CFRelease(key);
	}

	if (self.options[kSCTestDynamicStoreOptionIPv4]) {
		key = SCDynamicStoreKeyCreateNetworkGlobalEntity(kCFAllocatorDefault, kSCDynamicStoreDomainState, kSCEntNetIPv4);
		CFPropertyListRef value = SCDynamicStoreCopyValue(self.store, key);
		SCTestLog("%@ : %@", key, value);
		CFRelease(key);
	}

	if (self.options[kSCTestDynamicStoreOptionIPv6]) {
		key = SCDynamicStoreKeyCreateNetworkGlobalEntity(kCFAllocatorDefault, kSCDynamicStoreDomainState, kSCEntNetIPv6);
		CFPropertyListRef value = SCDynamicStoreCopyValue(self.store, key);
		SCTestLog("%@ : %@", key, value);
		CFRelease(key);
	}

	if (self.options[kSCTestDynamicStoreOptionProxies]) {
		key = SCDynamicStoreKeyCreateNetworkGlobalEntity(kCFAllocatorDefault, kSCDynamicStoreDomainState, kSCEntNetProxies);
		CFPropertyListRef value = SCDynamicStoreCopyValue(self.store, key);
		SCTestLog("%@ : %@", key, value);
		CFRelease(key);
	}

	[self cleanupAndExitWithErrorCode:0];
}

- (void)cleanupAndExitWithErrorCode:(int)error
{
	[super cleanupAndExitWithErrorCode:error];
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
	allUnitTestsPassed &= [self unitTestSetAndRemoveValue];
	allUnitTestsPassed &= [self unitTestCopyMultiple];
	allUnitTestsPassed &= [self unitTestSCDynamicStoreCallbackStress];
	allUnitTestsPassed &= [self unitTestSCDynamicStoreSetMultipleStress];

	if(![self tearDown]) {
		return NO;
	}

	return allUnitTestsPassed;
}

- (BOOL)tearDown
{
	return YES;
}

- (BOOL)unitTestSetAndRemoveValue
{
	int iterations = 1000;
	NSDictionary *bogusValue;
	SCTestDynamicStore *test;

	test = [[SCTestDynamicStore alloc] initWithOptions:self.options];
	bogusValue = @{@"Pretty":@"Useless"};

	for (int i = 0; i < iterations; i++) {
		NSUUID *uuid = [NSUUID UUID];
		CFStringRef key;
		BOOL ok;

		key = SCDynamicStoreKeyCreateNetworkServiceEntity(kCFAllocatorDefault,
								  CFSTR("SCTest"),
								  (__bridge CFStringRef)uuid.UUIDString,
								  kSCEntNetDNS);
		ok = SCDynamicStoreSetValue(test.store, key, (__bridge CFDictionaryRef)bogusValue);
		if (!ok) {
			SCTestLog("Failed to set value in SCDynamicStore. Error: %s", SCErrorString(SCError()));
			CFRelease(key);
			return NO;
		}

		ok = SCDynamicStoreRemoveValue(test.store, key);
		if (!ok) {
			SCTestLog("Failed to remove value from SCDynamicStore. Error: %s", SCErrorString(SCError()));
			CFRelease(key);
			return NO;
		}

		CFRelease(key);
	}

	SCTestLog("Successfully completed setAndRemove unit test");
	return YES;
}

- (BOOL)unitTestCopyMultiple
{
	NSString *pattern;
	SCTestDynamicStore *test;
	CFArrayRef keyList;
	int iterations = 1000;

	test = [[SCTestDynamicStore alloc] initWithOptions:self.options];
	pattern = (__bridge_transfer NSString *)SCDynamicStoreKeyCreate(kCFAllocatorDefault,
									CFSTR("%@/%@/%@/%@/%@"),
									kSCDynamicStoreDomainState,
									kSCCompNetwork,
									kSCCompInterface,
									kSCCompAnyRegex,
									kSCCompAnyRegex);

	keyList = SCDynamicStoreCopyKeyList(test.store, (__bridge CFStringRef)pattern);
	for (int i = 0; i < iterations; i++) {
		CFDictionaryRef value = SCDynamicStoreCopyMultiple(test.store, keyList, NULL);
		if (value == NULL) {
			SCTestLog("Failed to copy multiple values from SCDynamicStore. Error: %s", SCErrorString(SCError()));
			CFRelease(keyList);
			return NO;
		}
		CFRelease(value);
	}
	CFRelease(keyList);

	pattern = (__bridge_transfer NSString *)SCDynamicStoreKeyCreate(kCFAllocatorDefault,
									CFSTR("%@/%@/%@/%@/%@"),
									kSCDynamicStoreDomainSetup,
									kSCCompNetwork,
									kSCCompService,
									kSCCompAnyRegex,
									kSCCompAnyRegex);

	keyList = SCDynamicStoreCopyKeyList(test.store, (__bridge CFStringRef)pattern);
	for (int i = 0; i < iterations; i++) {
		CFDictionaryRef value = SCDynamicStoreCopyMultiple(test.store, keyList, NULL);
		if (value == NULL) {
			SCTestLog("Failed to copy multiple values from SCDynamicStore. Error: %s", SCErrorString(SCError()));
			CFRelease(keyList);
			return NO;
		}
		CFRelease(value);
	}
	CFRelease(keyList);

	SCTestLog("Successfully completed copyMultiple unit test");
	return YES;
}

void
myTestCallback(SCDynamicStoreRef store, CFArrayRef changedKeys, void *ctx)
{
#pragma unused(store)
#pragma unused(changedKeys)
	SCTestDynamicStore *test = (__bridge SCTestDynamicStore *)ctx;
	test.counter++;
	if (test.sem != NULL) {
		dispatch_semaphore_signal(test.sem);
	}
}

- (BOOL)unitTestSCDynamicStoreCallbackStress
{
	SCTestDynamicStore *test;
	int iterations = 100;
	NSString *testKey = @"State:/myTestKey";
	BOOL ok;
	dispatch_queue_t callbackQ;

	test = [[SCTestDynamicStore alloc] initWithOptions:self.options];
	if (test.store != NULL) {
		CFRelease(test.store);
		test.store = NULL;
	}

	SCDynamicStoreContext ctx = {0, (__bridge void * _Nullable)(test), CFRetain, CFRelease, NULL};
	test.store = SCDynamicStoreCreate(kCFAllocatorDefault, CFSTR("SCTest"), myTestCallback, &ctx);

	ok = SCDynamicStoreSetNotificationKeys(test.store, (__bridge CFArrayRef)@[testKey], NULL);
	if (!ok) {
		SCTestLog("Failed to set notification keys. Error: %s", SCErrorString(SCError()));
		return NO;
	}

	callbackQ = dispatch_queue_create("SCTestDynamicStore callback queue", NULL);
	ok = SCDynamicStoreSetDispatchQueue(test.store, callbackQ);
	if (!ok) {
		SCTestLog("Failed to set dispatch queue. Error: %s", SCErrorString(SCError()));
		return NO;
	}

	for (int i = 0; i < iterations; i++) {
		NSUUID *uuid = [NSUUID UUID];
		ok = SCDynamicStoreSetValue(test.store, (__bridge CFStringRef)testKey, (__bridge CFStringRef)uuid.UUIDString);
		if (!ok) {
			SCTestLog("Failed to set value. Error: %s", SCErrorString(SCError()));
			return NO;
		}
		// Perform a write to Dynamic Store every 10ms.
		[test waitFor:0.01];
	}

	ok = SCDynamicStoreRemoveValue(test.store, (__bridge CFStringRef)testKey);
	if (!ok) {
		SCTestLog("Failed to remove value. Error: %s", SCErrorString(SCError()));
		return NO;
	}

	if (test.counter < iterations) {
		// Not all callbacks were received
		SCTestLog("Not all SCDynamicStore callbacks were received (%d/%d). Something went wrong", test.counter, iterations);
		return NO;
	}

	SCTestLog("Successfully completed SCDynamicStore callbacks unit test");
	return YES;
}

- (BOOL)unitTestSCDynamicStoreSetMultipleStress
{
	SCTestDynamicStore *test;
	int iterations = 100;
	NSMutableArray *testKeyArray;
	NSMutableDictionary *testSetDictionary;
	int expectedCallbackCount = 0;
	BOOL ok;
	dispatch_queue_t callbackQ;
	uint8_t waitTime = 1; // second

	test = [[SCTestDynamicStore alloc] initWithOptions:self.options];
	if (test.store != NULL) {
		CFRelease(test.store);
		test.store = NULL;
	}

	SCDynamicStoreContext ctx = {0, (__bridge void * _Nullable)(test), CFRetain, CFRelease, NULL};
	test.store = SCDynamicStoreCreate(kCFAllocatorDefault, CFSTR("SCTest"), myTestCallback, &ctx);
	test.sem = dispatch_semaphore_create(0);

	testKeyArray = [[NSMutableArray alloc] init];

	for (int i = 0; i < iterations; i++) {
		NSUUID *uuid = [NSUUID UUID];
		NSString *str = [NSString stringWithFormat:@"State:/%@", uuid];
		[testKeyArray addObject:str];
	}

	testSetDictionary = [[NSMutableDictionary alloc] init];
	for (NSString *key in testKeyArray) {
		NSUUID *uuid = [NSUUID UUID];
		[testSetDictionary setObject:@[uuid.UUIDString] forKey:key];
	}

	callbackQ = dispatch_queue_create("SCTestDynamicStore callback queue", NULL);
	ok = SCDynamicStoreSetNotificationKeys(test.store, (__bridge CFArrayRef)testKeyArray, NULL);
	if (!ok) {
		SCTestLog("Failed to set notification keys. Error: %s", SCErrorString(SCError()));
		return NO;
	}

	ok = SCDynamicStoreSetDispatchQueue(test.store, callbackQ);
	if (!ok) {
		SCTestLog("Failed to set dispatch queue. Error: %s", SCErrorString(SCError()));
		return NO;
	}

	ok = SCDynamicStoreSetMultiple(test.store, (__bridge CFDictionaryRef)testSetDictionary, NULL, NULL);
	if (!ok) {
		SCTestLog("Failed to set multiple keys. Error: %s", SCErrorString(SCError()));
		return NO;
	} else {
		expectedCallbackCount++; // One callback for set multiple
	}

	if(dispatch_semaphore_wait(test.sem, dispatch_time(DISPATCH_TIME_NOW, waitTime * NSEC_PER_SEC))) {
		SCTestLog("Failed to get a callback when multiple keys are set");
		return NO;
	}

	ok = SCDynamicStoreSetMultiple(test.store, NULL, NULL, (__bridge CFArrayRef)testKeyArray);
	if (!ok) {
		SCTestLog("Failed to set multiple keys. Error: %s", SCErrorString(SCError()));
		return NO;
	} else {
		expectedCallbackCount++; // One callback for fake notification
	}

	if(dispatch_semaphore_wait(test.sem, dispatch_time(DISPATCH_TIME_NOW, waitTime * NSEC_PER_SEC))) {
		SCTestLog("Failed to get a callback when multiple keys are notified");
		return NO;
	}

	ok = SCDynamicStoreSetMultiple(test.store, NULL, (__bridge CFArrayRef)testKeyArray, NULL);
	if (!ok) {
		SCTestLog("Failed to set multiple keys. Error: %s", SCErrorString(SCError()));
		return NO;
	} else {
		expectedCallbackCount++; // One callback for key removal
	}

	if(dispatch_semaphore_wait(test.sem, dispatch_time(DISPATCH_TIME_NOW, waitTime * NSEC_PER_SEC))) {
		SCTestLog("Failed to get a callback when multiple keys are removed");
		return NO;
	}

	if (test.counter < expectedCallbackCount) {
		// Not all callbacks were received
		SCTestLog("Not all SCDynamicStore callbacks were received (%d/%d). Something went wrong", test.counter, expectedCallbackCount);
		return NO;
	}

	SCTestLog("Successfully completed SCDynamicStore set multiple unit test");
	return YES;

}


@end
