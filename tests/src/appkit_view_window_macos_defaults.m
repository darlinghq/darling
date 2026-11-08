// CFLAGS: -framework AppKit
// Copyright (C) 2026 Lubos Dolezel
//
// NSView and NSWindow behavior as observed on macOS 27.0.1 (arm64,
// black-box, through the public API):
//   - QUIRK: -[NSView setFrame:] sends -setFrameOrigin: and then
//     -setFrameSize: with the new values, also when the frame does not
//     change, and posts NSViewFrameDidChangeNotification once, only when the
//     frame changed;
//   - QUIRK: mouseDownCanMoveWindow is YES for a non-opaque NSView and a
//     label, NO for an opaque view, NSButton, NSImageView and an editable
//     text field;
//   - defaults: standardWindowButton: none for a titled window without closable,
//     miniaturizable or resizable, otherwise all three, enabled per style,
//     restorable for titled windows only, allowedTouchTypes direct, an empty
//     subtitle and a tabbing identifier shared by a class;
//   - QUIRK: the initializer moves a window created off screen onto the
//     screen without sending -constrainFrameRect:toScreen:, and ordering
//     the window in does not move it again.
//
// Needs an X display with a window manager (a disposable Xvfb); one 400x300
// window is shown for a moment. Prints PASS or FAIL lines, then ALL PASSED
// or FAILED n, and exits by itself.
#import <Foundation/NSArray.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSDate.h>
#import <Foundation/NSGeometry.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSObjCRuntime.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSString.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

// The SDK's AppKit headers do not build with this cross compiler, so only
// what is used is declared.
@interface NSApplication : NSObject
+ (NSApplication *) sharedApplication;
@end
@interface NSScreen : NSObject
+ (NSScreen *) mainScreen;
- (NSRect) visibleFrame;
@end
@interface NSView : NSObject
- (instancetype) initWithFrame: (NSRect) frame;
- (NSRect) frame;
- (void) setFrame: (NSRect) frame;
- (void) setFrameSize: (NSSize) size;
- (void) setFrameOrigin: (NSPoint) origin;
- (void) setPostsFrameChangedNotifications: (BOOL) flag;
- (BOOL) mouseDownCanMoveWindow;
- (BOOL) isOpaque;
- (NSUInteger) allowedTouchTypes;
@end
@interface NSButton : NSView
- (BOOL) isEnabled;
@end
@interface NSTextField : NSView
- (void) setBezeled: (BOOL) flag;
- (void) setDrawsBackground: (BOOL) flag;
- (void) setEditable: (BOOL) flag;
- (void) setSelectable: (BOOL) flag;
@end
@interface NSImageView : NSView
@end
@interface NSWindow : NSObject
- (instancetype) initWithContentRect: (NSRect) rect
                           styleMask: (NSUInteger) mask
                             backing: (NSUInteger) backing
                               defer: (BOOL) defer;
- (NSRect) constrainFrameRect: (NSRect) rect toScreen: (NSScreen *) screen;
- (NSView *) contentView;
- (NSRect) frame;
- (void) orderFront: (id) sender;
- (void) orderOut: (id) sender;
- (void) setReleasedWhenClosed: (BOOL) flag;
- (void) close;
- (BOOL) isRestorable;
- (BOOL) isMovable;
- (NSString *) subtitle;
- (NSString *) tabbingIdentifier;
- (NSInteger) titleVisibility;
- (NSInteger) titlebarSeparatorStyle;
- (NSInteger) toolbarStyle;
- (NSButton *) standardWindowButton: (NSUInteger) button;
@end
extern NSString *const NSViewFrameDidChangeNotification;

enum {
	StyleTitled = 1, StyleClosable = 2, StyleMiniaturizable = 4,
	StyleResizable = 8, BackingBuffered = 2,
	ButtonClose = 0, ButtonMiniaturize = 1, ButtonZoom = 2, ButtonToolbar = 3,
	TouchTypeMaskDirect = 1 << 0,
};

static int failures;

