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

// GUI test of the Language & Region pane (Localization.prefPane, com.apple.Localization): operates its controls,
// reads the global preferences back with `defaults` in another process, and checks what the application sees
// through NSLocale, NSCalendar and the formatters.
// usage: standard_prefpane_localization
#import "standard_prefpane.h"

static NSString *Text(NSString *tip) { return [(NSTextField *)Control(tip) stringValue]; }

static NSInteger IndexOfLanguage(NSString *code)
{
	NSPopUpButton *add = Control(@"LocalizationAddLanguage");
	for (NSInteger i = 0; i < [add numberOfItems]; i++)
		if ([[[add itemAtIndex:i] representedObject] isEqualToString:code])
			return i;
	return -1;
}

static void SelectLanguageRow(NSInteger row)
{
	NSTableView *table = Control(@"LocalizationLanguages");
	[table selectRowIndexes:[NSIndexSet indexSetWithIndex:row] byExtendingSelection:NO];
	Spin(0.3);
}

int main(int argc, char **argv)
{
	@autoreleasepool {
		[NSApplication sharedApplication];
		[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
		for (NSString *key in @[@"AppleLanguages", @"AppleLocale", @"AppleFirstWeekday", @"AppleICUForce24HourTime", @"AppleTemperatureUnit",
		                        @"AppleMeasurementUnits", @"AppleMetricUnits"])
			Remember(key, nil);
		SetPref(@"AppleLanguages", nil, @[@"en-US"]);
		SetPref(@"AppleLocale", nil, @"en_US");
		SetPref(@"AppleFirstWeekday", nil, nil);
		SetPref(@"AppleICUForce24HourTime", nil, nil);
		SetPref(@"AppleTemperatureUnit", nil, nil);
		SetPref(@"AppleMeasurementUnits", nil, nil);
		SetPref(@"AppleMetricUnits", nil, nil);

		CHECK(LoadPane(@"/System/Library/PreferencePanes/Localization.prefPane"), "the pane loads");
		NSBundle *bundle = [gPane bundle];
		CHECK([[bundle bundleIdentifier] isEqualToString:@"com.apple.Localization"], "its identifier is com.apple.Localization");
		CHECK([[bundle objectForInfoDictionaryKey:@"CFBundleName"] isEqualToString:@"Language & Region"], "its name is Language & Region");

		// preferred languages
		NSTableView *languages = Control(@"LocalizationLanguages");
		CHECK(languages != nil && [[languages dataSource] numberOfRowsInTableView:languages] == 1, "the list shows the one preferred language");
		NSInteger de = IndexOfLanguage(@"de");
		CHECK(de > 0, "German is offered");
		NSPopUpButton *add = Control(@"LocalizationAddLanguage");
		[add selectItemAtIndex:de];
		[NSApp sendAction:[add action] to:[add target] from:add];
		Spin(0.3);
		NSString *list = GlobalRead(@"AppleLanguages");
		CHECK(list != nil && [list containsString:@"en-US"] && [list containsString:@"de"], "adding German appends it to AppleLanguages: %s", [list UTF8String]);
		CHECK([[languages dataSource] numberOfRowsInTableView:languages] == 2, "the list has two rows");
		SelectLanguageRow(1);
		Press(@"LocalizationLanguageUp");
		CHECK([[[NSLocale preferredLanguages] firstObject] hasPrefix:@"de"], "moving German up makes it the application's first language: %s", [[[NSLocale preferredLanguages] description] UTF8String]);
		CHECK([[languages dataSource] tableView:languages objectValueForTableColumn:[languages tableColumns][0] row:0] != nil
		      && [(NSString *)[[languages dataSource] tableView:languages objectValueForTableColumn:[languages tableColumns][0] row:0] hasSuffix:@"(de)"], "German is the first row");
		Press(@"LocalizationLanguageDown");
		CHECK([[[NSLocale preferredLanguages] firstObject] hasPrefix:@"en"], "moving it down puts English first again");
		SelectLanguageRow(1);
		Press(@"LocalizationRemoveLanguage");
		CHECK([[languages dataSource] numberOfRowsInTableView:languages] == 1 && ![GlobalRead(@"AppleLanguages") containsString:@"de"], "removing German leaves one language");
		Press(@"LocalizationRemoveLanguage");
		CHECK([[languages dataSource] numberOfRowsInTableView:languages] == 1, "the last language cannot be removed");

		// 24-hour time, with the region still en_US
		NSLocale *current = [NSLocale currentLocale];
		NSString *format12 = [NSDateFormatter dateFormatFromTemplate:@"j" options:0 locale:current];
		CHECK([format12 containsString:@"a"], "a 12-hour clock by default: %s", [format12 UTF8String]);
		Toggle(@"LocalizationTime24", YES);
		CHECK([GlobalRead(@"AppleICUForce24HourTime") isEqualToString:@"1"], "24-Hour Time writes AppleICUForce24HourTime=1");
		NSString *format24 = [NSDateFormatter dateFormatFromTemplate:@"j" options:0 locale:[NSLocale currentLocale]];
		CHECK(![format24 containsString:@"a"] && [format24 containsString:@"H"], "formatters use a 24-hour clock: %s", [format24 UTF8String]);
		CHECK([Text(@"LocalizationPreviewTime") isEqualToString:@"22:13"], "the preview shows 22:13: %s", [Text(@"LocalizationPreviewTime") UTF8String]);
		Toggle(@"LocalizationTime24", NO);
		CHECK([GlobalRead(@"AppleICUForce24HourTime") isEqualToString:@"0"], "and off writes 0");

		// region
		NSPopUpButton *region = Control(@"LocalizationRegion");
		NSInteger deRegion = [region indexOfItemWithTitle:@"German (Germany) (de_DE)"];
		if (deRegion < 0) {
			for (NSInteger i = 0; i < [region numberOfItems]; i++)
				if ([[region itemTitleAtIndex:i] hasSuffix:@"(de_DE)"])
					deRegion = i;
		}
		CHECK(deRegion >= 0, "Germany is offered as a region");
		Choose(@"LocalizationRegion", deRegion);
		CHECK([GlobalRead(@"AppleLocale") isEqualToString:@"de_DE"], "choosing Germany writes AppleLocale=de_DE: %s", [GlobalRead(@"AppleLocale") UTF8String]);
		CHECK([[[NSLocale currentLocale] localeIdentifier] isEqualToString:@"de_DE"], "NSLocale's current locale is de_DE: %s", [[[NSLocale currentLocale] localeIdentifier] UTF8String]);
		NSString *number = [NSNumberFormatter localizedStringFromNumber:@1234567.89 numberStyle:NSNumberFormatterDecimalStyle];
		CHECK([number isEqualToString:@"1.234.567,89"], "a number formats as 1.234.567,89: %s", [number UTF8String]);
		CHECK([Text(@"LocalizationPreviewNumber") isEqualToString:@"1.234.567,89"], "and so does the preview: %s", [Text(@"LocalizationPreviewNumber") UTF8String]);
		CHECK([Text(@"LocalizationPreviewCurrency") containsString:@"€"], "the currency preview is in euros: %s", [Text(@"LocalizationPreviewCurrency") UTF8String]);

		// calendar
		NSPopUpButton *calendar = Control(@"LocalizationCalendar");
		CHECK([[calendar titleOfSelectedItem] isEqualToString:@"Gregorian"], "the calendar is Gregorian");
		Choose(@"LocalizationCalendar", 1);
		CHECK([GlobalRead(@"AppleLocale") isEqualToString:@"de_DE@calendar=buddhist"], "Buddhist writes the calendar into AppleLocale: %s", [GlobalRead(@"AppleLocale") UTF8String]);
		CHECK([[[[NSLocale currentLocale] objectForKey:NSLocaleCalendar] calendarIdentifier] isEqualToString:NSCalendarIdentifierBuddhist],
		      "the current locale's calendar is Buddhist: %s", [[[[NSLocale currentLocale] objectForKey:NSLocaleCalendar] calendarIdentifier] UTF8String]);
		CHECK([[[NSLocale currentLocale] localeIdentifier] hasPrefix:@"de_DE"], "and the region is still Germany");
		Choose(@"LocalizationCalendar", 0);
		CHECK([GlobalRead(@"AppleLocale") isEqualToString:@"de_DE"], "Gregorian writes the plain locale again");

		// first day of the week
		Choose(@"LocalizationFirstWeekday", 7);
		CHECK([[GlobalRead(@"AppleFirstWeekday") stringByReplacingOccurrencesOfString:@" " withString:@""] containsString:@"gregorian=7"], "Saturday writes AppleFirstWeekday: %s", [GlobalRead(@"AppleFirstWeekday") UTF8String]);
		CHECK([[NSCalendar currentCalendar] firstWeekday] == 7, "the current calendar's first weekday is 7: %ld", (long)[[NSCalendar currentCalendar] firstWeekday]);
		Choose(@"LocalizationFirstWeekday", 0);
		CHECK(GlobalRead(@"AppleFirstWeekday") == nil, "the default removes it");

		// units
		CHECK([[[NSLocale currentLocale] objectForKey:NSLocaleUsesMetricSystem] boolValue], "Germany uses the metric system");
		Choose(@"LocalizationMeasurement", 2);
		CHECK([GlobalRead(@"AppleMeasurementUnits") isEqualToString:@"Inches"] && [GlobalRead(@"AppleMetricUnits") isEqualToString:@"0"], "US units write AppleMeasurementUnits=Inches and AppleMetricUnits=0");
		CHECK(![[[NSLocale currentLocale] objectForKey:NSLocaleUsesMetricSystem] boolValue], "the current locale no longer uses the metric system");
		Choose(@"LocalizationMeasurement", 0);
		CHECK(GlobalRead(@"AppleMeasurementUnits") == nil && GlobalRead(@"AppleMetricUnits") == nil, "the default removes both");
		Choose(@"LocalizationTemperature", 2);
		CHECK([GlobalRead(@"AppleTemperatureUnit") isEqualToString:@"Fahrenheit"], "Fahrenheit writes AppleTemperatureUnit");
		Choose(@"LocalizationTemperature", 0);
		CHECK(GlobalRead(@"AppleTemperatureUnit") == nil, "the default removes it");

		// a second instance shows the region
		NSPreferencePane *second = [[[bundle principalClass] alloc] initWithBundle:bundle];
		NSView *secondView = [second loadMainView];
		NSMutableArray *found = [NSMutableArray array];
		CollectByToolTip(secondView, @"LocalizationRegion", found);
		CHECK([found count] == 1 && [[(NSPopUpButton *)found[0] titleOfSelectedItem] hasSuffix:@"(de_DE)"], "a second instance of the pane shows Germany");

		[gPane willUnselect];
		[gPane didUnselect];
		RestorePrefs();
	}
	printf(gFailures ? "FAILED: %d\n" : "ALL PASSED\n", gFailures);
	return gFailures != 0;
}
