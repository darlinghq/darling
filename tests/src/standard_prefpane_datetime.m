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

// GUI test of the Date & Time pane (DateAndTime.prefPane, com.apple.preference.datetime): the clock and the
// time zone shown are those the application sees, and the clock options are written to the menu bar clock's
// preferences, which `defaults` reads back in another process.
// usage: standard_prefpane_datetime
#import "standard_prefpane.h"

static NSString *Text(NSString *tip) { return [(NSTextField *)Control(tip) stringValue]; }

static NSString *ClockRead(NSString *key)
{
	return DefaultsRead(@[@"com.apple.menuextra.clock", key]);
}

int main(int argc, char **argv)
{
	@autoreleasepool {
		[NSApplication sharedApplication];
		[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
		for (NSString *key in @[@"IsAnalog", @"ShowSeconds", @"ShowAMPM", @"ShowDayOfWeek", @"ShowDate", @"FlashDateSeparators"])
			SetPref(key, @"com.apple.menuextra.clock", nil);
		SetPref(@"AppleICUForce24HourTime", nil, nil);
		SetPref(@"AppleLocale", nil, @"en_US");

		CHECK(LoadPane(@"/System/Library/PreferencePanes/DateAndTime.prefPane"), "the pane loads");
		NSBundle *bundle = [gPane bundle];
		CHECK([[bundle bundleIdentifier] isEqualToString:@"com.apple.preference.datetime"], "its identifier is com.apple.preference.datetime");
		CHECK([[bundle objectForInfoDictionaryKey:@"CFBundleName"] isEqualToString:@"Date & Time"], "its name is Date & Time");

		// the date, time and time zone
		NSString *zoneName = [[NSTimeZone systemTimeZone] name];
		CHECK([zoneName length] > 0 && [Text(@"DateTimeZone") containsString:zoneName], "the pane shows the system time zone %s: %s", [zoneName UTF8String], [Text(@"DateTimeZone") UTF8String]);
		NSString *link = [[NSFileManager defaultManager] destinationOfSymbolicLinkAtPath:@"/etc/localtime" error:nil];
		NSRange range = [link rangeOfString:@"zoneinfo/"];
		if (link != nil && range.location != NSNotFound)
			CHECK([[link substringFromIndex:NSMaxRange(range)] isEqualToString:zoneName], "the application's system time zone is the one /etc/localtime names: %s vs %s", [link UTF8String], [zoneName UTF8String]);
		NSString *first = Text(@"DateTimeCurrent");
		CHECK([first length] > 10, "the pane shows the current date and time: %s", [first UTF8String]);
		CHECK(WaitUntil(^BOOL{ return ![Text(@"DateTimeCurrent") isEqualToString:first]; }, 5), "the time moves on while the pane is shown");
		NSDateFormatter *f = [NSDateFormatter new];
		[f setLocale:[NSLocale localeWithLocaleIdentifier:@"en_US"]];
		[f setDateStyle:NSDateFormatterFullStyle];
		CHECK([Text(@"DateTimeCurrent") hasPrefix:[f stringFromDate:[NSDate date]]], "its date is today's");

		// the clock options
		struct { NSString *tip, *key; BOOL initial; } checks[] = {
			{ @"DateTimeAnalog", @"IsAnalog", NO }, { @"DateTimeSeconds", @"ShowSeconds", NO }, { @"DateTimeAMPM", @"ShowAMPM", YES },
			{ @"DateTimeDayOfWeek", @"ShowDayOfWeek", YES }, { @"DateTimeFlash", @"FlashDateSeparators", NO } };
		for (size_t i = 0; i < sizeof(checks) / sizeof(checks[0]); i++) {
			CHECK([(NSButton *)Control(checks[i].tip) state] == (checks[i].initial ? NSControlStateValueOn : NSControlStateValueOff), "%s starts %s", [checks[i].tip UTF8String], checks[i].initial ? "on" : "off");
			Toggle(checks[i].tip, !checks[i].initial);
			CHECK([ClockRead(checks[i].key) isEqualToString:(checks[i].initial ? @"0" : @"1")], "toggling %s writes the clock's %s: %s", [checks[i].tip UTF8String], [checks[i].key UTF8String], [ClockRead(checks[i].key) UTF8String]);
		}
		Choose(@"DateTimeShowDate", 2);
		CHECK([ClockRead(@"ShowDate") isEqualToString:@"2"], "Never writes ShowDate=2");

		// the 24-hour clock is the global setting that formatters follow
		NSString *format12 = [NSDateFormatter dateFormatFromTemplate:@"j" options:0 locale:[NSLocale currentLocale]];
		CHECK([format12 containsString:@"a"], "a 12-hour clock by default: %s", [format12 UTF8String]);
		Toggle(@"DateTime24Hour", YES);
		CHECK([GlobalRead(@"AppleICUForce24HourTime") isEqualToString:@"1"], "the 24-hour box writes AppleICUForce24HourTime=1");
		NSString *format24 = [NSDateFormatter dateFormatFromTemplate:@"j" options:0 locale:[NSLocale currentLocale]];
		CHECK(![format24 containsString:@"a"], "formatters use a 24-hour clock: %s", [format24 UTF8String]);

		[gPane willUnselect];
		[gPane didUnselect];
		RestorePrefs();
	}
	printf(gFailures ? "FAILED: %d\n" : "ALL PASSED\n", gFailures);
	return gFailures != 0;
}
