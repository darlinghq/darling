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
#import "SCTestUtils.h"

@interface SCTestUnitTest : SCTest
@end

@implementation SCTestUnitTest

+ (NSString *)command
{
	return @"unit_test";
}

+ (NSString *)commandDescription
{
	return @"Runs the unit test for all commands";
}

- (void)listTests
{
	NSMutableDictionary *testDictionary;
	NSMutableArray *testsArray;
	NSString *thisClass;
	NSArray<NSString *> *testClasses;
	NSData *data;
	NSString *jsonString;

	testDictionary = [[NSMutableDictionary alloc] init];
	[testDictionary setObject:@"SystemConfiguration Unit Tests" forKey:@"Name"];
	[testDictionary setObject:@"These tests exercise 'configd' and the 'SystemConfiguration' framework" forKey:@"Description"];

	testsArray = [[NSMutableArray alloc] init];
	thisClass = NSStringFromClass([self class]);
	testClasses = getTestClasses();
	for (NSString *className in testClasses) {
		Class testClass;
		NSMutableDictionary *subTest;
		NSArray *list;
		NSMutableArray *subTestArray;

		if ([className isEqualToString:thisClass] ) {
			continue;
		}

		testClass = NSClassFromString(className);
		list = getUnitTestListForClass(testClass);

		subTest = [[NSMutableDictionary alloc] init];
		[subTest setObject:@NO forKey:@"RequiresTCPDUMP"];
		[subTest setObject:@YES forKey:@"RequiresNetwork"];
		[subTest setObject:@NO forKey:@"RequiresRoot"];
		[subTest setObject:@NO forKey:@"RequiresPowermetrics"];
		[subTest setObject:[testClass command] forKey:@"Name"];
		[subTest setObject:[testClass commandDescription] forKey:@"Description"];
		subTestArray = [[NSMutableArray alloc] init];
		for (NSString *unitTest in list) {
			NSDictionary *testDict = @{@"Command":@[@"/usr/local/bin/sctest",
								@"unit_test",
								@"-test_method",
								unitTest],
							@"Name":[unitTest stringByReplacingOccurrencesOfString:@"unitTest" withString:@""],
							@"Description":@"Unit test"
							};
			[subTestArray addObject:testDict];
		}
		[subTest setObject:subTestArray forKey:@"SubTests"];
		[testsArray addObject:subTest];
	}
	[testDictionary setObject:testsArray forKey:@"Tests"];
	data = [NSJSONSerialization dataWithJSONObject:testDictionary
						options:NSJSONWritingPrettyPrinted
						error:nil];
	jsonString = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
	//SCTestLog("%@", jsonString);
	SCPrint(TRUE, stderr, CFSTR("%@"), jsonString);
}

- (void)start
{
	NSArray<NSString *> *testClasses;
	NSString *thisClass = NSStringFromClass([self class]);;
	testClasses = getTestClasses();
	BOOL errorOccured = NO;

	if (self.options[kSCTestUnitTestListTests]) {
		[self listTests];
	} else if (self.options[kSCTestUnitTestTestMethodList]) {
		SCTestLog("List of unit tests:");
		for (NSString *className in testClasses) {
			Class testClass;
			NSArray *list;

			if ([className isEqualToString:thisClass] ) {
				continue;
			}

			testClass = NSClassFromString(className);
			if (self.options[kSCTestUnitTestCommand] != nil) {
				if (![self.options[kSCTestUnitTestCommand] isEqualToString:[testClass command]]) {
					// Run unit test only for a specific command
					continue;
				}
			}

			SCTestLog("\n======= '%@' unit tests =======", [testClass command]);
			list = getUnitTestListForClass(testClass);
			for (NSString *unitTest in list) {
				SCTestLog("%@", unitTest);
			}
		}

		SCTestLog("\nEach of the unit tests can be run with the 'test_method' option\n");
	} else if (self.options[kSCTestUnitTestTestMethod]) {
		for (NSString *className in testClasses) {
			Class testClass;
			NSArray *list;

			if ([className isEqualToString:thisClass] ) {
				continue;
			}

			testClass = NSClassFromString(className);
			if (self.options[kSCTestUnitTestCommand] != nil) {
				if (![self.options[kSCTestUnitTestCommand] isEqualToString:[testClass command]]) {
					// Run unit test only for a specific command
					continue;
				}
			}

			list = getUnitTestListForClass(testClass);
			for (NSString *unitTest in list) {
				if ([unitTest isEqualToString:self.options[kSCTestUnitTestTestMethod]]) {
					id obj = [(SCTest *)[testClass alloc] initWithOptions:self.options];

					SCTestLog("Running unit test %@ ...", unitTest);

					SEL methodSelector = NSSelectorFromString(unitTest);
					Boolean retVal = false;
					if ([obj respondsToSelector:methodSelector]) {
						NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:[obj methodSignatureForSelector:methodSelector]];
						invocation.target = obj;
						invocation.selector = methodSelector;
						[invocation invoke];
						[invocation getReturnValue:&retVal];
					}

					if (!retVal) {
						SCTestLog("FAILED");
						errorOccured = YES;
					} else {
						SCTestLog("PASSED");
					}
					break;
				}
			}
		}
	} else {
		// This command runs unit tests for all commands.
		for (NSString *className in testClasses) {
			Class testClass;
			id obj;

			if ([className isEqualToString:thisClass] ) {
				continue;
			}

			testClass = NSClassFromString(className);
			if (self.options[kSCTestUnitTestCommand] != nil) {
				if (![self.options[kSCTestUnitTestCommand] isEqualToString:[testClass command]]) {
					// Run unit test only for a specific command
					continue;
				}
			}

			obj = [(SCTest *)[testClass alloc] initWithOptions:self.options];
			if ([obj respondsToSelector:@selector(unitTest)]) {
				SCTestLog("\n*** Running unit test for \"%@\" command ***\n", [testClass command]);
				BOOL passed = [obj unitTest];
				if (!passed) {
					SCTestLog("FAILED");
					errorOccured = YES;
				}
			}
		}
	}

	[self cleanupAndExitWithErrorCode:errorOccured];
}

- (void)cleanupAndExitWithErrorCode:(int)error
{
	[super cleanupAndExitWithErrorCode:error];
}

@end
