/*
 * Copyright (c) 2017-2018 Apple Inc. All rights reserved.
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

/*
 * Modification History
 *
 * November 15, 2017	Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#import <os/log.h>
#import "EventFactory.h"
#import "SCLogParser.h"
#import "InterfaceNamerParser.h"
#import "IPMonitorParser.h"
#import "KernelEventMonitorParser.h"
#import "PreferencesMonitorParser.h"
#import "StateDumpParser.h"
#import "IPConfigurationParser.h"

#pragma mark -
#pragma mark Logging

os_log_t
__log_Spectacles(void)
{
	static os_log_t	log	= NULL;

	if (log == NULL) {
		log = os_log_create("com.apple.spectacles", "SystemConfiguration");
	}

	return log;
}

#pragma mark -
#pragma mark SystemConfiguratioin Network Event Factory

@interface EventFactory ()
@property NSDictionary<NSString *, SCLogParser *> *parserMap;
@end

@implementation EventFactory

- (void)startWithLogSourceAttributes:(__unused NSDictionary<NSString *, NSObject *> *)attributes
{
	NSMutableDictionary<NSString *, SCLogParser *> *newParserMap = [[NSMutableDictionary alloc] init];
	SCLogParser *parser;

	parser = [[InterfaceNamerParser alloc] init];
	newParserMap[parser.category] = parser;

	parser = [[IPConfigurationParser alloc] init];
	newParserMap[parser.category] = parser;

	parser = [[IPMonitorParser alloc] init];
	newParserMap[parser.category] = parser;

	parser = [[KernelEventMonitorParser alloc] init];
	newParserMap[parser.category] = parser;

	parser = [[PreferencesMonitorParser alloc] init];
	newParserMap[parser.category] = parser;

	parser = [[StateDumpParser alloc] init];
	newParserMap[parser.category] = parser;

	_parserMap = [[NSDictionary alloc] initWithDictionary:newParserMap];
}

- (void)handleLogEvent:(EFLogEvent *)logEvent completionHandler:(void (^)(NSArray<EFEvent *> * _Nullable))completionHandler
{
	NSString *category = nil;
	if ([logEvent.eventType isEqualToString:@"stateEvent"]) {
		category = @"StateDump";
		logEvent.subsystem = @"com.apple.SystemConfiguration";
		logEvent.category = category;
	} else if ([logEvent.subsystem isEqualToString:@"com.apple.IPConfiguration"]) {
		logEvent.category = @"IPConfiguration";
	}

	if (logEvent.category.length == 0) {
		specs_log_debug("Skipped message without a category: %@", logEvent.eventMessage);
		completionHandler(nil);
		return;
	}

	SCLogParser *parser = _parserMap[logEvent.category];
	if (parser == nil) {
		specs_log_debug("Skipped message with an unknown category (%@): %@", logEvent.category, logEvent.eventMessage);
		completionHandler(nil);
		return;
	}

	NSArray<EFEvent *> *completeEvents = [parser.eventParser parseLogEventIntoMultipleEvents:logEvent];
	completionHandler(completeEvents);
}

- (void)finishWithCompletionHandler:(void (^)(NSArray<EFEvent *> * _Nullable))completionHandler
{
	specs_log_notice("Event factory is finishing");
	completionHandler(nil);
}

@end


