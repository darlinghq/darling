// Copyright (C) 2026 Lubos Dolezel
//
// QTMovieView is an NSView subclass, and on the legacy (fragile) Objective-C
// ABI (i386) an application's subclass of it is compiled against the
// framework's instance size: on macOS that is 0x74 (NSView's 0x50 plus the
// class's own state), and the subclass's ivars start there. This checks the
// superclass, that size (legacy ABI only; ivars slide on the non-fragile ABI)
// and that a subclass with ivars of its own keeps them intact while the
// view's NSView state is in use. Works for i386 and x86_64:
//   guest-cc.sh -a i386 -o qtkit-movieview-layout tests/qtkit-movieview-layout.m -framework Foundation -framework AppKit -framework QTKit
// Exits non-zero on failure.
#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#import <QTKit/QTKit.h>
#include <objc/runtime.h>
#include <stdio.h>

static int failures;

static void expect(BOOL condition, const char *what) {
	if (!condition) {
		printf("FAIL: %s\n", what);
		failures++;
	}
}

#define CANARY_WORDS 16
#define CANARY(i) (0xC0DE0000u + (unsigned) (i))

@interface MyMovieView : QTMovieView {
  @public
	unsigned _canary[CANARY_WORDS];
}
@end

@implementation MyMovieView
@end

int main(void) {
	@autoreleasepool {
		[NSApplication sharedApplication];

		expect(class_getSuperclass([QTMovieView class]) == [NSView class], "QTMovieView superclass is NSView");
#if !__OBJC2__
		expect(class_getInstanceSize([QTMovieView class]) == 0x74, "QTMovieView instance size is 0x74");
		Ivar canary = class_getInstanceVariable([MyMovieView class], "_canary");
		expect(canary != NULL && ivar_getOffset(canary) == 0x74, "subclass ivars start at 0x74");
#endif

		MyMovieView *view = [[MyMovieView alloc] initWithFrame:NSMakeRect(1, 2, 300, 200)];
		expect(view != nil, "QTMovieView subclass initWithFrame:");
		for (int i = 0; i < CANARY_WORDS; i++)
			view->_canary[i] = CANARY(i);

		NSView *child = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 10, 10)];
		[view addSubview:child];
		[view setHidden:YES];
		[view setToolTip:@"movie"];
		[view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
		NSWindow *window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 400, 300)
		                                               styleMask:NSWindowStyleMaskTitled
		                                                 backing:NSBackingStoreBuffered
		                                                   defer:YES];
		[[window contentView] addSubview:view];

		BOOL intact = YES;
		for (int i = 0; i < CANARY_WORDS; i++)
			intact = intact && view->_canary[i] == CANARY(i);
		expect(intact, "subclass ivars intact after using NSView API");
		expect(NSEqualRects([view frame], NSMakeRect(1, 2, 300, 200)), "frame");
		expect([view isHidden], "hidden");
		expect([[view toolTip] isEqual:@"movie"], "toolTip");
		expect([[view subviews] count] == 1 && [view superview] == [window contentView], "view hierarchy");

		[view removeFromSuperview];
		[child release];
		[window release];
		[view release];
	}
	if (failures == 0)
		printf("PASS\n");
	return failures != 0;
}
