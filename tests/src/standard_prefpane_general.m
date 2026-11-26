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

// GUI test of the General pane (Appearance.prefPane, com.apple.preference.general): operates its controls,
// reads the global preferences back with `defaults` in another process, and checks what the application
// (this process) observes: the appearance, the accent colour, the scroll bar style, the notifications.
// usage: standard_prefpane_general
#import "standard_prefpane.h"

extern bool CGFontRenderingGetFontSmoothingDisabled(void);

@interface AppearanceWatcher : NSObject
@property int count;
@end
@implementation AppearanceWatcher
- (void)observeValueForKeyPath:(NSString *)path ofObject:(id)object change:(NSDictionary *)change context:(void *)context { self.count++; }
@end

int main(int argc, char **argv)
{
	@autoreleasepool {
		[NSApplication sharedApplication];
		[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
		for (NSString *key in @[@"AppleInterfaceStyle", @"AppleAccentColor", @"AppleAquaColorVariant", @"AppleHighlightColor", @"AppleShowScrollBars",
		                        @"AppleScrollerPagingBehavior", @"NSTableViewDefaultSizeMode", @"NSQuitAlwaysKeepsWindows", @"NSCloseAlwaysConfirmsChanges",
		                        @"AppleFontSmoothing"])
			SetPref(key, nil, nil);
		SetPref(@"Documents", @"com.apple.recentitems", nil);
		SetPref(@"Applications", @"com.apple.recentitems", nil);
		SetPref(@"Hosts", @"com.apple.recentitems", nil);

		CHECK(LoadPane(@"/System/Library/PreferencePanes/Appearance.prefPane"), "the pane loads");
		NSBundle *bundle = [gPane bundle];
		CHECK([[bundle bundleIdentifier] isEqualToString:@"com.apple.preference.general"], "its identifier is com.apple.preference.general");
		CHECK([[bundle objectForInfoDictionaryKey:@"CFBundleName"] isEqualToString:@"General"], "its name is General");
		CHECK([[bundle objectForInfoDictionaryKey:@"NSPrefPaneIconLabel"] isEqualToString:@"General"], "its icon label is General");

		ObserveDistributed(@"AppleInterfaceThemeChangedNotification");
		ObserveDistributed(@"AppleColorPreferencesChangedNotification");
		ObserveDistributed(@"AppleShowScrollBarsSettingChanged");
		AppearanceWatcher *watcher = [AppearanceWatcher new];
		[NSApp addObserver:watcher forKeyPath:@"effectiveAppearance" options:0 context:NULL];
		__block int colorChanges = 0;
		[[NSNotificationCenter defaultCenter] addObserverForName:NSSystemColorsDidChangeNotification object:nil queue:nil usingBlock:^(NSNotification *n) { colorChanges++; }];

		// Appearance
		CHECK([[[NSApp effectiveAppearance] name] isEqualToString:NSAppearanceNameAqua], "the application starts with the light appearance");
		Choose(@"GeneralAppearance", 1);
		CHECK([GlobalRead(@"AppleInterfaceStyle") isEqualToString:@"Dark"], "Dark writes AppleInterfaceStyle=Dark: %s", [GlobalRead(@"AppleInterfaceStyle") UTF8String]);
		CHECK(WaitUntil(^BOOL{ return NotifiedCount(@"AppleInterfaceThemeChangedNotification") > 0; }, 5), "AppleInterfaceThemeChangedNotification is posted");
		CHECK(WaitUntil(^BOOL{ return [[[NSApp effectiveAppearance] name] isEqualToString:NSAppearanceNameDarkAqua]; }, 5), "the application's effectiveAppearance is dark");
		CHECK(WaitUntil(^BOOL{ return watcher.count > 0; }, 5), "effectiveAppearance announced its change (KVO)");
		Choose(@"GeneralAppearance", 0);
		CHECK(GlobalRead(@"AppleInterfaceStyle") == nil, "Light removes AppleInterfaceStyle");
		CHECK(WaitUntil(^BOOL{ return [[[NSApp effectiveAppearance] name] isEqualToString:NSAppearanceNameAqua]; }, 5), "the application is light again");

		// Accent colour: red is 0, graphite -1 (with the graphite colour variant)
		NSColor *blue = [[NSColor controlAccentColor] colorUsingColorSpace:[NSColorSpace sRGBColorSpace]];
		Press(@"GeneralAccent-3");
		CHECK([GlobalRead(@"AppleAccentColor") isEqualToString:@"0"], "the red swatch writes AppleAccentColor=0: %s", [GlobalRead(@"AppleAccentColor") UTF8String]);
		CHECK(WaitUntil(^BOOL{ return NotifiedCount(@"AppleColorPreferencesChangedNotification") > 0; }, 5), "AppleColorPreferencesChangedNotification is posted");
		CHECK(WaitUntil(^BOOL{ return colorChanges > 0; }, 5), "NSSystemColorsDidChangeNotification reaches the application");
		NSColor *red = [[NSColor controlAccentColor] colorUsingColorSpace:[NSColorSpace sRGBColorSpace]];
		CHECK([red redComponent] > 0.7 && [red blueComponent] < 0.4 && [blue blueComponent] > 0.7, "controlAccentColor is red now (r %.2f b %.2f), was blue (b %.2f)",
		      [red redComponent], [red blueComponent], [blue blueComponent]);
		Press(@"GeneralAccent-7");
		CHECK([GlobalRead(@"AppleAccentColor") isEqualToString:@"-1"] && [GlobalRead(@"AppleAquaColorVariant") isEqualToString:@"6"],
		      "the graphite swatch writes AppleAccentColor=-1 and AppleAquaColorVariant=6");
		CHECK(WaitUntil(^BOOL{ return [NSColor currentControlTint] == NSGraphiteControlTint; }, 5), "the control tint is graphite");
		Press(@"GeneralAccent-0");
		CHECK(GlobalRead(@"AppleAccentColor") == nil && GlobalRead(@"AppleAquaColorVariant") == nil, "the blue swatch removes both keys");

		// Highlight colour
		Choose(@"GeneralHighlight", 5);
		CHECK([GlobalRead(@"AppleHighlightColor") isEqualToString:@"0.752941 0.964706 0.678431 Green"], "Green writes AppleHighlightColor: %s", [GlobalRead(@"AppleHighlightColor") UTF8String]);
		Choose(@"GeneralHighlight", 0);
		CHECK(GlobalRead(@"AppleHighlightColor") == nil, "Blue removes AppleHighlightColor");

		// Sidebar icon size
		Choose(@"GeneralSidebarIconSize", 2);
		CHECK([GlobalRead(@"NSTableViewDefaultSizeMode") isEqualToString:@"3"], "Large writes NSTableViewDefaultSizeMode=3");
		Choose(@"GeneralSidebarIconSize", 1);

		// Scroll bars
		CHECK([NSScroller preferredScrollerStyle] == NSScrollerStyleLegacy, "scroll bars start as legacy ones");
		Choose(@"GeneralScrollBars", 1);
		CHECK([GlobalRead(@"AppleShowScrollBars") isEqualToString:@"WhenScrolling"], "When scrolling writes AppleShowScrollBars");
		CHECK(NotifiedCount(@"AppleShowScrollBarsSettingChanged") > 0 || WaitUntil(^BOOL{ return NotifiedCount(@"AppleShowScrollBarsSettingChanged") > 0; }, 5),
		      "AppleShowScrollBarsSettingChanged is posted");
		CHECK(WaitUntil(^BOOL{ return [NSScroller preferredScrollerStyle] == NSScrollerStyleOverlay; }, 5), "applications get overlay scrollers");
		Choose(@"GeneralScrollBars", 2);
		CHECK([GlobalRead(@"AppleShowScrollBars") isEqualToString:@"Always"], "Always writes AppleShowScrollBars=Always");
		CHECK(WaitUntil(^BOOL{ return [NSScroller preferredScrollerStyle] == NSScrollerStyleLegacy; }, 5), "applications get legacy scrollers");
		Choose(@"GeneralScrollBars", 0);
		CHECK([GlobalRead(@"AppleShowScrollBars") isEqualToString:@"Automatic"], "Automatic writes AppleShowScrollBars=Automatic");
		Choose(@"GeneralScrollPaging", 1);
		CHECK([GlobalRead(@"AppleScrollerPagingBehavior") isEqualToString:@"1"], "Jump to the spot writes AppleScrollerPagingBehavior=1");
		Choose(@"GeneralScrollPaging", 0);
		CHECK([GlobalRead(@"AppleScrollerPagingBehavior") isEqualToString:@"0"], "Jump to the next page writes 0");

		// Default web browser: the list is what Launch Services knows
		NSPopUpButton *browser = Control(@"GeneralBrowser");
		NSArray *handlers = CFBridgingRelease(LSCopyAllHandlersForURLScheme(CFSTR("http")));
		CHECK(browser != nil && (handlers.count == 0 ? [browser numberOfItems] == 1 : (NSUInteger)[browser numberOfItems] >= handlers.count),
		      "the browser pop-up lists the %lu handlers of http", (unsigned long)handlers.count);

		// Recent items
		Choose(@"GeneralRecentItems", 0);
		NSString *recent = DefaultsRead(@[@"com.apple.recentitems", @"Documents"]);
		CHECK(recent != nil && [recent containsString:@"MaxAmount = 0"], "None writes com.apple.recentitems Documents MaxAmount=0: %s", [recent UTF8String]);
		CHECK([DefaultsRead(@[@"com.apple.recentitems", @"Applications"]) containsString:@"MaxAmount = 0"], "and for Applications");
		CHECK([[NSDocumentController sharedDocumentController] maximumRecentDocumentCount] == 0, "applications keep no recent documents");
		Choose(@"GeneralRecentItems", 2);
		CHECK([[NSDocumentController sharedDocumentController] maximumRecentDocumentCount] == 10, "applications keep 10 recent documents");

		// Closing windows and document changes
		NSButton *close = Control(@"GeneralCloseWindowsOnQuit");
		CHECK([close state] == NSControlStateValueOn, "closing windows on quit is on by default");
		Toggle(@"GeneralCloseWindowsOnQuit", NO);
		CHECK([GlobalRead(@"NSQuitAlwaysKeepsWindows") isEqualToString:@"1"], "turning it off writes NSQuitAlwaysKeepsWindows=1");
		Toggle(@"GeneralCloseWindowsOnQuit", YES);
		CHECK([GlobalRead(@"NSQuitAlwaysKeepsWindows") isEqualToString:@"0"], "turning it on writes NSQuitAlwaysKeepsWindows=0");
		Toggle(@"GeneralAskToKeepChanges", YES);
		CHECK([GlobalRead(@"NSCloseAlwaysConfirmsChanges") isEqualToString:@"1"], "Ask to keep changes writes NSCloseAlwaysConfirmsChanges=1");
		Toggle(@"GeneralAskToKeepChanges", NO);
		CHECK([GlobalRead(@"NSCloseAlwaysConfirmsChanges") isEqualToString:@"0"], "and off writes 0");

		// Font smoothing
		CHECK(!CGFontRenderingGetFontSmoothingDisabled(), "font smoothing is enabled by default");
		Choose(@"GeneralFontSmoothing", 1);
		CHECK([GlobalRead(@"AppleFontSmoothing") isEqualToString:@"0"], "Off writes AppleFontSmoothing=0");
		CHECK(CGFontRenderingGetFontSmoothingDisabled(), "font smoothing is disabled for applications");
		Choose(@"GeneralFontSmoothing", 0);
		CHECK(GlobalRead(@"AppleFontSmoothing") == nil, "Default removes AppleFontSmoothing");

		// a second instance of the pane shows what was stored
		Choose(@"GeneralAppearance", 1);
		Choose(@"GeneralScrollBars", 1);
		NSBundle *b2 = [gPane bundle];
		NSPreferencePane *second = [[[b2 principalClass] alloc] initWithBundle:b2];
		NSView *secondView = [second loadMainView];
		NSMutableArray *found = [NSMutableArray array];
		CollectByToolTip(secondView, @"GeneralAppearance", found);
		CollectByToolTip(secondView, @"GeneralScrollBars", found);
		CHECK([found count] == 2 && [(NSPopUpButton *)found[0] indexOfSelectedItem] == 1 && [(NSPopUpButton *)found[1] indexOfSelectedItem] == 1,
		      "a second instance of the pane shows Dark and When scrolling");

		[NSApp removeObserver:watcher forKeyPath:@"effectiveAppearance"];
		[gPane willUnselect];
		[gPane didUnselect];
		RestorePrefs();
		[NSDocumentController class];
	}
	printf(gFailures ? "FAILED: %d\n" : "ALL PASSED\n", gFailures);
	return gFailures != 0;
}
