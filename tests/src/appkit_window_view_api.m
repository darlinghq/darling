// CFLAGS: -framework AppKit
// Copyright (C) 2026 Lubos Dolezel
//
// NSWindow and NSView API:
//   - tabbingMode and collectionBehavior belong to each window;
//   - the title bar, tabbing and toolbar properties are stored, subtitle and
//     tabbingIdentifier are never nil;
//   - movable, standardWindowButton:, sheetParent, the screen and backing
//     conversions;
//   - a full-size content view leaves no room for the in-window menu;
//   - NSWindowDidChangeOcclusionStateNotification and the delegate's
//     windowDidChangeOcclusionState: when the window is ordered in and out;
//   - performWindowDragWithEvent: returns at once for a window that is not
//     movable, and at the button release for one that is;
//   - the frame view sizes the window's content view, not a view added to
//     it later;
//   - a drag started with -dragImage:... runs in-process when the display has
//     no dragging manager, and reaches a registered destination;
//   - a subview resized by its superview sees the superview's new transform;
//   - sortSubviewsUsingFunction:context:, inputContext and
//     NSTextInputContext -handleEvent:, and the stored layer properties.
//
// Needs an X display with a window manager (a disposable Xvfb); one 300x200
// window and one small sheet are shown. Prints PASS or FAIL lines, then
// ALL PASSED or FAILED n, and exits by itself.
#import <Foundation/NSArray.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSDate.h>
#import <Foundation/NSGeometry.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSObjCRuntime.h>
#import <Foundation/NSRange.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSString.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

// The SDK's AppKit headers do not build with this cross compiler, so only
// what is used is declared.
@class NSWindow, NSPasteboard, NSImage, NSTextInputContext;

@interface NSEvent : NSObject
+ (NSEvent *) mouseEventWithType: (NSUInteger) type
                        location: (NSPoint) location
                   modifierFlags: (NSUInteger) flags
                       timestamp: (NSTimeInterval) timestamp
                    windowNumber: (NSInteger) windowNumber
                         context: (id) context
                     eventNumber: (NSInteger) eventNumber
                      clickCount: (NSInteger) clickCount
                        pressure: (float) pressure;
+ (NSEvent *) keyEventWithType: (NSUInteger) type
                      location: (NSPoint) location
                 modifierFlags: (NSUInteger) flags
                     timestamp: (NSTimeInterval) timestamp
                  windowNumber: (NSInteger) windowNumber
                       context: (id) context
                    characters: (NSString *) characters
   charactersIgnoringModifiers: (NSString *) unmodCharacters
                     isARepeat: (BOOL) repeat
                       keyCode: (unsigned short) code;
@end
@interface NSApplication : NSObject
+ (NSApplication *) sharedApplication;
- (void) postEvent: (NSEvent *) event atStart: (BOOL) atStart;
@end
@interface NSView : NSObject
- (instancetype) initWithFrame: (NSRect) frame;
- (NSRect) frame;
- (void) setFrameSize: (NSSize) size;
- (void) setAutoresizingMask: (NSUInteger) mask;
- (void) addSubview: (NSView *) view;
- (NSView *) superview;
- (NSArray *) subviews;
- (NSWindow *) window;
- (NSPoint) convertPoint: (NSPoint) point toView: (NSView *) view;
- (void) resizeWithOldSuperviewSize: (NSSize) size;
- (void) sortSubviewsUsingFunction: (NSComparisonResult (*)(id, id, void *)) f
                           context: (void *) context;
- (NSTextInputContext *) inputContext;
- (void) registerForDraggedTypes: (NSArray *) types;
- (void) dragImage: (NSImage *) image
                at: (NSPoint) location
            offset: (NSSize) offset
             event: (NSEvent *) event
        pasteboard: (NSPasteboard *) pasteboard
            source: (id) source
         slideBack: (BOOL) slideBack;
