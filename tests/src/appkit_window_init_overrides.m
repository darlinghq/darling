// CFLAGS: -framework AppKit
// Copyright (C) 2026 Lubos Dolezel
//
// QUIRK test for -[NSWindow initWithContentRect:styleMask:backing:defer:].
// A subclass records which of its overrides run before the initializer
// returns. On macOS 27.0.1 (arm64, black-box observation) the initializer
// sends +frameRectForContentRect:styleMask: once and -setContentView: once
// with a plain NSView, and none of -frameRectForContentRect:,
// -contentRectForFrameRect: and +contentRectForFrameRect:styleMask:. Once
// init has returned, the instance overrides are used as usual
// (-setContentSize: goes through -contentRectForFrameRect:).
//
// Needs an X display (a disposable Xvfb is enough); no window is ordered in.
// Prints PASS or FAIL lines, then ALL PASSED or FAILED n, and exits by
// itself.
#import <Foundation/NSArray.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSGeometry.h>
#import <Foundation/NSObjCRuntime.h>
#import <Foundation/NSString.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

// The SDK's AppKit headers do not build with this cross compiler, so only
// what is used is declared.
@interface NSApplication : NSObject
+ (NSApplication *) sharedApplication;
@end
@interface NSView : NSObject
- (NSRect) frame;
@end
@interface NSWindow : NSObject
+ (NSRect) frameRectForContentRect: (NSRect) rect styleMask: (NSUInteger) mask;
+ (NSRect) contentRectForFrameRect: (NSRect) rect styleMask: (NSUInteger) mask;
- (instancetype) initWithContentRect: (NSRect) rect
                           styleMask: (NSUInteger) mask
                             backing: (NSUInteger) backing
                               defer: (BOOL) defer;
- (NSRect) frameRectForContentRect: (NSRect) rect;
- (NSRect) contentRectForFrameRect: (NSRect) rect;
- (void) setContentView: (NSView *) view;
- (NSView *) contentView;
- (void) setContentSize: (NSSize) size;
- (NSRect) frame;
- (void) setReleasedWhenClosed: (BOOL) flag;
- (void) close;
@end

enum {
	StyleBorderless = 0,
	StyleTitled = 1,
	StyleClosable = 2,
	StyleMiniaturizable = 4,
	StyleResizable = 8,
	BackingBuffered = 2,
};

static BOOL recording;
static NSMutableArray *calls;
static int failures;

static void note(NSString *s)
{
	if (recording)
		[calls addObject: s];
}

static void check(BOOL ok, const char *what, unsigned long mask, BOOL deferFlag)
{
	printf("%s: %s (styleMask=0x%lx defer=%d)\n", ok ? "PASS" : "FAIL", what,
		mask, (int) deferFlag);
	if (!ok)
		failures++;
}

@interface ProbeWindow : NSWindow
@end

@implementation ProbeWindow
+ (NSRect) frameRectForContentRect: (NSRect) r styleMask: (NSUInteger) m
{
	note(@"+frameRectForContentRect:styleMask:");
	return [super frameRectForContentRect: r styleMask: m];
}
+ (NSRect) contentRectForFrameRect: (NSRect) r styleMask: (NSUInteger) m
{
	note(@"+contentRectForFrameRect:styleMask:");
	return [super contentRectForFrameRect: r styleMask: m];
}
- (NSRect) frameRectForContentRect: (NSRect) r
{
	note(@"-frameRectForContentRect:");
	return [super frameRectForContentRect: r];
}
- (NSRect) contentRectForFrameRect: (NSRect) r
{
	note(@"-contentRectForFrameRect:");
	return [super contentRectForFrameRect: r];
}
- (void) setContentView: (NSView *) v
{
	note([NSString stringWithFormat: @"-setContentView:%@",
		v ? NSStringFromClass([v class]) : @"nil"]);
	[super setContentView: v];
}
@end

static NSUInteger countOf(NSString *entry)
{
	NSUInteger n = 0;
	for (NSString *c in calls)
		if ([c isEqualToString: entry])
			n++;
	return n;
}

static void run(unsigned long mask, BOOL deferFlag)
{
	NSAutoreleasePool *pool = [NSAutoreleasePool new];
	NSRect content = NSMakeRect(100, 100, 400, 300);

	calls = [NSMutableArray array];
	recording = YES;
	ProbeWindow *w = [[ProbeWindow alloc] initWithContentRect: content
		styleMask: mask backing: BackingBuffered defer: deferFlag];
	recording = NO;

	printf("styleMask=0x%lx defer=%d: %lu call(s) during init\n", mask,
		(int) deferFlag, (unsigned long) [calls count]);
	for (NSString *c in calls)
		printf("  %s\n", [c UTF8String]);

	check(countOf(@"-frameRectForContentRect:") == 0,
		"init does not send -frameRectForContentRect:", mask, deferFlag);
	check(countOf(@"-contentRectForFrameRect:") == 0,
		"init does not send -contentRectForFrameRect:", mask, deferFlag);
	check(countOf(@"+contentRectForFrameRect:styleMask:") == 0,
		"init does not send +contentRectForFrameRect:styleMask:", mask, deferFlag);
	check(countOf(@"+frameRectForContentRect:styleMask:") == 1,
		"init sends +frameRectForContentRect:styleMask: once", mask, deferFlag);
	check(countOf(@"-setContentView:NSView") == 1 && countOf(@"-setContentView:nil") == 0,
		"init sends -setContentView: once with an NSView", mask, deferFlag);

	NSSize size = [[w contentView] frame].size;
	check(size.width == 400 && size.height == 300,
		"the content view has the content size", mask, deferFlag);

	calls = [NSMutableArray array];
	recording = YES;
	[w setContentSize: NSMakeSize(410, 310)];
	recording = NO;
	check(countOf(@"-contentRectForFrameRect:") >= 1,
		"after init, -setContentSize: uses the -contentRectForFrameRect: override",
		mask, deferFlag);

	[w setReleasedWhenClosed: NO];
	[w close];
	[w release];
	[pool release];
}

static void timeout(int sig)
{
	(void) sig;
	printf("TIMEOUT\n");
	fflush(stdout);
	_exit(3);
}

int main(void)
{
	signal(SIGALRM, timeout);
	alarm(30);

	NSAutoreleasePool *pool = [NSAutoreleasePool new];
	[NSApplication sharedApplication];

	unsigned long titled = StyleTitled | StyleClosable | StyleMiniaturizable |
		StyleResizable;
	run(titled, YES);
	run(titled, NO);
	run(StyleBorderless, YES);
	[pool release];

	if (failures == 0)
		printf("ALL PASSED\n");
	else
		printf("FAILED %d\n", failures);
	fflush(stdout);
	_exit(failures == 0 ? 0 : 1);
}
