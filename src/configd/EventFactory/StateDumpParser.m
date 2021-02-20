/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
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

#import <Foundation/Foundation.h>
#import <EventFactory/EventFactory.h>
#import <arpa/inet.h>

#import "StateDumpParser.h"

#define TokenFlagsDescription "flagsDescription"
#define TokenAddress "address"
#define TokenReachabilityDescription "reachabilityDescription"
#define TokenRank "rank"
#define TokenOrder "order"
#define TokenDomain "domain"
#define TokenSearchDomains "searchDomains"
#define TokenNameServers "nameServers"

#define ResolverSearchDomainsKey @"searchDomains"
#define ResolverNameServersKey @"nameServers"
#define ResolverInterfaceNameKey @"interfaceName"
#define ResolverFlagsDescriptionKey @"flagsDescription"
#define ResolverReachabilityDescriptionKey @"reachabilityDescription"
#define ResolverMatchDomainsKey @"matchDomains"

@interface StateDumpParser ()
@property (readonly, nonatomic) NSRegularExpression *nwiRegex;
@property (readonly, nonatomic) NSRegularExpression *dnsRegex;
@property (readonly, nonatomic) NSRegularExpression *nameserverRegex;
@property (readonly, nonatomic) NSRegularExpression *searchDomainRegex;
@end

@implementation StateDumpParser

