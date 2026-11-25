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

// Contract tests of PreferencePanes.framework's NSPreferencePane.
// usage: preferencepanes <directory with Main.nib and Plain.nib> [output file]
// (the output file is for systems where `open` cannot redirect the output)
// Test fixtures: tests/preferencepanes (Main.xib and Plain.xib compiled with ibtool).
// Behaviour the header does not spell out is pinned from black-box runs of this
// test and of a probe on macOS 10.14.4 and 26.7.3 (this test builds and passes
// there, except for the nib cases on 32-bit i386 of 10.14.4, whose AppKit
// cannot load the fixtures).
#import <Cocoa/Cocoa.h>
#import <PreferencePanes/PreferencePanes.h>
#import <objc/runtime.h>

static int gFailures;
#define CHECK(cond, ...) do { \
	if (cond) printf("PASS: %s\n", #cond); \
	else { gFailures++; printf("FAIL: %s: ", #cond); printf(__VA_ARGS__); printf("\n"); } \
} while (0)

static NSMutableArray *gLog;

// LogPane records every call the framework makes on the pane
@interface LogPane : NSPreferencePane
{
	BOOL _overrideAssign;     // explicit instance variables: the 32-bit runtime does not synthesize them
	BOOL _laterReply;
	NSString *_nibNameOverride;
}
@property BOOL overrideAssign;
@property BOOL laterReply;
@property (copy) NSString *nibNameOverride;
@end

@implementation LogPane
@synthesize overrideAssign = _overrideAssign, laterReply = _laterReply, nibNameOverride = _nibNameOverride;
- (void)note:(NSString *)s { [gLog addObject:s]; }
- (NSView *)loadMainView { [self note:@"loadMainView"]; return [super loadMainView]; }
- (void)mainViewDidLoad { [self note:@"mainViewDidLoad"]; [super mainViewDidLoad]; }
- (NSString *)mainNibName { [self note:@"mainNibName"]; return self.nibNameOverride ? self.nibNameOverride : [super mainNibName]; }
- (void)assignMainView { [self note:@"assignMainView"]; if (!self.overrideAssign) [super assignMainView]; }
- (NSPreferencePaneUnselectReply)shouldUnselect { return self.laterReply ? NSUnselectLater : [super shouldUnselect]; }
- (void)setMainView:(NSView *)v { [self note:@"setMainView:"]; [super setMainView:v]; }
- (void)setInitialKeyView:(NSView *)v { [self note:@"setInitialKeyView:"]; [super setInitialKeyView:v]; }
@end

static NSString *LogString(void) { return [gLog componentsJoinedByString:@" "]; }

static NSBundle *MakeBundle(NSString *root, NSString *name, id mainNib, NSArray *nibs, BOOL localized)
{
	NSFileManager *fm = [NSFileManager defaultManager];
	NSString *b = [root stringByAppendingPathComponent:name];
	NSString *res = [b stringByAppendingPathComponent:@"Contents/Resources"];
	[fm createDirectoryAtPath:res withIntermediateDirectories:YES attributes:nil error:nil];
	NSMutableDictionary *d = [NSMutableDictionary dictionaryWithObjectsAndKeys:[@"org.darlinghq.test." stringByAppendingString:name], @"CFBundleIdentifier", name, @"CFBundleName", nil];
	if (mainNib)
		[d setObject:mainNib forKey:@"NSMainNibFile"];
	[d writeToFile:[b stringByAppendingPathComponent:@"Contents/Info.plist"] atomically:YES];
	for (NSArray *pair in nibs) { // @[source path, destination name]
		NSString *dir = localized ? [res stringByAppendingPathComponent:@"English.lproj"] : res;
		[fm createDirectoryAtPath:dir withIntermediateDirectories:YES attributes:nil error:nil];
		[fm copyItemAtPath:pair[0] toPath:[dir stringByAppendingFormat:@"/%@.nib", pair[1]] error:nil];
	}
	return [NSBundle bundleWithPath:b];
}

@interface NotificationRecorder : NSObject
{
	NSMutableArray *_seen;
}
@property (retain) NSMutableArray *seen;
- (void)note:(NSNotification *)n;
@end
@implementation NotificationRecorder
@synthesize seen = _seen;
- (instancetype)init { if ((self = [super init])) _seen = [NSMutableArray new]; return self; }
- (void)note:(NSNotification *)n { [self.seen addObject:n]; }
@end

int main(int argc, char **argv)
{
	@autoreleasepool {
		if (argc < 2) { fprintf(stderr, "usage: preferencepanes <fixtures dir>\n"); return 2; }
		setvbuf(stdout, NULL, _IONBF, 0);
		if (argc > 2)
			freopen(argv[2], "w", stdout);
		[NSApplication sharedApplication];
		gLog = [NSMutableArray new];
		NSString *fixtures = [NSString stringWithUTF8String:argv[1]];
		NSString *mainNib = [fixtures stringByAppendingPathComponent:@"Main.nib"];
		NSString *plainNib = [fixtures stringByAppendingPathComponent:@"Plain.nib"];
		NSString *tmp = [NSTemporaryDirectory() stringByAppendingPathComponent:[NSString stringWithFormat:@"preferencepanes-%d", getpid()]];
		[[NSFileManager defaultManager] createDirectoryAtPath:tmp withIntermediateDirectories:YES attributes:nil error:nil];

		// constants
		CHECK([NSPreferencePaneDoUnselectNotification isEqualToString:@"NSPreferencePaneDoUnselectNotification"], "value");
		CHECK([NSPreferencePaneCancelUnselectNotification isEqualToString:@"NSPreferencePaneCancelUnselectNotification"], "value");
		CHECK([NSPreferencePaneSwitchToPaneNotification isEqualToString:@"NSPreferencePaneSwitchToPaneNotification"], "value");
		CHECK([NSPreferencePrefPaneIsAvailableNotification isEqualToString:@"NSPreferencePrefPaneIsAvailableNotification"], "value");
		CHECK([NSPreferencePaneUpdateHelpMenuNotification isEqualToString:@"NSPreferencePaneUpdateHelpMenuNotification"], "value");
		CHECK([NSPrefPaneHelpMenuInfoPListKey isEqualToString:@"NSPrefPaneHelpAnchors"], "value");
		CHECK([NSPrefPaneHelpMenuTitleKey isEqualToString:@"title"], "value");
		CHECK([NSPrefPaneHelpMenuAnchorKey isEqualToString:@"anchor"], "value");
		CHECK(NSUnselectCancel == 0 && NSUnselectNow == 1 && NSUnselectLater == 2, "enum values");

		// instance layout of the header
		{
			unsigned n; Ivar *iv = class_copyIvarList([NSPreferencePane class], &n);
			NSMutableArray *names = [NSMutableArray array];
			for (unsigned i = 0; i < n; i++) [names addObject:[NSString stringWithUTF8String:ivar_getName(iv[i])]];
			free(iv);
			NSString *want = @"_window _initialKeyView _firstKeyView _lastKeyView _mainView _bundle _reserved1 _reserved2 _reserved3";
			CHECK([[names componentsJoinedByString:@" "] isEqualToString:want], "ivars: %s", [[names componentsJoinedByString:@" "] UTF8String]);
		}

		NSBundle *mainBundle = MakeBundle(tmp, @"main.bundle", @"Main", @[ @[ mainNib, @"Main" ], @[ plainNib, @"Plain" ], @[ mainNib, @"Alt" ] ], NO);

		// a fresh pane
		{
			LogPane *p = [[LogPane alloc] initWithBundle:mainBundle];
			CHECK(p.bundle == mainBundle, "bundle");
			CHECK(!p.isSelected, "selected");
			CHECK(p.autoSaveTextFields, "autoSaveTextFields default is YES");
			CHECK(p.shouldUnselect == NSUnselectNow, "default shouldUnselect is NSUnselectNow");
			CHECK(p.mainView == nil && p.initialKeyView == nil && p.firstKeyView == nil && p.lastKeyView == nil, "views unset");
			CHECK([[p mainNibName] isEqualToString:@"Main"], "mainNibName from NSMainNibFile");
			p.laterReply = YES;
			CHECK(p.shouldUnselect == NSUnselectLater, "override can answer NSUnselectLater");
		}

		// loadMainView with a nib
		{
			LogPane *p = [[LogPane alloc] initWithBundle:mainBundle];
			[gLog removeAllObjects];
			NSView *v = [p loadMainView];
			CHECK([LogString() isEqualToString:@"loadMainView mainNibName assignMainView setMainView: mainViewDidLoad"], "call order: %s", [LogString() UTF8String]);
			CHECK(v != nil && v == p.mainView, "loadMainView returns the main view");
			CHECK(NSEqualRects(v.frame, NSMakeRect(0, 0, 480, 270)), "frame of the nib's content view: %s", [NSStringFromRect(v.frame) UTF8String]);
			CHECK(v.superview == nil && v.window == nil, "main view is detached from the nib window");
			CHECK(v.subviews.count == 3, "subviews: %lu", (unsigned long)v.subviews.count);
			CHECK([(NSButton *)p.initialKeyView isKindOfClass:[NSButton class]] && [[(NSButton *)p.initialKeyView title] isEqualToString:@"Initial"], "initialKeyView outlet");
			CHECK([[(NSButton *)p.firstKeyView title] isEqualToString:@"First"], "firstKeyView outlet");
			CHECK([[(NSButton *)p.lastKeyView title] isEqualToString:@"Last"], "lastKeyView outlet");
			CHECK([p.initialKeyView isDescendantOf:v], "key views are subviews of the main view");
			CHECK([p valueForKey:@"_window"] == nil, "the nib window is released after assignMainView");
			CHECK(!p.isSelected, "loading does not select");
			NSView *v2 = [p loadMainView];
			CHECK(v2 != nil && v2 != v && v2 == p.mainView, "a second load makes a new view");
		}

		// a nib that does not connect the key views
		{
			NSBundle *b = MakeBundle(tmp, @"plain.bundle", @"Plain", @[ @[ plainNib, @"Plain" ] ], NO);
			LogPane *p = [[LogPane alloc] initWithBundle:b];
			NSView *v = [p loadMainView];
			CHECK(v != nil && v == p.mainView, "plain: main view");
			CHECK(p.initialKeyView == nil && p.firstKeyView == nil && p.lastKeyView == nil, "no default key views are invented");
		}

		// QUIRK test: no NSMainNibFile in the Info.plist
		{
			NSBundle *b = MakeBundle(tmp, @"nokey.bundle", nil, @[ @[ mainNib, @"Main" ] ], NO);
			LogPane *p = [[LogPane alloc] initWithBundle:b];
			CHECK([[p mainNibName] isEqualToString:@"Main"], "test_quirk_main_nib_name_defaults_to_main");
			CHECK([p loadMainView] == p.mainView && p.mainView != nil, "the default name finds Main.nib");
		}

		// nib names: the extension is not part of the name, localized nibs are found
		{
			NSBundle *b = MakeBundle(tmp, @"ext.bundle", @"Main.nib", @[ @[ mainNib, @"Main" ] ], NO);
			LogPane *p = [[LogPane alloc] initWithBundle:b];
			CHECK([[p mainNibName] isEqualToString:@"Main.nib"], "mainNibName is returned as given");
			[gLog removeAllObjects];
			NSView *v = [p loadMainView];
			CHECK(p.mainView == nil, "a name with extension finds no nib; main view stays unset");
			CHECK(v != nil && NSEqualRects(v.frame, NSZeroRect), "failure still returns a (empty) view");
			CHECK([LogString() rangeOfString:@"assignMainView"].location == NSNotFound && [LogString() rangeOfString:@"mainViewDidLoad"].location == NSNotFound, "failure: no assignMainView/mainViewDidLoad: %s", [LogString() UTF8String]);

			b = MakeBundle(tmp, @"loc.bundle", @"Main", @[ @[ mainNib, @"Main" ] ], YES);
			p = [[LogPane alloc] initWithBundle:b];
			CHECK([p loadMainView] == p.mainView && p.mainView != nil, "localized nib is found");

			b = MakeBundle(tmp, @"missing.bundle", @"Missing", @[], NO);
			p = [[LogPane alloc] initWithBundle:b];
			v = [p loadMainView];
			CHECK(p.mainView == nil && v != nil, "missing nib");
		}

		// subclass hooks
		{
			LogPane *p = [[LogPane alloc] initWithBundle:mainBundle];
			p.nibNameOverride = @"Alt";
			[gLog removeAllObjects];
			CHECK([p loadMainView] == p.mainView && p.mainView != nil, "mainNibName override decides the nib");

			p = [[LogPane alloc] initWithBundle:mainBundle];
			[gLog removeAllObjects];
			[p assignMainView];
			CHECK(p.mainView == nil, "assignMainView without a window leaves the main view unset");

			p = [[LogPane alloc] initWithBundle:mainBundle];
			p.overrideAssign = YES;
			[gLog removeAllObjects];
			NSView *v = [p loadMainView];
			CHECK(v == nil && [LogString() hasSuffix:@"assignMainView mainViewDidLoad"], "assign override that sets nothing: %s", [LogString() UTF8String]);
		}

		// selection state
		{
			LogPane *p = [[LogPane alloc] initWithBundle:mainBundle];
			[p willSelect];
			CHECK(!p.isSelected, "willSelect");
			[p didSelect];
			CHECK(p.isSelected, "didSelect selects");
			[p willUnselect];
			CHECK(p.isSelected, "willUnselect");
			[p didUnselect];
			CHECK(!p.isSelected, "didUnselect deselects");
		}

		// setters
		{
			LogPane *p = [[LogPane alloc] initWithBundle:mainBundle];
			NSView *a = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 10, 20)];
			p.mainView = a;
			p.firstKeyView = a;
			p.initialKeyView = nil;
			CHECK(p.mainView == a && p.firstKeyView == a && p.initialKeyView == nil, "key view and main view setters");
		}

		// notifications
		{
			LogPane *p = [[LogPane alloc] initWithBundle:mainBundle];
			NotificationRecorder *r = [NotificationRecorder new];
			NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
			NSArray *names = @[ NSPreferencePaneDoUnselectNotification, NSPreferencePaneCancelUnselectNotification, NSPreferencePaneUpdateHelpMenuNotification ];
			for (NSString *n in names)
				[nc addObserver:r selector:@selector(note:) name:n object:nil];

			[p replyToShouldUnselect:YES];
			CHECK(r.seen.count == 1 && [[r.seen[0] name] isEqualToString:NSPreferencePaneDoUnselectNotification] && [r.seen[0] object] == p && [r.seen[0] userInfo] == nil, "replyToShouldUnselect:YES posts DoUnselect for the pane");
			[r.seen removeAllObjects];
			[p replyToShouldUnselect:NO];
			CHECK(r.seen.count == 1 && [[r.seen[0] name] isEqualToString:NSPreferencePaneCancelUnselectNotification] && [r.seen[0] object] == p, "replyToShouldUnselect:NO posts CancelUnselect for the pane");
			[r.seen removeAllObjects];

			NSArray *items = @[ @{ NSPrefPaneHelpMenuTitleKey: @"Help item", NSPrefPaneHelpMenuAnchorKey: @"anch" } ];
			[p updateHelpMenuWithArray:items];
			CHECK(r.seen.count == 1 && [r.seen[0] object] == items && [r.seen[0] userInfo] == nil, "test_quirk_update_help_menu_notification_object");
			[r.seen removeAllObjects];
			[p updateHelpMenuWithArray:nil];
			CHECK(r.seen.count == 1 && [r.seen[0] object] == nil, "update help menu with nil");
			[nc removeObserver:r];
		}

		[[NSFileManager defaultManager] removeItemAtPath:tmp error:nil];
	}
	printf(gFailures ? "FAILED: %d\n" : "ALL PASSED\n", gFailures);
	return gFailures != 0;
}
