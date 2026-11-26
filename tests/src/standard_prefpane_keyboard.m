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

// GUI test of the Keyboard pane (Keyboard.prefPane, com.apple.preference.keyboard): operates its controls and
// reads the global preferences back with `defaults` in another process.
// usage: standard_prefpane_keyboard
#import "standard_prefpane.h"

int main(int argc, char **argv)
{
	@autoreleasepool {
		[NSApplication sharedApplication];
		[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
		for (NSString *key in @[@"KeyRepeat", @"InitialKeyRepeat", @"com.apple.keyboard.fnState", @"AppleKeyboardUIMode", @"NSAutomaticSpellingCorrectionEnabled",
		                        @"NSAutomaticCapitalizationEnabled", @"NSAutomaticPeriodSubstitutionEnabled", @"NSAutomaticQuoteSubstitutionEnabled",
		                        @"NSAutomaticDashSubstitutionEnabled"])
			SetPref(key, nil, nil);

		CHECK(LoadPane(@"/System/Library/PreferencePanes/Keyboard.prefPane"), "the pane loads");
		NSBundle *bundle = [gPane bundle];
		CHECK([[bundle bundleIdentifier] isEqualToString:@"com.apple.preference.keyboard"], "its identifier is com.apple.preference.keyboard");
		CHECK([[bundle objectForInfoDictionaryKey:@"CFBundleName"] isEqualToString:@"Keyboard"], "its name is Keyboard");

		// key repeat: the stops from slow (120) to fast (2); 6 and 25 are what applies while nothing is set
		NSSlider *rate = Control(@"KeyboardKeyRepeat"), *delay = Control(@"KeyboardInitialKeyRepeat");
		CHECK([rate integerValue] == 5 && [delay integerValue] == 4, "the sliders show the defaults (stops %ld and %ld)", (long)[rate integerValue], (long)[delay integerValue]);
		Slide(@"KeyboardKeyRepeat", 6);
		CHECK([GlobalRead(@"KeyRepeat") isEqualToString:@"2"], "the fastest repeat writes KeyRepeat=2: %s", [GlobalRead(@"KeyRepeat") UTF8String]);
		Slide(@"KeyboardKeyRepeat", 0);
		CHECK([GlobalRead(@"KeyRepeat") isEqualToString:@"120"], "the slowest writes KeyRepeat=120");
		Slide(@"KeyboardInitialKeyRepeat", 5);
		CHECK([GlobalRead(@"InitialKeyRepeat") isEqualToString:@"15"], "the shortest delay writes InitialKeyRepeat=15");
		Slide(@"KeyboardInitialKeyRepeat", 0);
		CHECK([GlobalRead(@"InitialKeyRepeat") isEqualToString:@"120"], "the longest writes InitialKeyRepeat=120");
		CHECK([[NSUserDefaults standardUserDefaults] integerForKey:@"KeyRepeat"] == 120, "the application reads KeyRepeat from its defaults");

		// function keys, keyboard navigation
		Toggle(@"KeyboardFnState", YES);
		CHECK([GlobalRead(@"com.apple.keyboard.fnState") isEqualToString:@"1"], "the function keys box writes com.apple.keyboard.fnState=1");
		Toggle(@"KeyboardFnState", NO);
		CHECK([GlobalRead(@"com.apple.keyboard.fnState") isEqualToString:@"0"], "and off writes 0");
		Choose(@"KeyboardFullAccess", 1);
		CHECK([GlobalRead(@"AppleKeyboardUIMode") isEqualToString:@"3"], "all controls writes AppleKeyboardUIMode=3");
		Choose(@"KeyboardFullAccess", 0);
		CHECK([GlobalRead(@"AppleKeyboardUIMode") isEqualToString:@"0"], "text boxes and lists writes 0");

		// text options
		NSArray *tips = @[@"KeyboardSpelling", @"KeyboardCapitalize", @"KeyboardPeriod", @"KeyboardQuotes", @"KeyboardDashes"];
		NSArray *keys = @[@"NSAutomaticSpellingCorrectionEnabled", @"NSAutomaticCapitalizationEnabled", @"NSAutomaticPeriodSubstitutionEnabled",
		                  @"NSAutomaticQuoteSubstitutionEnabled", @"NSAutomaticDashSubstitutionEnabled"];
		for (NSUInteger i = 0; i < [tips count]; i++) {
			CHECK([(NSButton *)Control(tips[i]) state] == NSControlStateValueOn, "%s is on by default", [tips[i] UTF8String]);
			Toggle(tips[i], NO);
			CHECK([GlobalRead(keys[i]) isEqualToString:@"0"], "turning %s off writes 0", [keys[i] UTF8String]);
			CHECK(![[NSUserDefaults standardUserDefaults] boolForKey:keys[i]], "the application reads %s as off", [keys[i] UTF8String]);
		}

		// the link to the Darling pane's keyboard section
		NSButton *link = Control(@"KeyboardDarlingLink");
		CHECK(link != nil && [[link title] containsString:@"Darling"], "a button leads to the keyboard settings of Darling");

		[gPane willUnselect];
		[gPane didUnselect];
		RestorePrefs();
	}
	printf(gFailures ? "FAILED: %d\n" : "ALL PASSED\n", gFailures);
	return gFailures != 0;
}
