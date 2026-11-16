// Copyright (C) 2026 Lubos Dolezel
//
// The characters of key events for the active keymap: a window takes the first N key-down events
// and compares their -characters with the expected strings (UTF-8, as arguments), for typed keys
// injected by the test driver (xdotool on X11, wtype or a virtual keyboard on Wayland).
// usage: keyboard_layout_events <x11|wayland> <expected characters>...
// Prints READY once the window is key, then PASS or FAIL lines; SKIP for the wrong backend.
#import <AppKit/AppKit.h>
#include <stdio.h>
#include <string.h>

@interface KeyView : NSView {
@public
	NSMutableArray* received;
}
@end

@implementation KeyView
- (BOOL) acceptsFirstResponder { return YES; }
- (void) keyDown: (NSEvent*) event
{
	[received addObject: [event characters] ?: @""];
}
@end

int main(int argc, char** argv)
{
	@autoreleasepool {
		if (argc < 3) {
			printf("usage: keyboard_layout_events <x11|wayland> <expected>...\n");
			return 2;
		}
		[NSApplication sharedApplication];
		NSString* displayClass = NSStringFromClass([[NSClassFromString(@"NSDisplay") performSelector: @selector(currentDisplay)] class]);
		const char* want = !strcmp(argv[1], "wayland") ? "Wayland" : "X11";
		if (![displayClass hasPrefix: [NSString stringWithUTF8String: want]]) {
			printf("SKIP: the display backend is %s\n", [displayClass UTF8String]);
			return 0;
		}
		[NSApp setActivationPolicy: NSApplicationActivationPolicyRegular];
		NSWindow* window = [[NSWindow alloc] initWithContentRect: NSMakeRect(100, 100, 300, 200)
				styleMask: NSTitledWindowMask backing: NSBackingStoreBuffered defer: NO];
		KeyView* view = [[KeyView alloc] initWithFrame: NSMakeRect(0, 0, 300, 200)];
		view->received = [NSMutableArray array];
		[window setContentView: view];
		[window makeKeyAndOrderFront: nil];
		[window makeFirstResponder: view];
		[NSApp activateIgnoringOtherApps: YES];

		const int expected = argc - 2;
		NSDate* ready = [NSDate dateWithTimeIntervalSinceNow: 2];
		while ([ready timeIntervalSinceNow] > 0)
			[[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.1]];
		printf("READY\n");
		fflush(stdout);

		NSDate* deadline = [NSDate dateWithTimeIntervalSinceNow: 30];
		while ((int) [view->received count] < expected && [deadline timeIntervalSinceNow] > 0) {
			NSEvent* event = [NSApp nextEventMatchingMask: NSAnyEventMask untilDate: [NSDate dateWithTimeIntervalSinceNow: 0.2]
					inMode: NSDefaultRunLoopMode dequeue: YES];
			if (event)
				[NSApp sendEvent: event];
		}

		int failures = 0;
		if ((int) [view->received count] != expected) {
			printf("FAIL: got %d key events, expected %d\n", (int) [view->received count], expected);
			failures++;
		}
		for (int i = 0; i < expected && i < (int) [view->received count]; i++) {
			NSString* wantString = [NSString stringWithUTF8String: argv[2 + i]];
			if (![[view->received objectAtIndex: i] isEqualToString: wantString]) {
				printf("FAIL: event %d characters %s, expected %s\n", i, [[[view->received objectAtIndex: i] description] UTF8String], argv[2 + i]);
				failures++;
			}
		}
		printf(failures ? "%d failure(s)\n" : "PASS\n", failures);
		return failures != 0;
	}
}
