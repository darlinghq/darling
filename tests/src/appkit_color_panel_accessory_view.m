// Copyright (C) 2026 Lubos Dolezel
//
// NSColorPanel.accessoryView: a view that the program gives the colour panel is shown at the bottom of the
// panel. The panel grows by the height of the view; replacing the view makes it the size for the new one, and
// taking the view away gives the size back.
//
//   appkit_color_panel_accessory_view
//
// Needs a window system (X11 or Wayland).
#import <AppKit/AppKit.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static BOOL near(CGFloat a, CGFloat b)
{
	return fabs(a - b) < 0.5;
}

static void on_alarm(int sig)
{
	printf("FAIL: timed out\n");
	fflush(stdout);
	_exit(1);
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(60);
	@autoreleasepool
	{
		[NSApplication sharedApplication];
		NSColorPanel* panel = [NSColorPanel sharedColorPanel];
		NSView* content = [panel contentView];
		NSRect original = [panel frame];

		CHECK([panel accessoryView] == nil);

		NSView* first = [[[NSView alloc] initWithFrame: NSMakeRect(0, 0, 180, 30)] autorelease];
		[panel setAccessoryView: first];
		CHECK([panel accessoryView] == first);
		CHECK([first window] == panel);
		CHECK([first superview] == content);
		CHECK(near([panel frame].size.height, original.size.height + 30));
		CHECK(near([panel frame].size.width, original.size.width));
		// inside the panel's content
		CHECK(NSMaxX([first frame]) <= [content bounds].size.width + 0.5 && [first frame].origin.x >= -0.5);
		CHECK(NSMaxY([first frame]) <= [content bounds].size.height + 0.5);

		// replacing the view
		NSView* second = [[[NSView alloc] initWithFrame: NSMakeRect(0, 0, 100, 50)] autorelease];
		[panel setAccessoryView: second];
		CHECK([panel accessoryView] == second);
		CHECK([first superview] == nil);
		CHECK([second superview] == content);
		CHECK(near([panel frame].size.height, original.size.height + 50));

		// taking it away
		[panel setAccessoryView: nil];
		CHECK([panel accessoryView] == nil);
		CHECK([second superview] == nil);
		CHECK(near([panel frame].size.height, original.size.height));
	}
	if (g_failures)
		printf("FAIL appkit_color_panel_accessory_view: %d check(s)\n", g_failures);
	else
		printf("PASS appkit_color_panel_accessory_view\n");
	return g_failures != 0;
}