- (instancetype)init
{
	NSError *regexError = nil;

	_nwiRegex = [[NSRegularExpression alloc] initWithPattern:@"\\s+(?<"TokenInterfaceName">\\w+) : flags\\s+: \\w+ \\(.+\\)\\n"
		     						  "\\s+address\\s+: (?<"TokenAddress">\\S+)\\n"
		     						  "(\\s+VPN server\\s+: \\S+\\n)?"
		     						  "\\s+reach\\s+: \\w+ \\(.+\\)\\n"
		     						  "\\s+rank\\s+: \\w+ \\((?<"TokenRank">\\w+), (?<"TokenOrder">\\w+)\\)"
							 options:0
							   error:&regexError];
	if (_nwiRegex == nil || regexError != nil) {
		specs_log_err("Failed to create NWI regex: %@", regexError);
		return nil;
	}

	regexError = nil;
	_dnsRegex = [[NSRegularExpression alloc] initWithPattern:@"resolver #\\d+\\n"
		     						  "(  domain   : (?<"TokenDomain">\\S+)\\n)?"
		     						  "(?<"TokenSearchDomains">(?:  search domain\\[\\d+\\] : \\S+\\n)*)"
		     						  "(?<"TokenNameServers">(?:  nameserver\\[\\d+\\] : \\S+\\n)*)"
		     						  "(  if_index : \\d+ \\((?<"TokenInterfaceName">\\w+)\\)\\n)"
		     						  "(  flags    : \\w+ \\((?<"TokenFlagsDescription">.+)\\)\\n)"
		     						  "(  reach    : \\w+ \\((?<"TokenReachabilityDescription">.+)\\)\\n)"
							 options:0
							   error:&regexError];
	if (_dnsRegex == nil || regexError != nil) {
		specs_log_err("Failed to create DNS configuration regex: %@", regexError);
		return nil;
	}

	regexError = nil;
	_nameserverRegex = [[NSRegularExpression alloc] initWithPattern:@"  nameserver\\[\\d+\\] : (?<"TokenAddress">\\S+)\\n"
								options:0
								  error:&regexError];
	if (_nameserverRegex == nil || regexError != nil) {
		specs_log_err("Failed to create the nameserver regex: %@", regexError);
		return nil;
	}

	regexError = nil;
	_searchDomainRegex = [[NSRegularExpression alloc] initWithPattern:@"  search domain\\[\\d+\\] : (?<"TokenDomain">\\S+)\\n"
								  options:0
								    error:&regexError];
	if (_searchDomainRegex == nil || regexError != nil) {
		specs_log_err("Failed to create the search domain regex: %@", regexError);
		return nil;
	}

	NSArray<EFLogEventMatch *> *matches = @[
		[[EFLogEventMatch alloc] initWithPattern:@"^Network information"
				 multipleNewEventHandler:
		 ^NSArray<EFEvent *> *(__unused NSTextCheckingResult *matchResult, EFLogEvent *logEvent) {
			 NSMutableDictionary<NSString *, EFNetworkControlPathEvent *> *newEvents = nil;
			 NSArray<NSTextCheckingResult *> *matches = [self.nwiRegex matchesInString:logEvent.eventMessage options:0 range:NSMakeRange(0, logEvent.eventMessage.length)];
			 BOOL primaryV4 = YES;
			 BOOL primaryV6 = YES;

			 for (NSString *interfaceName in SCLogParser.interfaceMap.allKeys) {
				 SCLogParser.interfaceMap[interfaceName] = @[ ];
			 }

			 for (NSTextCheckingResult *match in matches) {
				 NSString *interfaceName = [logEvent substringForCaptureGroup:@TokenInterfaceName inMatchResult:match];
				 if (interfaceName == nil) {
					 continue;
				 }
				 EFNetworkControlPathEvent *event = newEvents[interfaceName];
				 if (event == nil) {
					 event = [self createInterfaceEventWithLogEvent:logEvent matchResult:match];
					 if (newEvents == nil) {
						 newEvents = [[NSMutableDictionary alloc] init];
					 }
					 newEvents[event.interfaceBSDName] = event;
					 event.primaryStateIPv4 = EFPrimaryStateNotPrimary;
					 event.primaryStateIPv6 = EFPrimaryStateNotPrimary;
				 }
				 NSString *addressString = [logEvent substringForCaptureGroup:@TokenAddress inMatchResult:match];
				 if (addressString.length > 0) {
					 if (primaryV4 || primaryV6) {
						 sa_family_t addressFamily = [self getAddressFamilyOfAddress:addressString];
						 if (primaryV4 && addressFamily == AF_INET) {
							 event.primaryStateIPv4 = EFPrimaryStatePrimary;
							 primaryV4 = NO;
						 } else if (primaryV6 && addressFamily == AF_INET6) {
							 event.primaryStateIPv6 = EFPrimaryStatePrimary;
							 primaryV6 = NO;
						 }
					 }
					 [self addAddress:addressString toInterfaceEvent:event];
				 }
				 NSString *rankString = [logEvent substringForCaptureGroup:@TokenRank inMatchResult:match];
				 if (rankString.length > 0) {
					 event.rank = rankString;
				 }
				 NSString *orderString = [logEvent substringForCaptureGroup:@TokenOrder inMatchResult:match];
				 if (orderString.length > 0) {
					 if ([orderString isEqualToString:@"Last"]) {
						 event.order = -1;
					 } else {
						 event.order = orderString.integerValue;
					 }
				 }
			 }
			 return newEvents.allValues;
		 }],
		[[EFLogEventMatch alloc] initWithPattern:@"^DNS Configuration"
				 multipleNewEventHandler:
		 ^NSArray<EFEvent *> *(__unused NSTextCheckingResult *matchResult, EFLogEvent *logEvent) {
			 NSMutableArray<EFNetworkControlPathEvent *> *newEvents = nil;
			 NSArray<NSTextCheckingResult *> *matches = [self.dnsRegex matchesInString:logEvent.eventMessage options:0 range:NSMakeRange(0, logEvent.eventMessage.length)];
			 NSMutableDictionary<NSString *, NSDictionary<NSString *, NSObject *> *> *interfaceDNSConfigurations = nil;
			 NSMutableArray<NSDictionary<NSString *, NSObject *> *> *orderedDNSConfigurations = nil;

			 for (NSTextCheckingResult *match in matches) {
				 NSMutableDictionary<NSString *, NSObject *> *dnsConfiguration = [[NSMutableDictionary alloc] init];
				 NSString *matchDomain = [logEvent substringForCaptureGroup:@TokenDomain inMatchResult:match];
				 BOOL scoped = NO;
				 if (matchDomain.length > 0) {
					 NSArray<NSString *> *domains = (NSArray<NSString *> *)dnsConfiguration[ResolverMatchDomainsKey];
					 dnsConfiguration[ResolverMatchDomainsKey] = [self addUniqueString:matchDomain toArray:domains];
				 }
				 NSString *searchDomainsString = [logEvent substringForCaptureGroup:@TokenSearchDomains inMatchResult:match];
				 if (searchDomainsString.length > 0) {
					 [self addSubstringsFromString:searchDomainsString
						       forCaptureGroup:@TokenDomain
							       inRegex:self.searchDomainRegex
							  toArrayAtKey:ResolverSearchDomainsKey
							  inDictionary:dnsConfiguration];
				 }
				 NSString *nameServersString = [logEvent substringForCaptureGroup:@TokenNameServers inMatchResult:match];
				 if (nameServersString.length > 0) {
					 [self addSubstringsFromString:nameServersString
						       forCaptureGroup:@TokenAddress
							       inRegex:self.nameserverRegex
							  toArrayAtKey:ResolverNameServersKey
							  inDictionary:dnsConfiguration];
				 }
				 NSString *flagsDescription = [logEvent substringForCaptureGroup:@TokenFlagsDescription inMatchResult:match];
				 if (flagsDescription.length > 0) {
					 dnsConfiguration[ResolverFlagsDescriptionKey] = flagsDescription;
					 if ([flagsDescription containsString:@"Scoped"]) {
						 scoped = YES;
					 }
				 }
				 NSString *reachabilityDescription = [logEvent substringForCaptureGroup:@TokenReachabilityDescription inMatchResult:match];
				 if (reachabilityDescription.length > 0) {
					 dnsConfiguration[ResolverReachabilityDescriptionKey] = reachabilityDescription;
				 }

				 NSString *interfaceName = [logEvent substringForCaptureGroup:@TokenInterfaceName inMatchResult:match];
				 if (interfaceName != nil) {
					 dnsConfiguration[ResolverInterfaceNameKey] = interfaceName;
				 }

				 NSDictionary<NSString *, NSObject *> *newConfiguration = nil;
				 if (interfaceName != nil && (scoped || matchDomain.length > 0)) {
					 NSDictionary<NSString *, NSObject *> *existingConfiguration = interfaceDNSConfigurations[interfaceName];
					 if (existingConfiguration != nil) {
						 if (matchDomain.length > 0) {
							 NSArray<NSString *> *matchDomains = (NSArray<NSString *> *)existingConfiguration[ResolverMatchDomainsKey];
							 dnsConfiguration[ResolverMatchDomainsKey] = [self addUniqueString:matchDomain toArray:matchDomains];
						 } else {
							 dnsConfiguration[ResolverMatchDomainsKey] = existingConfiguration[ResolverMatchDomainsKey];
						 }
					 }
					 newConfiguration = [[NSDictionary alloc] initWithDictionary:dnsConfiguration];
					 if (interfaceDNSConfigurations == nil) {
						 interfaceDNSConfigurations = [[NSMutableDictionary alloc] init];
					 }
					 interfaceDNSConfigurations[interfaceName] = newConfiguration;
				 } else {
					 newConfiguration = [[NSDictionary alloc] initWithDictionary:dnsConfiguration];
				 }

				 if (!scoped) {
					 if (orderedDNSConfigurations == nil) {
						 orderedDNSConfigurations = [[NSMutableArray alloc] init];
					 }
					 NSUInteger existingIndex = [orderedDNSConfigurations indexOfObjectPassingTest:
						^BOOL(NSDictionary<NSString *,NSObject *> *obj, __unused NSUInteger idx, __unused BOOL *stop) {
							NSString *existingInterfaceName = (NSString *)obj[ResolverInterfaceNameKey];
							return (existingInterfaceName != nil && [interfaceName isEqualToString:existingInterfaceName]);
						}];
					 if (existingIndex == NSNotFound) {
						 [orderedDNSConfigurations addObject:newConfiguration];
					 } else {
						 orderedDNSConfigurations[existingIndex] = newConfiguration;
					 }
				 }
			 }

			 for (NSString *interfaceName in interfaceDNSConfigurations) {
				 NSDictionary<NSString *, NSObject *> *dnsConfiguration = interfaceDNSConfigurations[interfaceName];
				 if (dnsConfiguration == nil || ![NSJSONSerialization isValidJSONObject:dnsConfiguration]) {
					 specs_log_err("DNS configuration is not valid JSON: %@", dnsConfiguration);
					 continue;
				 }
				 NSError *jsonError = nil;
				 NSData *jsonData = [NSJSONSerialization dataWithJSONObject:dnsConfiguration options:NSJSONWritingPrettyPrinted error:&jsonError];
				 if (jsonData == nil) {
					 specs_log_err("Failed to generate JSON from %@: %@", dnsConfiguration, jsonError);
					 continue;
				 }
				 EFNetworkControlPathEvent *newEvent = [self createInterfaceEventWithLogEvent:logEvent interfaceName:interfaceName];
				 newEvent.dnsConfiguration = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];

				 if (newEvents == nil) {
					 newEvents = [[NSMutableArray alloc] init];
				 }
				 [newEvents addObject:newEvent];
			 }
			 if (orderedDNSConfigurations.count > 0) {
				 NSError *jsonError = nil;
				 NSData *jsonData = [NSJSONSerialization dataWithJSONObject:orderedDNSConfigurations options:NSJSONWritingPrettyPrinted error:&jsonError];
				 if (jsonData != nil) {
					 NSData *subsystemIdentifier = [self createSubsystemIdentifier];
					 EFNetworkControlPathEvent *newEvent = [[EFNetworkControlPathEvent alloc] initWithLogEvent:logEvent subsystemIdentifier:subsystemIdentifier];
					 newEvent.interfaceBSDName = @"system";
					 newEvent.interfaceDescription = @"System Configuration";
					 newEvent.dnsConfiguration = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];

					 if (newEvents == nil) {
						 newEvents = [[NSMutableArray alloc] init];
					 }
					 [newEvents addObject:newEvent];
				 }
			 }
			 for (NSString *interfaceName in SCLogParser.interfaceMap) {
				 if (interfaceDNSConfigurations[interfaceName] == nil) {
					 EFNetworkControlPathEvent *newEvent = [self createInterfaceEventWithLogEvent:logEvent interfaceName:interfaceName];
					 newEvent.dnsConfiguration = EFNetworkControlPathEvent.configurationNotSet;
					 if (newEvents == nil) {
						 newEvents = [[NSMutableArray alloc] init];
					 }
					 [newEvents addObject:newEvent];
				 }
			 }
			 return newEvents;
		 }],
	];
	EFLogEventParser *parser = [[EFLogEventParser alloc] initWithMatches:matches];
	return [super initWithCategory:@"StateDump" eventParser:parser];
}

- (void)addSubstringsFromString:(NSString *)string forCaptureGroup:(NSString *)groupName inRegex:(NSRegularExpression *)regex toArrayAtKey:(NSString *)configKey inDictionary:(NSMutableDictionary<NSString *, NSObject *> *)dictionary
{
	NSArray<NSTextCheckingResult *> *matches = [regex matchesInString:string options:0 range:NSMakeRange(0, string.length)];
	for (NSTextCheckingResult *match in matches) {
		NSRange groupRange = [match rangeWithName:groupName];
		if (!NSEqualRanges(groupRange, NSMakeRange(NSNotFound, 0))) {
			NSString *substring = [string substringWithRange:groupRange];
			if (substring.length > 0) {
				NSArray<NSString *> *existingList = (NSArray<NSString *> *)dictionary[configKey];
				dictionary[configKey] = [self addUniqueString:substring toArray:existingList];
			}
		}
	}
}

@end
