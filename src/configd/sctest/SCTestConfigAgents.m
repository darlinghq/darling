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
#import <Network/Network_Private.h>
#import <CoreFoundation/CFXPCBridge.h>
#import <config_agent_info.h>

#define TEST_DOMAIN	"filemaker.com"
#define PATH_QUIESCE_TIME 1.5 // seconds

@interface SCTestConfigAgent : SCTest
@property NSString *serviceID;
@property NSString *proxyKey;
@property NSString *dnsKey;
@property NSArray<NSArray<NSDictionary *> *> *testProxy;
@property NSArray<NWEndpoint *> *testDNS;
@property SCDynamicStoreRef store;
@property (copy) NSArray<NSArray<NSDictionary *> *> *pathProxy;
@property (copy) NSArray<NWEndpoint *> *pathDNS;
@end

@implementation SCTestConfigAgent

- (instancetype)initWithOptions:(NSDictionary *)options
{
	self = [super initWithOptions:options];
	if (self) {
		_serviceID = @"8F66B505-EAEF-4611-BD4D-C523FD9451F0";
		_store = SCDynamicStoreCreate(kCFAllocatorDefault,
						CFSTR("SCTest"),
						NULL,
						NULL);
		_proxyKey = (__bridge_transfer NSString *)[self copyStateKeyWithServiceID:(__bridge CFStringRef)(self.serviceID) forEntity:kSCEntNetProxies];
		_dnsKey = (__bridge_transfer NSString *)[self copyStateKeyWithServiceID:(__bridge CFStringRef)(self.serviceID) forEntity:kSCEntNetDNS];
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

+ (NSString *)command
{
	return @"config_agent";
}

+ (NSString *)commandDescription
{
	return @"Tests the Config Agent code path";
}

- (void)start
{
	if (self.options[kSCTestConfigAgentRemoveProxy]) {
		[self removeFromSCDynamicStore:self.proxyKey];
	}

	if (self.options[kSCTestConfigAgentRemoveDNS]) {
		[self removeFromSCDynamicStore:self.dnsKey];
	}

	NSDictionary *proxyConfig = [self parseProxyAgentOptions];
	if (proxyConfig != nil) {
		[self publishToSCDynamicStore:self.proxyKey value:proxyConfig];
		self.testProxy = @[@[proxyConfig]];
	}

	NSDictionary *dnsConfig = [self parseDNSAgentOptions];
	if (dnsConfig != nil) {
		[self publishToSCDynamicStore:self.dnsKey value:dnsConfig];
		self.testDNS = [self createDNSArray:dnsConfig];
	}

	[self cleanupAndExitWithErrorCode:0];
}

- (CFStringRef)copyStateKeyWithServiceID:(CFStringRef)serviceID
				 forEntity:(CFStringRef)entity
{
	return SCDynamicStoreKeyCreateNetworkServiceEntity(kCFAllocatorDefault,
							   kSCDynamicStoreDomainState,
							   serviceID,
							   entity);
}

- (NSArray<NWEndpoint *> *)createDNSArray:(NSDictionary *)dnsConfig
{
	NSArray<NSString *> *dnsServers;
	NSMutableArray<NWEndpoint *> *dnsArray;

	dnsServers = [dnsConfig objectForKey:(__bridge NSString *)kSCPropNetDNSServerAddresses];
	if (dnsServers == nil || [dnsServers count] == 0) {
		return nil;
	}

	dnsArray = [[NSMutableArray alloc] init];
	for (NSString *server in dnsServers) {
		NWEndpoint *endpoint = (NWEndpoint *)[NWAddressEndpoint endpointWithHostname:server port:@"0"];
		[dnsArray addObject:endpoint];
	}

	return dnsArray;
}

- (void)publishToSCDynamicStore:(NSString *)key
			  value:(NSDictionary *)value
{

	BOOL ok = SCDynamicStoreSetValue(self.store, (__bridge CFStringRef)key, (__bridge CFPropertyListRef _Nonnull)(value));
	if (!ok) {
		int error = SCError();
		if (error == kSCStatusNoKey) {
			return;
		}
		SCTestLog("Could not set in SCDynamicStore: Error: %s", SCErrorString(error));
		return;
	}
}

- (void)removeFromSCDynamicStore:(NSString *)key
{
	BOOL ok = SCDynamicStoreRemoveValue(self.store, (__bridge CFStringRef)key);
	if (!ok) {
		int error = SCError();
		if (error == kSCStatusNoKey) {
			return;
		}
		SCTestLog("Could not remove key: %@, Error: %s", key, SCErrorString(error));
		return;
	}
}

- (NSDictionary *)parseProxyAgentOptions
{
	NSMutableDictionary *proxyConfig = [[NSMutableDictionary alloc] init];

#define NS_NUMBER(x) [NSNumber numberWithInt:x]

#define SET_PROXY_CONFIG(proxyType)														\
	do {																	\
		if (self.options[kSCTestConfigAgent ## proxyType ## Proxy] != nil) {								\
			NSString *serverAndPortString = self.options[kSCTestConfigAgent ## proxyType ## Proxy];					\
			NSArray<NSString *> *serverAndPortArray = [serverAndPortString componentsSeparatedByString:@":"];			\
			if ([serverAndPortArray count] != 2) {											\
				SCTestLog("server address or port missing");									\
				ERR_EXIT;													\
			}															\
			NSString *server = [serverAndPortArray objectAtIndex:0];								\
			NSString *port = [serverAndPortArray objectAtIndex:1];									\
			[proxyConfig setObject:server forKey:(__bridge NSString *)kSCPropNetProxies ## proxyType ## Proxy];			\
			[proxyConfig setObject:NS_NUMBER(port.intValue) forKey:(__bridge NSString *)kSCPropNetProxies ## proxyType ## Port];	\
			[proxyConfig setObject:NS_NUMBER(1) forKey:(__bridge NSString *)kSCPropNetProxies ## proxyType ## Enable];		\
		}																\
	} while(0);

	SET_PROXY_CONFIG(HTTP);
	SET_PROXY_CONFIG(HTTPS);
	SET_PROXY_CONFIG(FTP);
	SET_PROXY_CONFIG(Gopher);
	SET_PROXY_CONFIG(SOCKS);

	if ([proxyConfig count] > 0) {

		NSArray<NSString *> *domains = nil;
		NSString *matchDomains = self.options[kSCTestConfigAgentProxyMatchDomain];
		if (matchDomains == nil) {
			domains = @[@TEST_DOMAIN];
		} else {
			domains = [matchDomains componentsSeparatedByString:@","];
		}

		[proxyConfig setObject:domains forKey:(__bridge NSString *)kSCPropNetProxiesSupplementalMatchDomains];
	} else {
		proxyConfig = nil;
	}

	return proxyConfig;
#undef SET_PROXY_CONFIG
}

- (NSDictionary *)parseDNSAgentOptions
{
	NSMutableDictionary *dnsConfig;
	NSString *dnsServerString;
	NSString *dnsDomainString;
	NSArray<NSString *> *dnsServers;
	NSArray<NSString *> *dnsDomains;

	dnsConfig = [[NSMutableDictionary alloc] init];
	dnsServerString = self.options[kSCTestConfigAgentDNSServers];
	if (dnsServerString == nil) {
		return nil;
	}

	dnsDomainString = self.options[kSCTestConfigAgentDNSDomains];
	if (dnsDomainString == nil) {
		dnsDomainString = @TEST_DOMAIN;
	}

	dnsServers = [dnsServerString componentsSeparatedByString:@","];
	[dnsConfig setObject:dnsServers forKey:(__bridge NSString *)kSCPropNetDNSServerAddresses];

	dnsDomains = [dnsDomainString componentsSeparatedByString:@","];
	[dnsConfig setObject:dnsDomains forKey:(__bridge NSString *)kSCPropNetDNSSupplementalMatchDomains];

	return dnsConfig;
}

- (void)cleanupAndExitWithErrorCode:(int)error
{
	CFRelease(self.store);
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
	allUnitTestsPassed &= [self unitTestInstallProxy];
	allUnitTestsPassed &= [self unitTestInstallProxyWithLargeConfig];
	allUnitTestsPassed &= [self unitTestInstallProxyWithConflictingDomain];
	allUnitTestsPassed &= [self unitTestInstallDNS];
	allUnitTestsPassed &= [self unitTestInstallDNSWithConflictingDomain];

	if(![self tearDown]) {
		return NO;
	}

	return allUnitTestsPassed;
}

- (BOOL)unitTestInstallProxy
{
	BOOL success = NO;
	SCTestConfigAgent *test;
	NSDictionary *proxyConfig;
	NSString *hostname;
	NSNumber *port;
	NWHostEndpoint *hostEndpoint;
	NWPathEvaluator *pathEvaluator;
	NSMutableDictionary *dict;

	test = [[SCTestConfigAgent alloc] initWithOptions:self.options];
	proxyConfig = [test parseProxyAgentOptions];
	if (proxyConfig == nil) {
		// Use default options
		proxyConfig = @{(__bridge NSString *)kSCPropNetProxiesHTTPEnable:@(1),
				(__bridge NSString *)kSCPropNetProxiesHTTPPort:@(80),
				(__bridge NSString *)kSCPropNetProxiesHTTPProxy:@"10.10.10.100",
				(__bridge NSString *)kSCPropNetProxiesSupplementalMatchDomains:@[@TEST_DOMAIN],
				};
	}

	hostname = [[proxyConfig objectForKey:(__bridge NSString *)kSCPropNetProxiesSupplementalMatchDomains] objectAtIndex:0];
	port = [proxyConfig objectForKey:(__bridge NSString *)kSCPropNetProxiesHTTPPort];
	hostEndpoint = [NWHostEndpoint endpointWithHostname:hostname port:port.stringValue];
	pathEvaluator = [[NWPathEvaluator alloc] initWithEndpoint:hostEndpoint parameters:NULL];
	[pathEvaluator addObserver:test
			forKeyPath:@"path"
			   options:NSKeyValueObservingOptionNew
			   context:nil];

	do {
		[test publishToSCDynamicStore:test.proxyKey value:proxyConfig];
		dict = [NSMutableDictionary dictionaryWithDictionary:proxyConfig];
		[dict removeObjectForKey:(__bridge NSString *)kSCPropNetProxiesSupplementalMatchDomains];
		test.testProxy = @[@[dict]];

		// Wait for the path changes to quiesce
		[test waitFor:PATH_QUIESCE_TIME];
		if (![test.testProxy isEqualToArray:test.pathProxy]) {
			SCTestLog("test proxy and applied proxy do not match. Test: %@, Applied: %@", test.testProxy, test.pathProxy);
			break;
		}

		SCTestLog("Verified the configured proxy is the same as applied proxy");
		[test removeFromSCDynamicStore:test.proxyKey];
		test.testProxy = nil;
		// Wait for the path changes to quiesce
		[test waitFor:PATH_QUIESCE_TIME];
		if (test.pathProxy != nil) {
			SCTestLog("proxy applied when there is no test proxy");
			break;
		}

		success = YES;
	} while(0);

	[pathEvaluator removeObserver:test
			   forKeyPath:@"path"];

	return success;
}

- (BOOL)unitTestInstallDNS
{
	BOOL success = NO;
	SCTestConfigAgent *test;
	NSDictionary *dnsConfig;
	NSString *hostname;
	NWHostEndpoint *hostEndpoint;
	NWPathEvaluator *pathEvaluator;

	test = [[SCTestConfigAgent alloc] initWithOptions:self.options];
	dnsConfig = [test parseDNSAgentOptions];
	if (dnsConfig == nil) {
		dnsConfig = @{	(__bridge NSString *)kSCPropNetDNSServerAddresses:@[@"10.10.10.101", @"10.10.10.102", @"10.10.10.103"],
				(__bridge NSString *)kSCPropNetDNSSupplementalMatchDomains:@[@TEST_DOMAIN],
				};
	}

	hostname = [[dnsConfig objectForKey:(__bridge NSString *)kSCPropNetDNSSupplementalMatchDomains] objectAtIndex:0];
	hostEndpoint = [NWHostEndpoint endpointWithHostname:hostname port:@"80"];
	pathEvaluator = [[NWPathEvaluator alloc] initWithEndpoint:hostEndpoint parameters:NULL];
	[pathEvaluator addObserver:test
			forKeyPath:@"path"
			   options:NSKeyValueObservingOptionNew
			   context:nil];

	do {
		[test publishToSCDynamicStore:test.dnsKey value:dnsConfig];
		test.testDNS = [test createDNSArray:dnsConfig];

		// Wait for the path changes to quiesce
		[test waitFor:PATH_QUIESCE_TIME];
		if (![test.testDNS isEqualToArray:test.pathDNS]) {
			SCTestLog("test DNS and applied DNS do not match. Test: %@, Applied: %@", test.testDNS, test.pathDNS);
			break;
		}

		[test removeFromSCDynamicStore:test.dnsKey];
		test.testDNS = nil;
		[test waitFor:PATH_QUIESCE_TIME];

		SCTestLog("Verified that the configured DNS is same as applied DNS for a domain");
		success = YES;
	} while (0);

	[pathEvaluator removeObserver:test
			   forKeyPath:@"path"];

	return success;
}

- (BOOL)unitTestInstallProxyWithLargeConfig
{
	BOOL success = NO;
	SCTestConfigAgent *test;
	NSString *str = @"0123456789";
	NSMutableString *largeStr;
	NSDictionary *proxyConfig;
	NSString *hostname;
	NSNumber *port;
	NWHostEndpoint *hostEndpoint;
	NWPathEvaluator *pathEvaluator;
	NSMutableDictionary *dict;

	test = [[SCTestConfigAgent alloc] initWithOptions:self.options];
	largeStr = [[NSMutableString alloc] init];
	for (int i = 0; i < 200; i++) {
		[largeStr appendString:str];
	}

	// We imitate a proxy config worth 2K bytes.
	proxyConfig = @{(__bridge NSString *)kSCPropNetProxiesHTTPEnable:@(1),
			(__bridge NSString *)kSCPropNetProxiesHTTPPort:@(80),
			(__bridge NSString *)kSCPropNetProxiesHTTPProxy:@"10.10.10.100",
			(__bridge NSString *)kSCPropNetProxiesProxyAutoConfigJavaScript:largeStr,
			(__bridge NSString *)kSCPropNetProxiesProxyAutoConfigEnable:@(1),
			(__bridge NSString *)kSCPropNetProxiesSupplementalMatchDomains:@[@TEST_DOMAIN],
			};

	hostname = [[proxyConfig objectForKey:(__bridge NSString *)kSCPropNetProxiesSupplementalMatchDomains] objectAtIndex:0];
	port = [proxyConfig objectForKey:(__bridge NSString *)kSCPropNetProxiesHTTPPort];
	hostEndpoint = [NWHostEndpoint endpointWithHostname:hostname port:port.stringValue];
	pathEvaluator = [[NWPathEvaluator alloc] initWithEndpoint:hostEndpoint parameters:NULL];
	[pathEvaluator addObserver:test
			forKeyPath:@"path"
			   options:NSKeyValueObservingOptionNew
			   context:nil];

	do {
		[test publishToSCDynamicStore:test.proxyKey value:proxyConfig];
		dict = [NSMutableDictionary dictionaryWithDictionary:proxyConfig];
		[dict removeObjectForKey:(__bridge NSString *)kSCPropNetProxiesSupplementalMatchDomains];
		test.testProxy = @[@[dict]];

		// Wait for the path changes to quiesce
		[test waitFor:PATH_QUIESCE_TIME];
		if ([test.testProxy isEqualToArray:test.pathProxy]) {
			SCTestLog("applied proxy does not contain Out of Band Agent UUID");
			break;
		}

		// Now we verify that we are able to fetch the proxy configuration from configd
		for (NSArray<NSDictionary *> *config in test.pathProxy) {
			xpc_object_t xpcConfig = _CFXPCCreateXPCObjectFromCFObject((__bridge CFArrayRef)config);
			xpc_object_t fetchedConfig = config_agent_update_proxy_information(xpcConfig);
			if (fetchedConfig != nil) {
				NSArray *nsConfig = (__bridge_transfer NSArray *)(_CFXPCCreateCFObjectFromXPCObject(fetchedConfig));
				test.pathProxy = @[nsConfig];
				break;
			}
		}

		if (![test.testProxy isEqualToArray:test.pathProxy]) {
			SCTestLog("Could not fetch proxy configuration from configd. Test: %@, Applied: %@", test.testProxy, test.pathProxy);
			break;
		}

		SCTestLog("Verified that the proxy configuration is successfully fetched from configd");
		test.testProxy = nil;
		[test removeFromSCDynamicStore:test.proxyKey];

		// Wait for the path changes to quiesce
		[test waitFor:PATH_QUIESCE_TIME];
		if (test.pathProxy != nil) {
			SCTestLog("proxy applied when there is no test proxy");
			break;
		}

		success = YES;
	} while (0);

	[pathEvaluator removeObserver:test
			   forKeyPath:@"path"];

	return success;
}

- (BOOL)unitTestInstallDNSWithConflictingDomain
{
	BOOL success = NO;
	SCTestConfigAgent *test;
	NSDictionary *dnsConfig;
	NSString *hostname;
	NWHostEndpoint *hostEndpoint;
	NWPathEvaluator *pathEvaluator;

	test = [[SCTestConfigAgent alloc] initWithOptions:self.options];
	dnsConfig = @{	(__bridge NSString *)kSCPropNetDNSServerAddresses:@[@"10.10.10.101", @"10.10.10.102", @"10.10.10.103"],
			(__bridge NSString *)kSCPropNetDNSSupplementalMatchDomains:@[@TEST_DOMAIN],
			};

	hostname = [[dnsConfig objectForKey:(__bridge NSString *)kSCPropNetDNSSupplementalMatchDomains] objectAtIndex:0];
	hostEndpoint = [NWHostEndpoint endpointWithHostname:hostname port:@"80"];
	pathEvaluator = [[NWPathEvaluator alloc] initWithEndpoint:hostEndpoint parameters:NULL];
	[pathEvaluator addObserver:test
			forKeyPath:@"path"
			   options:NSKeyValueObservingOptionNew
			   context:nil];

	do {
		NSDictionary *duplicateDnsConfig;
		NSString *anotherFakeServiceID;
		NSString *anotherDNSKey;
		NSArray *array;
		NSSet *testDNSSet;
		NSSet *pathDNSSet;

		[test publishToSCDynamicStore:test.dnsKey value:dnsConfig];
		test.testDNS = [test createDNSArray:dnsConfig];

		// Wait for the path changes to quiesce
		[test waitFor:PATH_QUIESCE_TIME];
		if (![test.testDNS isEqualToArray:test.pathDNS]) {
			SCTestLog("test DNS and applied DNS do not match. Test: %@, Applied: %@", test.testDNS, test.pathDNS);
			break;
		}

		// Now install the conflicting DNS configuration
		duplicateDnsConfig = @{	(__bridge NSString *)kSCPropNetDNSServerAddresses:@[@"10.10.10.104", @"10.10.10.105", @"10.10.10.106"],
					(__bridge NSString *)kSCPropNetDNSSupplementalMatchDomains:@[@TEST_DOMAIN],
					};

		anotherFakeServiceID = [NSUUID UUID].UUIDString;
		anotherDNSKey = (__bridge_transfer NSString *)[self copyStateKeyWithServiceID:(__bridge CFStringRef)(anotherFakeServiceID) forEntity:kSCEntNetDNS];

		[test publishToSCDynamicStore:anotherDNSKey value:duplicateDnsConfig];
		array = [test.testDNS arrayByAddingObjectsFromArray:[test createDNSArray:duplicateDnsConfig]];
		test.testDNS = array;

		// Wait for the path changes to quiesce
		[test waitFor:PATH_QUIESCE_TIME];

		// Use NSSet for unordered comparison
		testDNSSet = [NSSet setWithArray:test.testDNS];
		pathDNSSet = [NSSet setWithArray:test.pathDNS];
		success = [testDNSSet isEqualToSet:pathDNSSet];
		[test removeFromSCDynamicStore:anotherDNSKey];
		if (!success) {
			SCTestLog("test DNS and applied DNS for duplicate domains do not match. Test: %@, Applied: %@", test.testDNS, test.pathDNS);
			break;
		}

		[test removeFromSCDynamicStore:test.dnsKey];
		test.testDNS = nil;
		[test waitFor:PATH_QUIESCE_TIME];

		SCTestLog("Verified that the configured DNS with duplicate domains is same as applied DNS for a domain");
		success = YES;

	} while (0);

	[pathEvaluator removeObserver:test
			   forKeyPath:@"path"];

	return success;
}

- (BOOL)unitTestInstallProxyWithConflictingDomain
{
	BOOL success = NO;
	SCTestConfigAgent *test;
	NSDictionary *proxyConfig;
	NSString *hostname;
	NSNumber *port;
	NWHostEndpoint *hostEndpoint;
	NWPathEvaluator *pathEvaluator;

	test = [[SCTestConfigAgent alloc] initWithOptions:self.options];
	proxyConfig = @{(__bridge NSString *)kSCPropNetProxiesHTTPEnable:@(1),
			(__bridge NSString *)kSCPropNetProxiesHTTPPort:@(80),
			(__bridge NSString *)kSCPropNetProxiesHTTPProxy:@"10.10.10.100",
			(__bridge NSString *)kSCPropNetProxiesSupplementalMatchDomains:@[@TEST_DOMAIN],
			};

	hostname = [[proxyConfig objectForKey:(__bridge NSString *)kSCPropNetProxiesSupplementalMatchDomains] objectAtIndex:0];
	port = [proxyConfig objectForKey:(__bridge NSString *)kSCPropNetProxiesHTTPPort];
	hostEndpoint = [NWHostEndpoint endpointWithHostname:hostname port:port.stringValue];
	pathEvaluator = [[NWPathEvaluator alloc] initWithEndpoint:hostEndpoint parameters:NULL];
	[pathEvaluator addObserver:test
			forKeyPath:@"path"
			   options:NSKeyValueObservingOptionNew
			   context:nil];

	do {
		NSMutableDictionary *dict;
		NSMutableDictionary *dict2;
		NSDictionary *duplicateProxyConfig;
		NSString *anotherFakeServiceID;
		NSString *anotherProxyKey;
		NSSet *testProxySet;
		NSSet *pathProxySet;

		[test publishToSCDynamicStore:test.proxyKey value:proxyConfig];
		dict = [NSMutableDictionary dictionaryWithDictionary:proxyConfig];
		[dict removeObjectForKey:(__bridge NSString *)kSCPropNetProxiesSupplementalMatchDomains];
		test.testProxy = @[@[dict]];

		// Wait for the path changes to quiesce
		[test waitFor:PATH_QUIESCE_TIME];
		if (![test.testProxy isEqualToArray:test.pathProxy]) {
			SCTestLog("test proxy and applied proxy do not match. Test: %@, Applied: %@", test.testProxy, test.pathProxy);
			break;
		}

		// Now install the conflicting Proxy configuration
		duplicateProxyConfig = @{(__bridge NSString *)kSCPropNetProxiesHTTPSEnable:@(1),
							(__bridge NSString *)kSCPropNetProxiesHTTPSPort:@(8080),
							(__bridge NSString *)kSCPropNetProxiesHTTPSProxy:@"10.10.10.101",
							(__bridge NSString *)kSCPropNetProxiesSupplementalMatchDomains:@[@TEST_DOMAIN],
							};
		anotherFakeServiceID = [NSUUID UUID].UUIDString;
		anotherProxyKey = (__bridge_transfer NSString *)[self copyStateKeyWithServiceID:(__bridge CFStringRef)(anotherFakeServiceID) forEntity:kSCEntNetProxies];

		[test publishToSCDynamicStore:anotherProxyKey value:duplicateProxyConfig];;
		dict2 = [NSMutableDictionary dictionaryWithDictionary:duplicateProxyConfig];
		[dict2 removeObjectForKey:(__bridge NSString *)kSCPropNetProxiesSupplementalMatchDomains];
		test.testProxy = @[@[dict],@[dict2]];

		// Wait for the path changes to quiesce
		[test waitFor:PATH_QUIESCE_TIME];

		// Use NSSet for unordered comparison
		testProxySet = [NSSet setWithArray:test.testProxy];
		pathProxySet = [NSSet setWithArray:test.pathProxy];
		success = [testProxySet isEqualToSet:pathProxySet];
		[test removeFromSCDynamicStore:anotherProxyKey];
		if (!success) {
			SCTestLog("test proxy and applied proxy for duplicate domains do not match. Test: %@, Applied: %@", test.testDNS, test.pathDNS);
			break;
		}

		SCTestLog("Verified the configured proxy with Duplicate domains is the same as applied proxy");

		[test removeFromSCDynamicStore:test.proxyKey];
		test.testProxy = nil;

		// Wait for the path changes to quiesce
		[test waitFor:PATH_QUIESCE_TIME];

		if (test.pathProxy != nil) {
			SCTestLog("proxy applied when there is no test proxy");
			break;
		}

		success = YES;

	} while(0);

	[pathEvaluator removeObserver:test
			   forKeyPath:@"path"];

	return success;
}

- (BOOL)tearDown
{
	[self removeFromSCDynamicStore:self.proxyKey];
	[self removeFromSCDynamicStore:self.dnsKey];
	return YES;
}

- (void)observeValueForKeyPath:(NSString *)keyPath
			ofObject:(id)object
			change:(NSDictionary *)change
		context:(void *)context
{
#pragma unused(change)
#pragma unused(context)
	NWPathEvaluator *pathEvaluator = (NWPathEvaluator *)object;
	if ([keyPath isEqualToString:@"path"]) {
		self.pathProxy = pathEvaluator.path.proxySettings;
		self.pathDNS = pathEvaluator.path.dnsServers;
	}
}

@end