- (BOOL) wantsUpdateLayer;
- (BOOL) canDrawSubviewsIntoLayer;
- (void) setCanDrawSubviewsIntoLayer: (BOOL) value;
- (NSUInteger) allowedTouchTypes;
- (void) setAllowedTouchTypes: (NSUInteger) value;
- (NSRect) convertRectToBacking: (NSRect) rect;
@end
@interface NSButton : NSView
- (BOOL) isEnabled;
@end
@interface NSWindow : NSObject
+ (NSRect) frameRectForContentRect: (NSRect) rect styleMask: (NSUInteger) mask;
- (instancetype) initWithContentRect: (NSRect) rect
                           styleMask: (NSUInteger) mask
                             backing: (NSUInteger) backing
                               defer: (BOOL) defer;
- (NSView *) contentView;
- (NSRect) frame;
- (NSInteger) windowNumber;
- (void) setContentSize: (NSSize) size;
- (void) setDelegate: (id) delegate;
- (void) orderFront: (id) sender;
- (void) orderOut: (id) sender;
- (void) setReleasedWhenClosed: (BOOL) flag;
- (void) close;
- (NSUInteger) occlusionState;
- (NSInteger) tabbingMode;
- (void) setTabbingMode: (NSInteger) mode;
- (NSUInteger) collectionBehavior;
- (void) setCollectionBehavior: (NSUInteger) behavior;
- (NSString *) tabbingIdentifier;
- (void) setTabbingIdentifier: (NSString *) identifier;
- (NSString *) subtitle;
- (void) setSubtitle: (NSString *) subtitle;
- (NSInteger) titleVisibility;
- (void) setTitleVisibility: (NSInteger) value;
- (BOOL) titlebarAppearsTransparent;
- (void) setTitlebarAppearsTransparent: (BOOL) value;
- (NSInteger) titlebarSeparatorStyle;
- (void) setTitlebarSeparatorStyle: (NSInteger) value;
- (NSInteger) toolbarStyle;
- (void) setToolbarStyle: (NSInteger) value;
- (BOOL) isRestorable;
- (BOOL) isMovable;
- (void) setMovable: (BOOL) movable;
- (BOOL) isOnActiveSpace;
- (NSButton *) standardWindowButton: (NSUInteger) button;
- (NSWindow *) sheetParent;
- (NSWindow *) attachedSheet;
- (void) beginSheet: (NSWindow *) sheet completionHandler: (void (^)(NSInteger)) handler;
- (void) endSheet: (NSWindow *) sheet returnCode: (NSInteger) code;
- (NSRect) convertRectToScreen: (NSRect) rect;
- (NSPoint) convertPointFromScreen: (NSPoint) point;
- (void) performWindowDragWithEvent: (NSEvent *) event;
@end
@interface NSPasteboard : NSObject
+ (NSPasteboard *) pasteboardWithName: (NSString *) name;
- (NSInteger) declareTypes: (NSArray *) types owner: (id) owner;
- (BOOL) setString: (NSString *) string forType: (NSString *) type;
@end
extern NSString *const NSDragPboard;
@interface NSImage : NSObject
- (instancetype) initWithSize: (NSSize) size;
@end
@interface NSTextInputContext : NSObject
- (id) client;
- (BOOL) handleEvent: (NSEvent *) event;
@end
extern NSString *const NSWindowDidChangeOcclusionStateNotification;
extern NSApplication *NSApp;

enum {
	StyleTitled = 1, StyleClosable = 2, StyleMiniaturizable = 4,
	StyleResizable = 8, StyleFullSizeContentView = 0x8000,
	BackingBuffered = 2,
	ButtonClose = 0, ButtonMiniaturize = 1, ButtonZoom = 2, ButtonToolbar = 3,
	EventLeftMouseDown = 1, EventLeftMouseUp = 2, EventKeyDown = 10,
	ModifierCommand = 1 << 20,
	DragOperationCopy = 1,
	OcclusionVisible = 1 << 1,
};

static int failures;

