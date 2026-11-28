// Copyright (C) 2026 Lubos Dolezel
//
// Navigation Services' Save dialog (NavCreatePutFileDialog + NavDialogRun) is a file panel underneath: its
// sidebar of places must be drawn in the light source-list colour, not black, when the dialog is run from a
// Carbon application (the program is a Carbon event loop application, 32-bit only). The program prints the
// sidebar's place on the screen and "PANEL_READY" when the dialog has been drawn, and cancels it after a pause
// so that the harness (open_panel_sidebar.py) can take a screenshot meanwhile.
//
//   carbon_nav_sidebar
#import <Carbon/Carbon.h>
#import <Cocoa/Cocoa.h>
#include <stdio.h>
#include <unistd.h>

#ifdef __LP64__
int main(void)
{
	printf("SIDEBAR 0 0 0 0\nPANEL_READY\nPASS carbon_nav_sidebar (no 64-bit API)\n");
	return 0;
}
#else

static NavDialogRef g_dialog;


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

static void report(EventLoopTimerRef timer, void *userData)
{
	for (NSWindow *window in [NSApp windows]) {
		NSScrollView *sidebar = findSidebar([window contentView]);
		if (!sidebar)
			continue;
		NSRect inWindow = [sidebar convertRect:[sidebar bounds] toView:nil];
		NSRect onScreen = [window convertRectToScreen:inWindow];
		NSScreen *scr = [window screen] ?: [NSScreen mainScreen];
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
		return;
	}
	printf("FAIL: no panel with a sidebar\n");
	fflush(stdout);
	_exit(1);
}

static void finish(EventLoopTimerRef timer, void *userData)
{
	NavDialogDispose(g_dialog);
	printf("PASS carbon_nav_sidebar\n");
	fflush(stdout);
	_exit(0);
}

static void navEvent(NavEventCallbackMessage message, NavCBRecPtr parameters, void *userData)
{
}

int main(void)
{
	NavDialogCreationOptions options;
	EventLoopTimerRef timer;

	alarm(90);
	NavGetDefaultDialogCreationOptions(&options);
	options.modality = kWindowModalityAppModal;
	if (NavCreatePutFileDialog(&options, 'PDF ', 'prvw', navEvent, NULL, &g_dialog) != noErr
	    || NavDialogRun(g_dialog) != noErr) {
		printf("FAIL: the dialog was not created\n");
		return 1;
	}
	InstallEventLoopTimer(GetMainEventLoop(), 3.0, 0, NewEventLoopTimerUPP(report), NULL, &timer);
	InstallEventLoopTimer(GetMainEventLoop(), 10.0, 0, NewEventLoopTimerUPP(finish), NULL, &timer);
	RunApplicationEventLoop();
	return 0;
}
#endif
