/*
 * Copyright (c) 2016 Apple Inc. All rights reserved.
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

@implementation SCTest

- (instancetype)initWithOptions:(NSDictionary *)options
{
	self = [super init];
	if (self) {
		self.options = options;
		self.globalCPU = malloc(sizeof(CPUUsageInfo));
		self.globalTimer = malloc(sizeof(timerInfo));
	}
	return self;
}

- (void)dealloc
{
	if (self.globalTimer != NULL) {
		free(self.globalTimer);
	}
	if (self.globalCPU != NULL) {
		free(self.globalCPU);
	}
}

+ (NSString *)command
{
	return @"sctest";
}

+ (NSString *)commandDescription
{
	return @"This is a generic class";
}

+ (NSString *)commandHelp
{
	return @"This is a generic help";
}

- (void)start
{
	return;
}

- (void)waitFor:(double)seconds
{
	dispatch_semaphore_t sem = dispatch_semaphore_create(0);
	dispatch_source_t timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0));
	dispatch_source_set_timer(timer, dispatch_time(DISPATCH_TIME_NOW, seconds * NSEC_PER_SEC), DISPATCH_TIME_FOREVER, 0);
	dispatch_source_set_event_handler(timer, ^{
		dispatch_semaphore_signal(sem);
	});
	dispatch_resume(timer);
	dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER);
}

- (void)cleanupAndExitWithErrorCode:(int)error
{
	if (self.options[kSCTestGlobalOptionTime] != nil) {
		timerEnd(self.globalTimer);
		SCTestLog("Time: %@ s", createUsageStringForTimer(self.globalTimer));
	}

	if (self.options[kSCTestGlobalOptionCPU] != nil) {
		cpuEnd(self.globalCPU);
		SCTestLog("CPU: %@", createUsageStringForCPU(self.globalCPU));
	}

	if (self.options[kSCTestGlobalOptionWait] != nil) {
		return;
	}
	exit(error);
}

- (BOOL)unitTest
{
	return YES;
}

@end
