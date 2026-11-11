// Copyright (C) 2026 Lubos Dolezel
//
// The private RecapPerformanceTesting framework loads and its classes behave
// as a harness that cannot run tests: every test reports "not run", test
// parameter objects keep what they were created with, and the measuring
// functions report an empty rectangle.
#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <objc/runtime.h>
#include <dlfcn.h>
#include <stdio.h>

// The framework is private and has no SDK header, so the test declares the
// parts of the classes it uses.
@interface RPTTestRunner : NSObject
+ (BOOL)runTestWithParameters:(id)parameters;
- (BOOL)runTestWithParameters:(id)parameters;
@end

@interface RPTResizeTestParameters : NSObject
@property (nonatomic, copy) NSString *testName;
- (instancetype)initWithTestName:(NSString *)testName window:(id)window completionHandler:(void (^)(void))completionHandler;
@end

@interface RPTDirectionalSwipeTestParameters : NSObject
@property (nonatomic) NSUInteger direction;
@property (nonatomic) double distance;
- (instancetype)initWithTestName:(NSString *)testName direction:(NSUInteger)direction distance:(double)distance completionHandler:(void (^)(void))completionHandler;
- (BOOL)run;
@end

@interface RPTCoordinateSpaceConverter : NSObject
+ (CGPoint)convertPoint:(CGPoint)point fromSpace:(id)fromSpace toSpace:(id)toSpace;
@end

static int failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL: %s (line %d)\n", #cond, __LINE__); failures++; } } while (0)

typedef CGRect (*bounds_fn)(id);
typedef CGRect (*content_fn)(id, NSUInteger);

int main(void)
{
	@autoreleasepool {
		void* handle = dlopen("/System/Library/PrivateFrameworks/RecapPerformanceTesting.framework/RecapPerformanceTesting", RTLD_NOW);
		if (!handle) {
			printf("FAIL: dlopen: %s\n", dlerror());
			return 1;
		}

		Class runner = objc_getClass("RPTTestRunner");
		Class resize = objc_getClass("RPTResizeTestParameters");
		Class swipe = objc_getClass("RPTDirectionalSwipeTestParameters");
		Class converter = objc_getClass("RPTCoordinateSpaceConverter");
		CHECK(runner && resize && swipe && converter);
		if (!(runner && resize && swipe && converter)) {
			return 1;
		}

		RPTResizeTestParameters* params = [[resize alloc] initWithTestName:@"resize" window:nil completionHandler:nil];
		CHECK([[params testName] isEqualToString:@"resize"]);
		CHECK(![runner runTestWithParameters:params]);
		CHECK(![[[[runner alloc] init] autorelease] runTestWithParameters:params]);
		[params release];

		RPTDirectionalSwipeTestParameters* s = [[swipe alloc] initWithTestName:@"swipe" direction:2 distance:12.5 completionHandler:nil];
		CHECK([s direction] == 2);
		CHECK([s distance] == 12.5);
		CHECK(![s run]);
		[s release];

		CGPoint p = [converter convertPoint:CGPointMake(3, 4) fromSpace:nil toSpace:nil];
		CHECK(p.x == 3 && p.y == 4);

		bounds_fn bounds = (bounds_fn)dlsym(handle, "RPTGetBoundsForView");
		content_fn content = (content_fn)dlsym(handle, "RPTContentSizeInDirection");
		CHECK(bounds && content);
		if (bounds && content) {
			CGRect b = bounds(nil);
			CGRect c = content(nil, 1);
			CHECK(b.origin.x == 0 && b.origin.y == 0 && b.size.width == 0 && b.size.height == 0);
			CHECK(c.origin.x == 0 && c.origin.y == 0 && c.size.width == 0 && c.size.height == 0);
		}
	}

	if (failures) {
		return 1;
	}
	puts("OK");
	return 0;
}
