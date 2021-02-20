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

static void
usage()
{
	NSArray *testClasses = getTestClasses();
	SCTestLog("\nUsage: sctest <command> <options>");
	SCTestLog("\nCommands:");
	for (NSString *testClassName in testClasses) {
		Class testClass = NSClassFromString(testClassName);
		SCTestLog("  %15s:   %s", [testClass command].UTF8String, [testClass commandDescription].UTF8String);
	}

	SCTestLog("\n\nOptions:");
	SCTestLog(kSCTestOptionHelp "\n");

	ERR_EXIT;
}

int main(int argc, const char * argv[]) {
	@autoreleasepool {
		NSString *testCommand;
		NSArray<NSString *> *testClasses;
		BOOL commandValid = NO;
		NSDictionary *options;
		Class testClass;
		SCTest *testClassObject;

		if (argc == 1) {
			usage();
		}

		testCommand = @(argv[1]);
		// Check if the command is valid
		testClasses = getTestClasses();
		for (NSString *testClassName in testClasses) {
			Class testClass = NSClassFromString(testClassName);
			if ([[testClass command] isEqualToString:testCommand]) {
				commandValid = YES;
				break;
			}

		}

		if (!commandValid) {
			SCTestLog("Invalid command: %@", testCommand);
			usage();
		}

		// Create the options dictionary
		options = getOptionsDictionary(argc, argv);
		if (options == nil) {
			usage();
		}

		// Initialize the command
		for (NSString *className in testClasses) {
			Class	commandClass = NSClassFromString(className);
			if ([testCommand isEqualToString:[commandClass command]]) {
				testClass = commandClass;
				break;
			}
		}

		testClassObject = [(SCTest *)[testClass alloc] initWithOptions:options];
		if (testClassObject.options[kSCTestGlobalOptionCPU] != nil) {
			cpuStart(testClassObject.globalCPU);
		}

		if (testClassObject.options[kSCTestGlobalOptionTime] != nil) {
			timerStart(testClassObject.globalTimer);
		}

		[testClassObject start];

		dispatch_main();
	}

	return 0;
}