static void check(BOOL ok, const char *what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

static NSWindow *newWindow(NSRect content, NSUInteger mask)
{
	NSWindow *w = [[NSWindow alloc] initWithContentRect: content
		styleMask: mask backing: BackingBuffered defer: NO];
	[w setReleasedWhenClosed: NO];
	return w;
}

static void spin(void)
{
	[[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.2]];
}

// --- window properties ---------------------------------------------------

static void testWindowProperties(void)
{
	NSUInteger titled = StyleTitled | StyleClosable | StyleResizable;
	NSWindow *a = newWindow(NSMakeRect(0, 0, 100, 100), titled);
	NSWindow *b = newWindow(NSMakeRect(0, 0, 100, 100), titled);

	[a setTabbingMode: 2];
	[a setCollectionBehavior: 0x10];
	check([a tabbingMode] == 2 && [b tabbingMode] == 0,
		"tabbingMode belongs to each window");
	check([a collectionBehavior] == 0x10 && [b collectionBehavior] == 0,
		"collectionBehavior belongs to each window");

	check([a subtitle] != nil && [[a subtitle] length] == 0,
		"subtitle starts empty, not nil");
	NSMutableString *sub = [NSMutableString stringWithString: @"one"];
	[a setSubtitle: sub];
	[sub appendString: @"two"];
	check([[a subtitle] isEqualToString: @"one"], "subtitle is copied");
	check([a tabbingIdentifier] != nil &&
		[[a tabbingIdentifier] isEqualToString: [b tabbingIdentifier]],
		"windows of one class share a default tabbing identifier");
	[a setTabbingIdentifier: @"group"];
	check([[a tabbingIdentifier] isEqualToString: @"group"],
		"tabbingIdentifier is stored");

	[a setTitleVisibility: 1];
	[a setTitlebarAppearsTransparent: YES];
	[a setTitlebarSeparatorStyle: 3];
	[a setToolbarStyle: 4];
	check([a titleVisibility] == 1 && [a titlebarAppearsTransparent] &&
		[a titlebarSeparatorStyle] == 3 && [a toolbarStyle] == 4 &&
		[b titleVisibility] == 0 && ![b titlebarAppearsTransparent],
		"title bar and toolbar properties are stored per window");
	check([a isRestorable], "windows are restorable by default");
	check([a isOnActiveSpace], "isOnActiveSpace");

	check([a isMovable], "windows are movable by default");
	[a setMovable: NO];
	check(![a isMovable], "setMovable: NO is kept");

	NSButton *close = [a standardWindowButton: ButtonClose];
	check(close != nil && close == [a standardWindowButton: ButtonClose],
		"standardWindowButton: returns one close button");
	check([close isEnabled], "the close button of a closable window is enabled");
	NSButton *mini = [a standardWindowButton: ButtonMiniaturize];
	check(mini != nil && ![mini isEnabled],
		"the miniaturize button of a window that cannot miniaturize is disabled");
	check([a standardWindowButton: ButtonZoom] != nil &&
		[a standardWindowButton: ButtonToolbar] == nil,
		"zoom button exists, toolbar button does not");
	NSWindow *borderless = newWindow(NSMakeRect(0, 0, 50, 50), 0);
	check([borderless standardWindowButton: ButtonClose] == nil,
		"an untitled window has no close button");
	check([a sheetParent] == nil, "a window that is not a sheet has no sheetParent");

	NSRect fullFrame = [NSWindow frameRectForContentRect: NSMakeRect(0, 0, 100, 100)
		styleMask: StyleTitled | StyleFullSizeContentView];
	NSRect titledFrame = [NSWindow frameRectForContentRect: NSMakeRect(0, 0, 100, 100)
		styleMask: StyleTitled];
	check(fullFrame.size.height < titledFrame.size.height,
		"a full-size content view leaves no room for the in-window menu");

	[a close]; [b close]; [borderless close];
	[a release]; [b release]; [borderless release];
}

// --- occlusion, sheets, conversions, window drag --------------------------

@interface OcclusionDelegate : NSObject {
@public
	int delegateCalls;
	int notifications;
}
@end
@implementation OcclusionDelegate
- (void) windowDidChangeOcclusionState: (NSNotification *) note
{
	delegateCalls++;
}
- (void) observed: (NSNotification *) note
{
	notifications++;
}
@end

static void testVisibleWindow(NSWindow *w)
{
	OcclusionDelegate *d = [OcclusionDelegate new];
	[w setDelegate: d];
	[[NSNotificationCenter defaultCenter] addObserver: d
		selector: @selector(observed:)
		name: NSWindowDidChangeOcclusionStateNotification object: w];

	[w orderFront: nil];
	spin();
	check(d->notifications == 1 && d->delegateCalls == 1,
		"ordering in posts one occlusion change, also to the delegate");
	check(([w occlusionState] & OcclusionVisible) != 0,
		"an ordered-in window is visible");
	[w orderOut: nil];
	check(d->notifications == 2 && ([w occlusionState] & OcclusionVisible) == 0,
		"ordering out posts another and clears the visible state");
	[w orderFront: nil];
	spin();

	[[NSNotificationCenter defaultCenter] removeObserver: d];
	[w setDelegate: nil];
	[d release];

	NSRect frame = [w frame];
	NSRect r = [w convertRectToScreen: NSMakeRect(5, 6, 7, 8)];
	check(r.origin.x == frame.origin.x + 5 && r.origin.y == frame.origin.y + 6 &&
		r.size.width == 7 && r.size.height == 8, "convertRectToScreen:");

	NSWindow *sheet = newWindow(NSMakeRect(0, 0, 120, 60), StyleTitled);
	__block NSInteger code = 0;
	[w beginSheet: sheet completionHandler: ^(NSInteger rc) { code = rc; }];
	check([sheet sheetParent] == w && [w attachedSheet] == sheet,
		"sheetParent is the window the sheet is attached to");
	[w endSheet: sheet returnCode: 1000];
	check(code == 1000 && [sheet sheetParent] == nil,
		"ending the sheet calls its handler and detaches it");
	[sheet orderOut: nil];
	[sheet close];
	[sheet release];
	spin();

	NSPoint inWindow = NSMakePoint(20, 20);
	NSEvent *down = [NSEvent mouseEventWithType: EventLeftMouseDown location: inWindow
		modifierFlags: 0 timestamp: 0 windowNumber: [w windowNumber] context: nil
		eventNumber: 0 clickCount: 1 pressure: 1];
	NSEvent *up = [NSEvent mouseEventWithType: EventLeftMouseUp location: inWindow
		modifierFlags: 0 timestamp: 0 windowNumber: [w windowNumber] context: nil
		eventNumber: 0 clickCount: 1 pressure: 0];
	// A window that is not movable must return without waiting for the
	// release; if it waited, the alarm ends the test.
	[w setMovable: NO];
	[w performWindowDragWithEvent: down];
	check(YES, "performWindowDragWithEvent: returns at once for a fixed window");
	[w setMovable: YES];
	NSRect before = [w frame];
	[NSApp postEvent: up atStart: NO];
	[w performWindowDragWithEvent: down];
	check(NSEqualRects(before, [w frame]),
		"performWindowDragWithEvent: ends at the release, the window stays put");
}

// --- frame view layout ----------------------------------------------------

static void testFrameViewLayout(void)
{
	NSWindow *w = newWindow(NSMakeRect(50, 50, 300, 200), StyleTitled | StyleResizable);
	NSView *content = [w contentView];
	NSView *frameView = [content superview];
	NSView *overlay = [[NSView alloc] initWithFrame: NSMakeRect(0, 0, 10, 10)];
	[overlay setAutoresizingMask: 0];
	[frameView addSubview: overlay];

	[w setContentSize: NSMakeSize(320, 220)];
	NSSize size = [content frame].size;
	check(size.width == 320 && size.height == 220,
		"the content view is resized with the window");
	check(NSEqualRects([overlay frame], NSMakeRect(0, 0, 10, 10)),
		"a view added to the frame view keeps its own frame");
	[overlay release];
	[w close];
	[w release];
}

// --- in-process drag ------------------------------------------------------

@interface DropView : NSView {
@public
	int entered, performed, concluded;
}
@end
@implementation DropView
- (NSUInteger) draggingEntered: (id) info
{
	entered++;
	return DragOperationCopy;
}
- (BOOL) performDragOperation: (id) info
{
	performed++;
	return YES;
}
- (void) concludeDragOperation: (id) info
{
	concluded++;
}
@end

@interface DragSource : NSObject {
@public
	int ended;
	NSUInteger operation;
}
@end
@implementation DragSource
- (NSUInteger) draggingSourceOperationMaskForLocal: (BOOL) local
{
	return DragOperationCopy;
}
- (void) draggedImage: (NSImage *) image endedAt: (NSPoint) point operation: (NSUInteger) op
{
	ended++;
	operation = op;
}
@end

static void testDrag(NSWindow *w)
{
	DropView *view = [[DropView alloc] initWithFrame: NSMakeRect(0, 0, 300, 200)];
	[[w contentView] addSubview: view];
	[view registerForDraggedTypes: [NSArray arrayWithObject: @"org.darlinghq.test.drag"]];

	NSPasteboard *pb = [NSPasteboard pasteboardWithName: NSDragPboard];
	[pb declareTypes: [NSArray arrayWithObject: @"org.darlinghq.test.drag"] owner: nil];
	[pb setString: @"payload" forType: @"org.darlinghq.test.drag"];

	NSPoint at = NSMakePoint(150, 100);
	NSEvent *down = [NSEvent mouseEventWithType: EventLeftMouseDown location: at
		modifierFlags: 0 timestamp: 0 windowNumber: [w windowNumber] context: nil
		eventNumber: 0 clickCount: 1 pressure: 1];
	NSEvent *up = [NSEvent mouseEventWithType: EventLeftMouseUp location: at
		modifierFlags: 0 timestamp: 0 windowNumber: [w windowNumber] context: nil
		eventNumber: 0 clickCount: 1 pressure: 0];
	DragSource *source = [DragSource new];
	NSImage *image = [[NSImage alloc] initWithSize: NSMakeSize(16, 16)];

	[NSApp postEvent: up atStart: NO];
	[view dragImage: image at: NSMakePoint(142, 92) offset: NSMakeSize(0, 0)
		event: down pasteboard: pb source: source slideBack: NO];

	check(source->ended == 1, "the drag ends, and the source is told once");
	check(view->entered >= 1, "the registered view under the pointer is entered");
	check(view->performed == 1 && view->concluded == 1,
		"the drop is prepared (by default), performed and concluded");
	check(source->operation == DragOperationCopy,
		"the source learns the operation the destination performed");

	[image release];
	[source release];
	[view release];
}

// --- views ----------------------------------------------------------------

@interface FlippedView : NSView
@end
@implementation FlippedView
- (BOOL) isFlipped
{
	return YES;
}
@end

@interface ResizeProbe : NSView {
@public
	NSPoint seen;
	int resized;
}
@end
@implementation ResizeProbe
- (void) resizeWithOldSuperviewSize: (NSSize) size
{
	resized++;
	seen = [self convertPoint: NSMakePoint(0, 0) toView: nil];
	[super resizeWithOldSuperviewSize: size];
}
@end

static NSComparisonResult byX(id a, id b, void *context)
{
	double xa = [a frame].origin.x, xb = [b frame].origin.x;
	return xa < xb ? NSOrderedAscending : xa > xb ? NSOrderedDescending : NSOrderedSame;
}

@protocol NSTextInputClient
@end

@interface TextClient : NSView <NSTextInputClient> {
@public
	NSMutableArray *log;
}
@end
@implementation TextClient
- (void) insertText: (id) text replacementRange: (NSRange) range
{
	[log addObject: [NSString stringWithFormat: @"insert %@ %s", text,
		range.location == NSNotFound ? "none" : "range"]];
}
- (void) doCommandBySelector: (SEL) selector
{
	[log addObject: NSStringFromSelector(selector)];
}
@end

static NSEvent *key(NSString *chars, NSUInteger flags)
{
	return [NSEvent keyEventWithType: EventKeyDown location: NSMakePoint(0, 0)
		modifierFlags: flags timestamp: 0 windowNumber: 0 context: nil
		characters: chars charactersIgnoringModifiers: chars isARepeat: NO keyCode: 0];
}

static void testViews(void)
{
	NSWindow *w = newWindow(NSMakeRect(0, 0, 300, 300), StyleTitled);

	FlippedView *parent = [[FlippedView alloc] initWithFrame: NSMakeRect(10, 10, 100, 100)];
	ResizeProbe *child = [[ResizeProbe alloc] initWithFrame: NSMakeRect(0, 10, 20, 20)];
	[child setAutoresizingMask: 0];
	[parent addSubview: child];
	[[w contentView] addSubview: parent];
	(void) [child convertPoint: NSMakePoint(0, 0) toView: nil];
	[parent setFrameSize: NSMakeSize(100, 150)];
	NSPoint after = [child convertPoint: NSMakePoint(0, 0) toView: nil];
	check(child->resized == 1 && NSEqualPoints(child->seen, after),
		"a resized subview sees its superview's new transform");
	[child release];
	[parent release];

	NSView *box = [[NSView alloc] initWithFrame: NSMakeRect(0, 0, 100, 10)];
	double xs[3] = { 30, 10, 20 };
	for (int i = 0; i < 3; i++) {
		NSView *v = [[NSView alloc] initWithFrame: NSMakeRect(xs[i], 0, 5, 5)];
		[box addSubview: v];
		[v release];
	}
	[box sortSubviewsUsingFunction: byX context: NULL];
	NSArray *sorted = [box subviews];
	check([sorted count] == 3 && [[sorted objectAtIndex: 0] frame].origin.x == 10 &&
		[[sorted objectAtIndex: 1] frame].origin.x == 20 &&
		[[sorted objectAtIndex: 2] frame].origin.x == 30,
		"sortSubviewsUsingFunction:context: sorts the subviews");

	check([box inputContext] == nil, "a view that is not a text input client has no input context");
	TextClient *text = [[TextClient alloc] initWithFrame: NSMakeRect(0, 0, 10, 10)];
	text->log = [NSMutableArray array];
	NSTextInputContext *context = [text inputContext];
	check(context != nil && [text inputContext] == context && [context client] == text,
		"a text input client has one input context");
	BOOL handledA = [context handleEvent: key(@"a", 0)];
	BOOL handledReturn = [context handleEvent: key(@"\r", 0)];
	BOOL handledCommand = [context handleEvent: key(@"q", ModifierCommand)];
	check(handledA && handledReturn && !handledCommand && [text->log count] == 2 &&
		[[text->log objectAtIndex: 0] isEqualToString: @"insert a none"] &&
		[[text->log objectAtIndex: 1] isEqualToString: @"insertNewline:"],
		"handleEvent: inserts text and sends commands, and leaves key equivalents");
	[text release];

	check(![box wantsUpdateLayer], "views draw with drawRect: by default");
	[box setCanDrawSubviewsIntoLayer: YES];
	[box setAllowedTouchTypes: 3];
	check([box canDrawSubviewsIntoLayer] && [box allowedTouchTypes] == 3,
		"canDrawSubviewsIntoLayer and allowedTouchTypes are stored");
	check(NSEqualRects([box convertRectToBacking: NSMakeRect(1, 2, 3, 4)],
		NSMakeRect(1, 2, 3, 4)), "backing coordinates are points at scale 1");
	[box release];

	[w close];
	[w release];
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

	testWindowProperties();
	testFrameViewLayout();
	testViews();

	NSWindow *w = newWindow(NSMakeRect(200, 200, 300, 200), StyleTitled);
	testVisibleWindow(w);
	testDrag(w);
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