static void check(BOOL ok, const char *what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

static NSWindow *newWindow(NSRect content, NSUInteger mask, BOOL deferFlag)
{
	NSWindow *w = [[NSWindow alloc] initWithContentRect: content
		styleMask: mask backing: BackingBuffered defer: deferFlag];
	[w setReleasedWhenClosed: NO];
	return w;
}

// --- setFrame: -----------------------------------------------------------

static NSMutableArray *setterLog;

@interface SetterProbe : NSView {
@public
	int notifications;
}
@end
@implementation SetterProbe
- (void) setFrameOrigin: (NSPoint) p
{
	[setterLog addObject: [NSString stringWithFormat: @"origin %g,%g", p.x, p.y]];
	[super setFrameOrigin: p];
}
- (void) setFrameSize: (NSSize) s
{
	[setterLog addObject: [NSString stringWithFormat: @"size %gx%g", s.width, s.height]];
	[super setFrameSize: s];
}
- (void) frameChanged: (NSNotification *) note
{
	notifications++;
}
@end

static void testSetFrame(void)
{
	setterLog = [NSMutableArray array];
	SetterProbe *v = [[SetterProbe alloc] initWithFrame: NSMakeRect(0, 0, 10, 10)];
	[v setPostsFrameChangedNotifications: YES];
	[[NSNotificationCenter defaultCenter] addObserver: v
		selector: @selector(frameChanged:)
		name: NSViewFrameDidChangeNotification object: v];

	struct {
		NSRect frame;
		int notes;
		const char *what;
	} cases[] = {
		{ { { 5, 5 }, { 20, 20 } }, 1, "origin and size changed" },
		{ { { 5, 5 }, { 30, 30 } }, 1, "size changed" },
		{ { { 7, 7 }, { 30, 30 } }, 1, "origin changed" },
		{ { { 7, 7 }, { 30, 30 } }, 0, "nothing changed" },
	};
	for (int i = 0; i < 4; i++) {
		[setterLog removeAllObjects];
		v->notifications = 0;
		[v setFrame: cases[i].frame];
		NSString *expected = [NSString stringWithFormat: @"origin %g,%g, size %gx%g",
			cases[i].frame.origin.x, cases[i].frame.origin.y,
			cases[i].frame.size.width, cases[i].frame.size.height];
		NSString *got = [setterLog componentsJoinedByString: @", "];
		char what[160];
		snprintf(what, sizeof(what),
			"setFrame: (%s) sends -setFrameOrigin: then -setFrameSize: [%s]",
			cases[i].what, [got UTF8String]);
		check([got isEqualToString: expected], what);
		snprintf(what, sizeof(what),
			"setFrame: (%s) posts %d frame change notification(s), got %d",
			cases[i].what, cases[i].notes, v->notifications);
		check(v->notifications == cases[i].notes, what);
		check(NSEqualRects([v frame], cases[i].frame), "the frame is set");
	}

	v->notifications = 0;
	[v setFrameSize: NSMakeSize(40, 40)];
	check(v->notifications == 1, "setFrameSize: alone posts one notification");

	[[NSNotificationCenter defaultCenter] removeObserver: v];
	[v release];
}

// --- mouseDownCanMoveWindow -------------------------------------------------

@interface OpaqueView : NSView
@end
@implementation OpaqueView
- (BOOL) isOpaque
{
	return YES;
}
@end

static void testMouseDownCanMoveWindow(NSWindow *w)
{
	NSRect r = NSMakeRect(0, 0, 50, 20);
	NSView *plain = [[NSView alloc] initWithFrame: r];
	OpaqueView *opaque = [[OpaqueView alloc] initWithFrame: r];
	NSButton *button = [[NSButton alloc] initWithFrame: r];
	NSImageView *image = [[NSImageView alloc] initWithFrame: r];
	NSTextField *label = [[NSTextField alloc] initWithFrame: r];
	[label setBezeled: NO];
	[label setDrawsBackground: NO];
	[label setEditable: NO];
	[label setSelectable: NO];
	NSTextField *field = [[NSTextField alloc] initWithFrame: r];
	[field setBezeled: YES];
	[field setEditable: YES];

	check([plain mouseDownCanMoveWindow], "a plain view lets the window move");
	check(![opaque mouseDownCanMoveWindow], "an opaque view does not");
	check(![button mouseDownCanMoveWindow], "a button does not");
	check(![image mouseDownCanMoveWindow], "an image view does not");
	check([label mouseDownCanMoveWindow], "a label does");
	check(![field mouseDownCanMoveWindow], "an editable text field does not");
	check([[w contentView] mouseDownCanMoveWindow], "a window's content view does");

	[plain release]; [opaque release]; [button release];
	[image release]; [label release]; [field release];
}

// --- defaults ---------------------------------------------------------------

static void testDefaults(void)
{
	NSRect r = NSMakeRect(0, 0, 100, 100);
	NSUInteger full = StyleTitled | StyleClosable | StyleMiniaturizable | StyleResizable;
	NSWindow *a = newWindow(r, full, YES);
	NSWindow *b = newWindow(r, full, YES);
	NSWindow *titled = newWindow(r, StyleTitled, YES);
	NSWindow *borderless = newWindow(r, 0, YES);

	check([a isRestorable] && [titled isRestorable] && ![borderless isRestorable],
		"titled windows are restorable by default, borderless ones are not");
	check([a isMovable] && [borderless isMovable], "windows are movable by default");
	check([a subtitle] != nil && [[a subtitle] length] == 0, "subtitle is empty, not nil");
	check([a tabbingIdentifier] != nil &&
		[[a tabbingIdentifier] isEqualToString: [b tabbingIdentifier]],
		"windows of one class share a tabbing identifier");
	check([a titleVisibility] == 0 && [a titlebarSeparatorStyle] == 0 &&
		[a toolbarStyle] == 0, "title bar styles start at 0");

	BOOL allThere = YES;
	for (NSUInteger i = ButtonClose; i <= ButtonZoom; i++) {
		NSButton *button = [a standardWindowButton: i];
		if (button == nil || ![button isEnabled])
			allThere = NO;
	}
	check(allThere, "a closable, miniaturizable, resizable window has enabled buttons");
	check([a standardWindowButton: ButtonToolbar] == nil, "there is no toolbar button");
	BOOL noneTitled = YES, noneBorderless = YES;
	for (NSUInteger i = ButtonClose; i <= ButtonZoom; i++) {
		if ([titled standardWindowButton: i] != nil)
			noneTitled = NO;
		if ([borderless standardWindowButton: i] != nil)
			noneBorderless = NO;
	}
	check(noneTitled, "a window that is only titled has no window buttons");
	check(noneBorderless, "a borderless window has no window buttons");

	// With any one of closable, miniaturizable and resizable, a titled window has all
	// three buttons, each enabled only if its style asks for it (window buttons).
	const NSUInteger single[] = { StyleClosable, StyleMiniaturizable, StyleResizable };
	const char* names[] = { "closable", "miniaturizable", "resizable" };
	for (int s = 0; s < 3; s++) {
		NSWindow *w = newWindow(r, StyleTitled | single[s], YES);
		BOOL ok = YES;
		for (NSUInteger i = ButtonClose; i <= ButtonZoom; i++) {
			NSButton *button = [w standardWindowButton: i];
			if (button == nil || [button isEnabled] != (i == (NSUInteger)s))
				ok = NO;
		}
		char message[128];
		snprintf(message, sizeof(message),
			"a titled, %s window has all three buttons, only that one enabled", names[s]);
		check(ok, message);
		[w close];
		[w release];
	}

	NSView *view = [[NSView alloc] initWithFrame: r];
	check([view allowedTouchTypes] == TouchTypeMaskDirect,
		"views allow direct touches by default");
	[view release];

	[a close]; [b close]; [titled close]; [borderless close];
	[a release]; [b release]; [titled release]; [borderless release];
}

// --- init and the screen ----------------------------------------------------

static int constrainCalls;
static BOOL recording;

@interface ConstrainProbe : NSWindow
@end
@implementation ConstrainProbe
- (NSRect) constrainFrameRect: (NSRect) rect toScreen: (NSScreen *) screen
{
	if (recording)
		constrainCalls++;
	return [super constrainFrameRect: rect toScreen: screen];
}
@end

static BOOL insideRect(NSRect inner, NSRect outer)
{
	return NSMinX(inner) >= NSMinX(outer) && NSMinY(inner) >= NSMinY(outer) &&
		NSMaxX(inner) <= NSMaxX(outer) && NSMaxY(inner) <= NSMaxY(outer);
}

static NSWindow *testInitOnScreen(void)
{
	recording = YES;
	NSWindow *w = [[ConstrainProbe alloc]
		initWithContentRect: NSMakeRect(-500, 5000, 400, 300)
		styleMask: StyleTitled backing: BackingBuffered defer: NO];
	recording = NO;
	[w setReleasedWhenClosed: NO];

	NSRect afterInit = [w frame];
	check(constrainCalls == 0, "init does not send -constrainFrameRect:toScreen:");
	check(insideRect(afterInit, [[NSScreen mainScreen] visibleFrame]),
		"init moves an off-screen window onto the screen");
	check(afterInit.size.width == 400 && afterInit.size.height >= 300,
		"init keeps the window's size");
	// Read the frame before running the event loop: on X11 a window manager may then place
	// a titled window wherever it likes (ICCCM 4.1.2.3), which is not AppKit moving it.
	[w orderFront: nil];
	check(NSEqualRects(afterInit, [w frame]), "ordering the window in does not move it");
	[[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.3]];
	return w;
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
	alarm(60);

	NSAutoreleasePool *pool = [NSAutoreleasePool new];
	[NSApplication sharedApplication];

	testSetFrame();
	testDefaults();
	NSWindow *w = testInitOnScreen();
	testMouseDownCanMoveWindow(w);
	[w orderOut: nil];
	[w close];
	[w release];
	[pool release];

	if (failures == 0)
		printf("ALL PASSED\n");
	else
		printf("FAILED %d\n", failures);
	fflush(stdout);
	_exit(failures == 0 ? 0 : 1);
}
