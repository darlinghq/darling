// Copyright (C) 2026 Lubos Dolezel
//
// The open and save panels have a sidebar of places (Home, Desktop, ...) left of the file list. It has a light
// source-list background. This program shows an NSOpenPanel (not modal, from an ordinary Cocoa application) or an
// NSSavePanel, prints where the sidebar is on the screen, and lets the harness (open_panel_sidebar.py) take a
// screenshot of the window system and look at the pixels there; the sidebar must not be black.
//
//   appkit_open_panel_sidebar [save]
//
// Lines the harness reads:
//   SIDEBAR x y w h      the sidebar in screen pixels, origin top left, as the window system shows it
//   PANEL_READY          the panel is shown and drawn
//   PASS appkit_open_panel_sidebar / FAIL ...
#import <Cocoa/Cocoa.h>
#include <stdio.h>
#include <unistd.h>


// the share of the view's pixels that are dark, as the view draws itself into a bitmap
static double darkShare(NSView *view)
{
	NSBitmapImageRep *rep = [view bitmapImageRepForCachingDisplayInRect:[view bounds]];
	int dark = 0, total = 0;

	[view cacheDisplayInRect:[view bounds] toBitmapImageRep:rep];
	for (NSInteger y = 0; y < [rep pixelsHigh]; y++)
		for (NSInteger x = 0; x < [rep pixelsWide]; x++) {
			NSColor *c = [[rep colorAtX:x y:y] colorUsingColorSpaceName:NSCalibratedRGBColorSpace];
			total++;
			if ([c redComponent] + [c greenComponent] + [c blueComponent] < 0.6)
				dark++;
		}
	return total ? (double)dark / total : 1.0;
}

static NSScrollView *findSidebar(NSView *view)
{
	for (NSView *sub in [view subviews]) {
		if ([sub isKindOfClass:[NSScrollView class]] && [[(NSScrollView *)sub documentView] isKindOfClass:[NSTableView class]]
		    && NSMinX([sub frame]) < 1)
			return (NSScrollView *)sub;
		NSScrollView *found = findSidebar(sub);
		if (found)
			return found;
	}
	return nil;
}

@interface Probe : NSObject {
	@public NSSavePanel *_panel;
}
- (void)report:(id)unused;
- (void)finish:(id)unused;
@end

@implementation Probe
- (void)report:(id)unused
{
	NSView *content = [_panel contentView];
	NSScrollView *sidebar = findSidebar(content);
	if (sidebar == nil) {
		printf("FAIL: no sidebar found in the panel\n");
		fflush(stdout);
		_exit(1);
	}
	NSRect inWindow = [sidebar convertRect:[sidebar bounds] toView:nil];
	NSRect onScreen = [_panel convertRectToScreen:inWindow];
	NSScreen *scr = [_panel screen] ?: [NSScreen mainScreen];
	NSRect screen = [scr frame];
	printf("SIDEBAR %d %d %d %d\n", (int)onScreen.origin.x, (int)(screen.size.height - NSMaxY(onScreen)),
	       (int)onScreen.size.width, (int)onScreen.size.height);
	{
		double share = darkShare(sidebar);
		printf("sidebar drawn into a bitmap: %.1f %% dark pixels\n", 100 * share);
		if (share > 0.2) {
			printf("FAIL: the sidebar draws dark\n");
			fflush(stdout);
			_exit(1);
		}
	}
	printf("PANEL_READY\n");
	fflush(stdout);
	[self performSelector:@selector(finish:) withObject:nil afterDelay:6.0];
}
- (void)finish:(id)unused
{
	printf("PASS appkit_open_panel_sidebar\n");
	fflush(stdout);
	_exit(0);
}
@end

int main(int argc, char **argv)
{
	@autoreleasepool {
		[NSApplication sharedApplication];
		[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
		NSSavePanel *panel = (argc > 1) ? [NSSavePanel savePanel] : [NSOpenPanel openPanel];
		Probe *probe = [Probe new];
		probe->_panel = panel;
		[panel beginWithCompletionHandler:^(NSInteger result) {}];
		[probe performSelector:@selector(report:) withObject:nil afterDelay:2.5];
		alarm(60);
		[NSApp run];
	}
	return 0;
}
