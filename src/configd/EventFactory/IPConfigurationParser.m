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
#import "IPConfigurationParser.h"

#define TokenLinkStatus "linkStatus"
#define TokenSSID "ssid"
#define TokenMessage "message"
#define TokenAddress "address"
#define TokenComponentName "component"

@implementation IPConfigurationParser

- (instancetype)init
{
	NSArray<EFLogEventMatch *> *matches = @[
		[[EFLogEventMatch alloc] initWithPattern:@"(?<"TokenInterfaceName">\\w+) link (?<"TokenLinkStatus">ACTIVE|INACTIVE)"
					 newEventHandler:
		 ^EFEvent *(NSTextCheckingResult *matchResult, EFLogEvent *logEvent, BOOL *isComplete) {
			 NSString *statusString = [logEvent substringForCaptureGroup:@TokenLinkStatus inMatchResult:matchResult];
			 EFNetworkControlPathEvent *newEvent = [self createInterfaceEventWithLogEvent:logEvent matchResult:matchResult];
			 if ([statusString isEqualToString:@"ACTIVE"]) {
				 newEvent.link = @"link up";
			 } else {
				 newEvent.link = @"link down";
			 }
			 *isComplete = YES;
			 return newEvent;
		 }],
		[[EFLogEventMatch alloc] initWithPattern:@"(?<"TokenInterfaceName">\\w+): SSID (?<"TokenSSID">\\S+) BSSID"
					 newEventHandler:
		 ^EFEvent *(NSTextCheckingResult *matchResult, EFLogEvent *logEvent, BOOL *isComplete) {
			 NSString *ssid = [logEvent substringForCaptureGroup:@TokenSSID inMatchResult:matchResult];
			 EFNetworkControlPathEvent *newEvent = [self createInterfaceEventWithLogEvent:logEvent matchResult:matchResult];
			 EFSubEvent *subEvent = [[EFSubEvent alloc] initWithTimestamp:logEvent.date textDescription:ssid];
			 [newEvent addSubEvent:subEvent];
			 *isComplete = YES;
			 return newEvent;
		 }],
		[[EFLogEventMatch alloc] initWithPattern:@"\\[(?<"TokenComponentName">\\w+ )?(?<"TokenInterfaceName">\\w+)\\] (?<"TokenMessage">(?:Transmit|Receive) \\d+ byte packet xid \\w+ (?:to|from) .*)"
					 newEventHandler:
		 ^EFEvent *(NSTextCheckingResult *matchResult, EFLogEvent *logEvent, BOOL *isComplete) {
			 NSString *message = [logEvent substringForCaptureGroup:@TokenMessage inMatchResult:matchResult];
			 NSString *component = [logEvent substringForCaptureGroup:@TokenComponentName inMatchResult:matchResult];
			 EFNetworkControlPathEvent *newEvent = [self createInterfaceEventWithLogEvent:logEvent matchResult:matchResult];
			 NSString *description = [[NSString alloc] initWithFormat:@"%@ %@", component, message];
			 EFSubEvent *subEvent = [[EFSubEvent alloc] initWithTimestamp:logEvent.date textDescription:description];
			 [newEvent addSubEvent:subEvent];
			 *isComplete = YES;
			 return newEvent;
		 }],
		[[EFLogEventMatch alloc] initWithPattern:@"\\w+ (?<"TokenInterfaceName">\\w+): setting (?<"TokenAddress">\\S+) netmask \\S+ broadcast \\S+"
					 newEventHandler:
		 ^EFEvent *(NSTextCheckingResult *matchResult, EFLogEvent *logEvent, BOOL *isComplete) {
			 *isComplete = YES;
			 NSString *addressString = [logEvent substringForCaptureGroup:@TokenAddress inMatchResult:matchResult];
			 if (addressString.length > 0) {
				 EFNetworkControlPathEvent *newEvent = [self createInterfaceEventWithLogEvent:logEvent matchResult:matchResult];
				 [self addAddress:addressString toInterfaceEvent:newEvent];
				 return newEvent;
			 }
			 return nil;
		 }],
		[[EFLogEventMatch alloc] initWithPattern:@"\\w+ (?<"TokenInterfaceName">\\w+): removing (?<"TokenAddress">.+)"
					 newEventHandler:
		 ^EFEvent *(NSTextCheckingResult *matchResult, EFLogEvent *logEvent, BOOL *isComplete) {
			 *isComplete = YES;
			 NSString *addressString = [logEvent substringForCaptureGroup:@TokenAddress inMatchResult:matchResult];
			 if (addressString.length > 0) {
				 EFNetworkControlPathEvent *newEvent = [self createInterfaceEventWithLogEvent:logEvent matchResult:matchResult];
				 if ([self removeAddress:addressString fromInterfaceEvent:newEvent]) {
					 return newEvent;
				 }
			 }
			 return nil;
		 }]
	];

	EFLogEventParser *parser = [[EFLogEventParser alloc] initWithMatches:matches];
	return [super initWithCategory:@"IPConfiguration" eventParser:parser];
}

@end
