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
#import <CoreGraphics/CoreGraphics.h>
#include <stdio.h>

@implementation RPTTestRunner

+ (BOOL)runTestWithParameters:(id<RPTTestParameters>)parameters
{
	fprintf(stderr, "RecapPerformanceTesting: performance test '%s' is not supported\n",
		[[parameters testName] UTF8String]);
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

/* ---- Interaction tests -------------------------------------------------------
 *
 * The classes below describe an interaction the harness would synthesise: a
 * drag, a swipe, a block interaction. On a system with no input synthesis there
 * is nothing to drive them with, so each one is constructed, records what it was
 * asked for, and reports that it did not run. That is the same answer
 * RPTTestRunner already gives for a test it cannot run, and it is the state a
 * caller is required to handle: the harness reports a test as not run, and the
 * app carries on.
 *
 * Group scroll and paging take a scroll view; directional swipe takes a direction
 * and a distance, because that is what distinguishes it from the others.
 */

@implementation RPTInteractionTestParameters {
	void (^_completionHandler)(void);
}
@synthesize testName = _testName;

- (instancetype)initWithTestName:(NSString *)testName
{
	if ((self = [super init]))
	{
		_testName = [testName copy];
	}
	return self;
}

- (instancetype)initWithTestName:(NSString *)testName completionHandler:(void (^)(void))completionHandler
{
	if ((self = [super init]))
	{
		_testName = [testName copy];
		_completionHandler = [completionHandler copy];
	}
	return self;
}

- (void)dealloc
{
	[_testName release];
	[_completionHandler release];
	[super dealloc];
}

- (BOOL)run
{
	fprintf(stderr, "RecapPerformanceTesting: interaction test '%s' is not supported\n",
		[_testName UTF8String]);
	return NO;
}

@end

@implementation RPTGroupScrollTestParameters
@synthesize scrollView = _scrollView;

- (instancetype)initWithTestName:(NSString *)testName scrollView:(id)scrollView completionHandler:(void (^)(void))completionHandler
{
	if ((self = [super initWithTestName:testName completionHandler:completionHandler]))
	{
		_scrollView = [scrollView retain];
	}
	return self;
}

- (void)dealloc
{
	[_scrollView release];
	[super dealloc];
}

@end

@implementation RPTPagingScrollViewTestParameters
@synthesize scrollView = _scrollView;

- (instancetype)initWithTestName:(NSString *)testName scrollView:(id)scrollView completionHandler:(void (^)(void))completionHandler
{
	if ((self = [super initWithTestName:testName completionHandler:completionHandler]))
	{
		_scrollView = [scrollView retain];
	}
	return self;
}

- (void)dealloc
{
	[_scrollView release];
	[super dealloc];
}

@end

@implementation RPTDirectionalSwipeTestParameters
@synthesize direction = _direction;
@synthesize distance = _distance;

- (instancetype)initWithTestName:(NSString *)testName direction:(NSUInteger)direction distance:(double)distance completionHandler:(void (^)(void))completionHandler
{
	if ((self = [super initWithTestName:testName completionHandler:completionHandler]))
	{
		_direction = direction;
		_distance = distance;
	}
	return self;
}

@end

@implementation RPTBlockInteraction
@end

@implementation RPTDragInteraction
@end

@implementation RPTActivationTestParameters
@end

@implementation RPTCoordinateSpaceConverter

/* Two spaces cannot be related without the window hierarchy they belong to, and
   there is no window here. The origin is the answer that means "no offset known",
   which is what a caller gets when the conversion is not possible. */
+ (CGPoint)convertPoint:(CGPoint)point fromSpace:(id)fromSpace toSpace:(id)toSpace
{
	(void)fromSpace;
	(void)toSpace;
	return point;
}

@end

/* ---- Measuring ---------------------------------------------------------------
 *
 * Prototypes read off the call sites rather than guessed, since this framework has
 * no SDK headers: both take one object plus, for the second, a direction in w1,
 * and both return a four-float rectangle in v0-v3.
 *
 * CGRectZero is the honest answer for both. There is no view laid out here, so
 * there are no bounds to report, and a caller that gets a zero rect is reading
 * "nothing to measure" rather than a fabricated position.
 */

CGRect RPTGetBoundsForView(id view)
{
	CGRect bounds;
	(void)view;
	/* Spelled out rather than CGRectZero, which is a CoreGraphics global this
	   framework cannot link against on its own. Same value. */
	bounds.origin.x = 0.0;
	bounds.origin.y = 0.0;
	bounds.size.width = 0.0;
	bounds.size.height = 0.0;
	return bounds;
}

CGRect RPTContentSizeInDirection(id view, NSUInteger direction)
{
	CGRect contentSize;
	(void)view;
	(void)direction;
	contentSize.origin.x = 0.0;
	contentSize.origin.y = 0.0;
	contentSize.size.width = 0.0;
	contentSize.size.height = 0.0;
	return contentSize;
}
