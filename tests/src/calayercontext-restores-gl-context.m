/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * A layer-backed view's display must leave the application's OpenGL context
 * current: the context current before -displayIfNeeded is current after it,
 * and when none was current, none is. Also when a layer's drawing raises.
 * Needs a display (Xvfb :99).
 */
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#import <OpenGL/OpenGL.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

static int failed;

static void expect(int cond, const char* what)
{
	if (!cond)
	{
		printf("FAIL: %s\n", what);
		failed = 1;
	}
}

static void watchdog(int sig)
{
	static const char msg[] = "FAIL: timed out\n";
	write(1, msg, sizeof(msg) - 1);
	_exit(1);
}

@interface RaisingLayer : CALayer
@end

@implementation RaisingLayer
- (void) drawInContext: (CGContextRef) ctx
{
	[NSException raise: @"RaisingLayer" format: @"drawing raised"];
}
@end

@interface PlainView : NSView
@end

@implementation PlainView
- (void) drawRect: (NSRect) rect
{
	[[NSColor redColor] set];
	NSRectFill(rect);
}
@end

int main(void)
{
	NSAutoreleasePool* pool = [NSAutoreleasePool new];

	signal(SIGALRM, watchdog);
	alarm(60);

	[NSApplication sharedApplication];

	NSWindow* window = [[NSWindow alloc] initWithContentRect: NSMakeRect(0, 0, 200, 200)
		styleMask: NSTitledWindowMask backing: NSBackingStoreBuffered defer: NO];
	PlainView* view = [[PlainView alloc] initWithFrame: NSMakeRect(0, 0, 200, 200)];

	[window setContentView: view];
	[view setWantsLayer: YES];
	[window makeKeyAndOrderFront: nil];

	NSOpenGLPixelFormatAttribute attrs[] = { NSOpenGLPFADoubleBuffer, 0 };
	NSOpenGLPixelFormat* format = [[NSOpenGLPixelFormat alloc] initWithAttributes: attrs];
	NSOpenGLContext* appContext = [[NSOpenGLContext alloc] initWithFormat: format shareContext: nil];

	if (appContext == nil)
	{
		printf("SKIP: no OpenGL context\n");
		return 0;
	}

	// The application's context draws into a view of its own, which is not
	// layer-backed, and is current while the layer-backed view displays
	NSView* glView = [[NSView alloc] initWithFrame: NSMakeRect(0, 0, 20, 20)];
	[view addSubview: glView];
	[appContext setView: glView];
	[appContext makeCurrentContext];
	CGLContextObj app = CGLGetCurrentContext();
	expect(app != NULL, "the application context is current");

	[view setNeedsDisplay: YES];
	[view displayIfNeeded];
	expect(CGLGetCurrentContext() == app, "application context current after display");

	// With a raising sublayer
	RaisingLayer* bad = [RaisingLayer layer];
	bad.frame = CGRectMake(0, 0, 50, 50);
	[[view layer] addSublayer: bad];
	@try
	{
		[view setNeedsDisplay: YES];
		[view displayIfNeeded];
	}
	@catch (NSException* e)
	{
	}
	expect(CGLGetCurrentContext() == app, "application context current after a raising display");
	[bad removeFromSuperlayer];

	// And when nothing was current
	[NSOpenGLContext clearCurrentContext];
	expect(CGLGetCurrentContext() == NULL, "no context current");
	[view setNeedsDisplay: YES];
	[view displayIfNeeded];
	expect(CGLGetCurrentContext() == NULL, "still no context current after display");

	printf("%s\n", failed ? "FAIL" : "PASS");
	[pool release];
	return failed;
}
