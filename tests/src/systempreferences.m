/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

// GUI test of System Preferences.app: it installs copies of a test pane (tests/systempreferences,
// built into a TestPane.prefPane) under different identifiers, opens them the way users do
// (`open x-apple.systempreferences:<identifier>`, `open Foo.prefPane`, `open -a`) and reads what
// the panes log about the host's calls. Needs a display (X11, or Wayland with the application
// started with DARLING_APPKIT_BACKEND=wayland in the environment of this test).
// usage: systempreferences <directory containing TestPane.prefPane>
#import <Foundation/Foundation.h>
#include <signal.h>
#include <errno.h>

static int gFailures;
#define CHECK(cond, ...) do { \
	if (cond) printf("PASS: %s\n", #cond); \
	else { gFailures++; printf("FAIL: %s: ", #cond); printf(__VA_ARGS__); printf("\n"); } \
} while (0)

static NSString *gLog = @"/tmp/systempreferences-test.log";
static NSString *gMode = @"/tmp/systempreferences-test.mode";

static NSString *LogText(void) { return [NSString stringWithContentsOfFile:gLog encoding:NSUTF8StringEncoding error:nil] ?: @""; }
static void SetMode(NSString *mode) { [mode writeToFile:gMode atomically:YES encoding:NSUTF8StringEncoding error:nil]; }

static void Run(NSString *tool, NSArray *args)
{
	NSTask *t = [[NSTask alloc] init];
	[t setLaunchPath:tool];
	[t setArguments:args];
	[t launch];
	[t waitUntilExit];
}

static void Open(NSArray *args) { Run(@"/usr/bin/open", args); }

// The text logged after the offset that contains the needle, waiting up to the timeout.
static NSString *WaitFor(NSUInteger offset, NSString *needle, double timeout)
{
	for (double waited = 0; waited < timeout; waited += 0.25) {
		NSString *text = LogText();
		if ([text length] > offset) {
			NSString *tail = [text substringFromIndex:offset];
			if ([tail rangeOfString:needle].location != NSNotFound)
				return tail;
		}
		[NSThread sleepForTimeInterval:0.25];
	}
	return nil;
}

static BOOL Absent(NSUInteger offset, NSString *needle, double seconds)
{
	[NSThread sleepForTimeInterval:seconds];
	NSString *text = LogText();
	return [text length] <= offset || [[text substringFromIndex:offset] rangeOfString:needle].location == NSNotFound;
}

static NSString *MakePane(NSString *template, NSString *directory, NSString *name, NSString *identifier, NSString *label)
{
	NSFileManager *fm = [NSFileManager defaultManager];
	[fm createDirectoryAtPath:directory withIntermediateDirectories:YES attributes:nil error:nil];
	NSString *dest = [directory stringByAppendingPathComponent:name];
	[fm removeItemAtPath:dest error:nil];
	[fm copyItemAtPath:template toPath:dest error:nil];
	NSString *plist = [dest stringByAppendingPathComponent:@"Contents/Info.plist"];
	NSMutableDictionary *d = [NSMutableDictionary dictionaryWithContentsOfFile:plist];
	d[@"CFBundleIdentifier"] = identifier;
	d[@"NSPrefPaneIconLabel"] = label;
	// the window shows the bundle name, the grid the icon label
	d[@"CFBundleName"] = [label stringByAppendingString:@" Window"];
	[d writeToFile:plist atomically:YES];
	return dest;
}

static int PidOf(NSString *text)
{
	NSRange r = [text rangeOfString:@"pid=" options:NSBackwardsSearch];
	return r.location == NSNotFound ? 0 : atoi([[text substringFromIndex:r.location + 4] UTF8String]);
}

int main(int argc, char **argv)
{
	@autoreleasepool {
		if (argc < 2) { fprintf(stderr, "usage: systempreferences <dir with TestPane.prefPane>\n"); return 2; }
		NSString *template = [[NSString stringWithUTF8String:argv[1]] stringByAppendingPathComponent:@"TestPane.prefPane"];
		NSString *panes = [NSHomeDirectory() stringByAppendingPathComponent:@"Library/PreferencePanes"];
		NSString *tmp = [NSTemporaryDirectory() stringByAppendingPathComponent:@"systempreferences-test-docs"];

		[[NSFileManager defaultManager] removeItemAtPath:gLog error:nil];
		SetMode(@"now");
		// "Test Dock" has the identifier of a pane of the first group; the other two are third-party panes
		MakePane(template, panes, @"Dock.prefPane", @"com.apple.preference.dock", @"Test Dock");
		MakePane(template, panes, @"Zeta.prefPane", @"org.darlinghq.test.zeta", @"Zeta Pane");
		MakePane(template, panes, @"Alpha.prefPane", @"org.darlinghq.test.alpha", @"Alpha Pane");
		NSString *doc = MakePane(template, tmp, @"Doc.prefPane", @"org.darlinghq.test.doc", @"Doc Pane");
		NSString *tail;
		NSUInteger mark;

		// the URL scheme: the first request starts the application and shows the pane
		mark = [LogText() length];
		Open(@[ @"x-apple.systempreferences:com.apple.preference.dock?Test_Anchor" ]);
		tail = WaitFor(mark, @"com.apple.preference.dock pid=", 90);
		CHECK(tail != nil, "url launches the application and loads the pane");
		tail = WaitFor(mark, @"didSelect window=1", 20);
		CHECK(tail != nil, "didSelect with the main view in a window: %s", [LogText() UTF8String]);
		if (tail) {
			CHECK([tail rangeOfString:@"willSelect"].location < [tail rangeOfString:@"didSelect"].location, "willSelect comes first");
			CHECK([tail rangeOfString:@"title=Test Dock Window"].location != NSNotFound, "window title is the pane's bundle name, not its icon label: %s", [tail UTF8String]);
			CHECK([tail rangeOfString:@"mainView=480x270"].location != NSNotFound, "main view keeps its nib size");
			CHECK([tail rangeOfString:@"selected=1"].location != NSNotFound, "isSelected is YES in didSelect");
			CHECK([tail rangeOfString:@"key=set"].location != NSNotFound, "the nib's initialKeyView is available");
			// the window was resized to the pane: width at least the pane's, height more than it (the bar)
			NSRange c = [tail rangeOfString:@"content="];
			double cw = 0, ch = 0;
			sscanf([[tail substringFromIndex:c.location + 8] UTF8String], "%lfx%lf", &cw, &ch);
			CHECK(cw >= 480 && ch > 270 && ch < 270 + 120, "window content size %gx%g fits the pane plus the bar", cw, ch);
		}
		int pid = PidOf(LogText());

		// shouldUnselect: Cancel keeps the pane
		SetMode(@"cancel");
		mark = [LogText() length];
		Open(@[ @"x-apple.systempreferences:org.darlinghq.test.zeta" ]);
		CHECK(WaitFor(mark, @"shouldUnselect mode=cancel", 20) != nil, "host asks shouldUnselect before leaving");
		CHECK(Absent(mark, @"willUnselect", 3), "NSUnselectCancel keeps the pane selected");

		// NSUnselectLater answered with NO keeps it, too
		SetMode(@"later-cancel");
		mark = [LogText() length];
		Open(@[ @"x-apple.systempreferences:org.darlinghq.test.zeta" ]);
		CHECK(WaitFor(mark, @"replyToShouldUnselect:0", 20) != nil, "pane replies later with NO");
		CHECK(Absent(mark, @"willUnselect", 3), "replyToShouldUnselect:NO keeps the pane selected");

		// NSUnselectLater answered with YES lets the host go on
		SetMode(@"later");
		mark = [LogText() length];
		Open(@[ @"x-apple.systempreferences:org.darlinghq.test.zeta" ]);
		NSString *after = WaitFor(mark, @"didSelect window=1 title=Zeta Pane Window", 20);
		CHECK(after != nil, "the pane that was asked for is shown after the delayed reply: %s", [LogText() UTF8String]);
		if (after) {
			NSUInteger r = [after rangeOfString:@"replyToShouldUnselect:1"].location;
			NSUInteger w = [after rangeOfString:@"willUnselect"].location;
			NSUInteger d = [after rangeOfString:@"didUnselect selected=0"].location;
			NSUInteger s = [after rangeOfString:@"title=Zeta Pane Window"].location;
			CHECK(r != NSNotFound && w != NSNotFound && d != NSNotFound && r < w && w < d && d < s, "reply, willUnselect, didUnselect (not selected), then the new pane's didSelect: %s", [after UTF8String]);
		}

		// the anchor of the URL goes to the pane (the host's -handleSystemPreferencesAnchor: convention):
		// to a pane that is already on show, decoded, and to a pane that is selected by the same URL
		SetMode(@"now");
		mark = [LogText() length];
		Open(@[ @"x-apple.systempreferences:org.darlinghq.test.zeta?Privacy_Camera" ]);
		tail = WaitFor(mark, @"anchor Privacy_Camera selected=1", 20);
		CHECK(tail != nil, "an anchor is delivered after didSelect: %s", [LogText() UTF8String]);
		mark = [LogText() length];
		Open(@[ @"x-apple.systempreferences:org.darlinghq.test.zeta?Privacy%20Mic" ]);
		tail = WaitFor(mark, @"anchor Privacy Mic selected=1", 20);
		CHECK(tail != nil, "an anchor reaches the pane on show again, percent-decoded: %s", [LogText() UTF8String]);
		mark = [LogText() length];
		Open(@[ @"x-apple.systempreferences:org.darlinghq.test.alpha?Fresh_Anchor" ]);
		tail = WaitFor(mark, @"anchor Fresh_Anchor selected=1", 20);
		CHECK(tail != nil && [tail rangeOfString:@"didSelect window=1 title=Alpha Pane Window"].location < [tail rangeOfString:@"anchor Fresh_Anchor"].location,
		      "a pane that the URL selects gets the anchor after its didSelect: %s", tail ? [tail UTF8String] : "(none)");
		mark = [LogText() length];
		Open(@[ @"x-apple.systempreferences:org.darlinghq.test.alpha" ]);
		CHECK(Absent(mark, @"anchor ", 3), "a URL without an anchor sends none");

		// the window follows the pane's main view when the pane changes its height
		SetMode(@"grow-on-select");
		mark = [LogText() length];
		Open(@[ @"x-apple.systempreferences:org.darlinghq.test.zeta" ]);
		tail = WaitFor(mark, @"shrunk content=", 30);
		CHECK(tail != nil, "the pane grows and shrinks: %s", [LogText() UTF8String]);
		if (tail) {
			double before = 0, grown = 0, shrunk = 0;
			NSRange c = [tail rangeOfString:@"didSelect window=1"];
			NSRange cc = [tail rangeOfString:@"content=" options:0 range:NSMakeRange(c.location, [tail length] - c.location)];
			sscanf([[tail substringFromIndex:cc.location + 8] UTF8String], "%*lfx%lf", &before);
			NSRange g = [tail rangeOfString:@"grown content="];
			sscanf([[tail substringFromIndex:g.location + 14] UTF8String], "%lf", &grown);
			NSRange sh = [tail rangeOfString:@"shrunk content="];
			sscanf([[tail substringFromIndex:sh.location + 15] UTF8String], "%lf", &shrunk);
			CHECK(grown == before + 130, "the window grew by the pane's 130 points (%g to %g)", before, grown);
			CHECK(shrunk == before, "and shrank back (%g)", shrunk);
		}
		SetMode(@"now");

		// help menu items a pane gives the host
		SetMode(@"help-on-select");
		mark = [LogText() length];
		Open(@[ @"x-apple.systempreferences:org.darlinghq.test.alpha" ]);
		tail = WaitFor(mark, @"helpmenu ", 20);
		CHECK(tail != nil && [tail rangeOfString:@"helpmenu Test Help One,Test Help Two"].location != NSNotFound, "updateHelpMenuWithArray: fills the Help menu: %s", tail ? [tail UTF8String] : "(none)");

		// a pane asking for the grid; the grid lists installed panes by group, third-party ones after
		SetMode(@"showall-on-select");
		mark = [LogText() length];
		Open(@[ @"x-apple.systempreferences:com.apple.preference.dock" ]);
		tail = WaitFor(mark, @"grid ", 30);
		// (the panes that Darling installs, with their own groups, are among them and are left out here)
		// (the label of the Language & Region pane has a line break in it, as the system pane's)
		tail = [tail stringByReplacingOccurrencesOfString:@"Language\n& Region" withString:@"Language & Region"];
		NSMutableArray *gridNames = [NSMutableArray array];
		NSRange gridStart = [tail rangeOfString:@"grid "];
		if (gridStart.location != NSNotFound) {
			NSString *list = [[[tail substringFromIndex:NSMaxRange(gridStart)] componentsSeparatedByString:@"\n"] firstObject];
			NSSet *installed = [NSSet setWithObjects:@"Darling", @"General", @"Language & Region", @"Keyboard", @"Date & Time", nil];
			for (NSString *name in [list componentsSeparatedByString:@","])
				if (![installed containsObject:name])
					[gridNames addObject:name];
		}
		NSString *grid = [@"grid " stringByAppendingString:[gridNames componentsJoinedByString:@","]];
		CHECK(tail != nil && [grid rangeOfString:@"grid Test Dock,Alpha Pane,Zeta Pane"].location != NSNotFound, "grid icons: first-group pane, then third-party panes by name: %s", tail ? [tail UTF8String] : "(none)");
		SetMode(@"now");

		// a .prefPane passed as a document
		mark = [LogText() length];
		Open(@[ doc ]);
		tail = WaitFor(mark, @"didSelect window=1 title=Doc Pane Window", 20);
		CHECK(tail != nil, "open Foo.prefPane shows the pane: %s", [LogText() UTF8String]);

		// the last pane shown is shown again at the next start
		SetMode(@"quit-on-select");
		mark = [LogText() length];
		Open(@[ @"x-apple.systempreferences:org.darlinghq.test.zeta" ]);
		tail = WaitFor(mark, @"didUnselect", 30);
		CHECK(tail != nil && [tail rangeOfString:@"org.darlinghq.test.zeta pid=" ].location != NSNotFound, "quitting leaves the shown pane first: %s", [LogText() UTF8String]);
		BOOL gone = NO;
		for (int i = 0; i < 80 && !gone; i++) {
			gone = kill(pid, 0) != 0 && errno == ESRCH;
			if (!gone) [NSThread sleepForTimeInterval:0.25];
		}
		CHECK(gone, "the application ends after the pane let it");
		SetMode(@"now");
		mark = [LogText() length];
		Open(@[ @"-a", @"System Preferences" ]);
		tail = WaitFor(mark, @"didSelect window=1 title=Zeta Pane Window", 90);
		CHECK(tail != nil, "a plain start shows the last pane: %s", [LogText() UTF8String]);
		int pid2 = PidOf(LogText());
		CHECK(pid2 != 0 && pid2 != pid, "a new process");
		if (pid2 > 0)
			kill(pid2, SIGKILL);
	}
	printf(gFailures ? "FAILED: %d\n" : "ALL PASSED\n", gFailures);
	return gFailures != 0;
}
