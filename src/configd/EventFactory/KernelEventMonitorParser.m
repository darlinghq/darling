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

#import "KernelEventMonitorParser.h"

#define TokenStatus "status"
#define TokenLinkStatus "linkStatus"
#define TokenLinkQuality "linkQuality"

@implementation KernelEventMonitorParser

- (instancetype)init
{
	NSArray<EFLogEventMatch *> *matches = @[
		[[EFLogEventMatch alloc] initWithPattern:@"Process interface (?<"TokenStatus">attach|detach): (?<"TokenInterfaceName">\\w+)"
					 newEventHandler:
		 ^EFEvent *(NSTextCheckingResult *matchResult, EFLogEvent *logEvent, BOOL *isComplete) {
			 *isComplete = YES;
			 EFNetworkControlPathEvent *newEvent = nil;
			 NSString *statusString = [logEvent substringForCaptureGroup:@TokenStatus inMatchResult:matchResult];
			 if (statusString != nil) {
				 newEvent = [self createInterfaceEventWithLogEvent:logEvent matchResult:matchResult];
				 if (newEvent != nil) {
					newEvent.interfaceStatus = statusString;
				 }
			 }
			 return newEvent;
		 }],
		[[EFLogEventMatch alloc] initWithPattern:@"Process interface link (?<"TokenLinkStatus">down|up): (?<"TokenInterfaceName">\\w+)"
					 newEventHandler:
		 ^EFEvent *(NSTextCheckingResult *matchResult, EFLogEvent *logEvent, BOOL *isComplete) {
			 *isComplete = YES;
			 EFNetworkControlPathEvent *newEvent = nil;
			 NSString *linkStatusString = [logEvent substringForCaptureGroup:@TokenLinkStatus inMatchResult:matchResult];
			 if (linkStatusString != nil) {
				 newEvent = [self createInterfaceEventWithLogEvent:logEvent matchResult:matchResult];
				 if (newEvent != nil) {
					 if ([linkStatusString isEqualToString:@"up"]) {
						 newEvent.link = @"link up";
					 } else if ([linkStatusString isEqualToString:@"down"]) {
						 newEvent.link = @"link down";
					 } else {
						 newEvent.link = linkStatusString;
					 }
				 }
			 }
			 return newEvent;
		 }],
		[[EFLogEventMatch alloc] initWithPattern:@"Process interface quality: (?<"TokenInterfaceName">\\w+) \\(q=(?<"TokenLinkQuality">[-\\d]+)\\)"
					 newEventHandler:
		 ^EFEvent * _Nullable(NSTextCheckingResult * _Nonnull matchResult, EFLogEvent * _Nonnull logEvent, BOOL * _Nonnull isComplete) {
			 *isComplete = YES;
			 EFNetworkControlPathEvent *newEvent = nil;
			 NSString *qualityString = [logEvent substringForCaptureGroup:@TokenLinkQuality inMatchResult:matchResult];
			 if (qualityString != nil) {
				 newEvent = [self createInterfaceEventWithLogEvent:logEvent matchResult:matchResult];
				 if (newEvent != nil) {
					newEvent.linkQuality = qualityString.integerValue;
				 }
			 }
			 return newEvent;
		 }],
	];
	EFLogEventParser *parser = [[EFLogEventParser alloc] initWithMatches:matches];
	return [super initWithCategory:@"KernelEventMonitor" eventParser:parser];
}

@end
