/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#import <RecapPerformanceTesting/RecapPerformanceTesting.h>

@implementation RPTTestRunner

+ (BOOL)runTestWithParameters:(id<RPTTestParameters>)parameters
{
	NSLog(@"RecapPerformanceTesting: performance test '%@' is not supported", [parameters testName]);
	return NO;
}

- (BOOL)runTestWithParameters:(id<RPTTestParameters>)parameters
{
	return [RPTTestRunner runTestWithParameters:parameters];
}

@end

@implementation RPTResizeTestParameters {
	void (^_completionHandler)(void);
}

@synthesize testName = _testName;

- (instancetype)initWithTestName:(NSString *)testName window:(id)window completionHandler:(void (^)(void))completionHandler
{
	if ((self = [super init])) {
		_testName = [testName copy];
		_window = [window retain];
		_completionHandler = [completionHandler copy];
	}
	return self;
}

- (void)dealloc
{
	[_testName release];
	[_window release];
	[_completionHandler release];
	[super dealloc];
}

@end

@implementation RPTScrollViewTestParameters {
	void (^_completionHandler)(void);
}

@synthesize testName = _testName;

- (instancetype)initWithTestName:(NSString *)testName scrollView:(id)scrollView completionHandler:(void (^)(void))completionHandler
{
	if ((self = [super init])) {
		_testName = [testName copy];
		_scrollView = [scrollView retain];
		_completionHandler = [completionHandler copy];
	}
	return self;
}

- (void)dealloc
{
	[_testName release];
	[_scrollView release];
	[_completionHandler release];
	[super dealloc];
}

@end
