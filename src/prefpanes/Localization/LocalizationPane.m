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

#import "PKPane.h"

// The Language & Region pane (com.apple.Localization), in the global preferences domain:
//  - AppleLanguages: the preferred languages, most preferred first;
//  - AppleLocale: the region's locale identifier; a calendar other than the region's own is a
//    "@calendar=<name>" suffix of it;
//  - AppleFirstWeekday: a dictionary from the calendar's identifier to the first day of the week
//    (1 Sunday ... 7 Saturday);
//  - AppleICUForce24HourTime: use the 24-hour clock;
//  - AppleTemperatureUnit (Celsius, Fahrenheit), AppleMeasurementUnits (Centimeters, Inches)
//    and AppleMetricUnits (a boolean).
// Core Foundation's locale and date formatting read these keys, so what the pane shows in its
// preview is what formatters in applications produce.

static NSString * const kLanguages = @"AppleLanguages";
static NSString * const kLocale = @"AppleLocale";

static NSString *LanguageName(NSString *code)
{
    NSString *name = [[NSLocale currentLocale] displayNameForKey:NSLocaleIdentifier value:[code stringByReplacingOccurrencesOfString:@"-" withString:@"_"]];
    return name != nil ? name : code;
}

static NSArray<NSString *> *KnownLanguages(void)
{
    return @[@"en", @"en-GB", @"en-AU", @"de", @"fr", @"es", @"it", @"pt-BR", @"pt-PT", @"nl", @"sv", @"da", @"nb", @"fi", @"cs", @"pl",
             @"hu", @"ru", @"uk", @"tr", @"el", @"he", @"ar", @"ja", @"ko", @"zh-Hans", @"zh-Hant"];
}

static NSString *BaseLocale(NSString *identifier)
{
    NSRange at = [identifier rangeOfString:@"@"];
    return at.location == NSNotFound ? identifier : [identifier substringToIndex:at.location];
}

static NSString *CalendarOf(NSString *identifier)
{
    NSRange at = [identifier rangeOfString:@"@calendar="];
    if (at.location == NSNotFound)
        return @"gregorian";
    NSString *rest = [identifier substringFromIndex:NSMaxRange(at)];
    NSRange end = [rest rangeOfString:@";"];
    return end.location == NSNotFound ? rest : [rest substringToIndex:end.location];
}

static NSString *StoredLocale(void)
{
    id stored = [PKPane preferenceForKey:kLocale domain:nil currentHost:NO];
    if ([stored isKindOfClass:[NSString class]])
        return stored;
    return [[NSLocale currentLocale] localeIdentifier];
}

static void StoreLocale(NSString *base, NSString *calendar)
{
    NSString *identifier = ([calendar length] == 0 || [calendar isEqualToString:@"gregorian"]) ? base : [NSString stringWithFormat:@"%@@calendar=%@", base, calendar];
    [PKPane setPreference:identifier forKey:kLocale domain:nil currentHost:NO];
}

@interface LocalizationPreferencePane : PKPane <NSTableViewDataSource, NSTableViewDelegate> {
    NSTableView *_languages;
    NSPopUpButton *_add;
    NSButton *_remove, *_up, *_down;
}
@end

@implementation LocalizationPreferencePane

- (NSArray<NSString *> *)languageList
{
    id stored = [PKPane preferenceForKey:kLanguages domain:nil currentHost:NO];
    // while the preference is not set, the languages the system falls back to are the list
    return [stored isKindOfClass:[NSArray class]] ? stored : [NSLocale preferredLanguages];
}

- (void)writeLanguages:(NSArray<NSString *> *)list
{
    [PKPane setPreference:([list count] != 0 ? list : nil) forKey:kLanguages domain:nil currentHost:NO];
    [self languagesChanged];
}

// A changed locale or language: formatters made from now on use it, and the preview is redrawn
- (void)languagesChanged
{
    [[NSNotificationCenter defaultCenter] postNotificationName:NSCurrentLocaleDidChangeNotification object:nil];
    [_languages reloadData];
    [self updateLanguageButtons];
    [self refresh];
}

- (NSView *)languageView
{
    NSView *view = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 600, 126)];
    NSScrollView *scroll = [[NSScrollView alloc] initWithFrame:NSMakeRect(0, 0, 360, 120)];
    _languages = [[NSTableView alloc] initWithFrame:NSMakeRect(0, 0, 340, 120)];
    NSTableColumn *column = [[NSTableColumn alloc] initWithIdentifier:@"language"];
    [column setWidth:340];
    [column setEditable:NO];
    [_languages addTableColumn:column];
    [_languages setHeaderView:nil];
    [_languages setDataSource:self];
    [_languages setDelegate:self];
    [_languages setToolTip:@"LocalizationLanguages"];
    [scroll setDocumentView:_languages];
    [scroll setHasVerticalScroller:YES];
    [scroll setBorderType:NSBezelBorder];
    [view addSubview:scroll];

    _add = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(374, 98, 150, 22) pullsDown:YES];
    [_add addItemWithTitle:@"Add Language"];
    for (NSString *code in KnownLanguages()) {
        [_add addItemWithTitle:[NSString stringWithFormat:@"%@ (%@)", LanguageName(code), code]];
        [[_add lastItem] setRepresentedObject:code];
    }
    [_add setTarget:self];
    [_add setAction:@selector(addLanguage:)];
    [_add setToolTip:@"LocalizationAddLanguage"];
    [view addSubview:_add];

    NSArray *titles = @[@"Remove", @"Move Up", @"Move Down"];
    NSArray *actions = @[@"removeLanguage:", @"moveLanguageUp:", @"moveLanguageDown:"];
    NSArray *tips = @[@"LocalizationRemoveLanguage", @"LocalizationLanguageUp", @"LocalizationLanguageDown"];
    NSMutableArray *buttons = [NSMutableArray array];
    for (int i = 0; i < 3; i++) {
        NSButton *button = [[NSButton alloc] initWithFrame:NSMakeRect(374, 68 - i * 30, 150, 24)];
        [button setBezelStyle:NSBezelStyleRounded];
        [button setTitle:titles[i]];
        [button setTarget:self];
        [button setAction:NSSelectorFromString(actions[i])];
        [button setToolTip:tips[i]];
        [view addSubview:button];
        [buttons addObject:button];
    }
    _remove = buttons[0];
    _up = buttons[1];
    _down = buttons[2];
    return view;
}

- (void)updateLanguageButtons
{
    NSInteger row = [_languages selectedRow], count = [[self languageList] count];
    [_remove setEnabled:row >= 0 && count > 1];
    [_up setEnabled:row > 0];
    [_down setEnabled:row >= 0 && row < count - 1];
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)table
{
    return [[self languageList] count];
}

- (id)tableView:(NSTableView *)table objectValueForTableColumn:(NSTableColumn *)column row:(NSInteger)row
{
    NSString *code = [self languageList][row];
    return [NSString stringWithFormat:@"%@ (%@)", LanguageName(code), code];
}

- (void)tableViewSelectionDidChange:(NSNotification *)notification
{
    [self updateLanguageButtons];
}

- (void)addLanguage:(NSPopUpButton *)sender
{
    NSString *code = [[sender selectedItem] representedObject];
    NSArray *list = [self languageList];
    if (code == nil || [list containsObject:code])
        return;
    [self writeLanguages:[list arrayByAddingObject:code]];
    [_languages selectRowIndexes:[NSIndexSet indexSetWithIndex:[list count]] byExtendingSelection:NO];
}

- (void)removeLanguage:(id)sender
{
    NSMutableArray *list = [[self languageList] mutableCopy];
    NSInteger row = [_languages selectedRow];
    if (row < 0 || [list count] < 2)
        return;
    [list removeObjectAtIndex:row];
    [self writeLanguages:list];
}

- (void)moveLanguage:(NSInteger)delta
{
    NSMutableArray *list = [[self languageList] mutableCopy];
    NSInteger row = [_languages selectedRow], target = row + delta;
    if (row < 0 || target < 0 || target >= (NSInteger)[list count])
        return;
    [list exchangeObjectAtIndex:row withObjectAtIndex:target];
    [self writeLanguages:list];
    [_languages selectRowIndexes:[NSIndexSet indexSetWithIndex:target] byExtendingSelection:NO];
}

- (void)moveLanguageUp:(id)sender { [self moveLanguage:-1]; }
- (void)moveLanguageDown:(id)sender { [self moveLanguage:1]; }

- (NSArray<PKItem *> *)items
{
    NSMutableArray *items = [NSMutableArray array];
    [items addObject:[PKItem heading:@"Preferred languages"]];
    PKItem *languages = [PKItem new];
    languages.kind = PKKindView;
    languages.view = [self languageView];
    [items addObject:languages];
    [items addObject:[PKItem note:@"Applications use the first of these languages that they are available in. Applications that are already open keep their language until they are opened again."]];

    [items addObject:[PKItem heading:@"Region"]];
    NSArray *regions = @[@"en_US", @"en_GB", @"en_AU", @"en_CA", @"de_DE", @"de_AT", @"de_CH", @"fr_FR", @"fr_CA", @"es_ES", @"es_MX", @"it_IT",
                         @"pt_BR", @"pt_PT", @"nl_NL", @"sv_SE", @"da_DK", @"nb_NO", @"fi_FI", @"cs_CZ", @"pl_PL", @"hu_HU", @"ru_RU", @"uk_UA",
                         @"tr_TR", @"el_GR", @"he_IL", @"ar_SA", @"ja_JP", @"ko_KR", @"zh_CN", @"zh_TW", @"en_CZ"];
    NSString *current = BaseLocale(StoredLocale());
    if (![regions containsObject:current])
        regions = [regions arrayByAddingObject:current];
    NSMutableArray *regionTitles = [NSMutableArray array];
    for (NSString *region in regions)
        [regionTitles addObject:[NSString stringWithFormat:@"%@ (%@)", LanguageName(region), region]];
    PKItem *region = [PKItem popup:@"LocalizationRegion" title:@"Region:" key:nil values:regions titles:regionTitles defaultValue:nil];
    region.readBlock = ^id{ return BaseLocale(StoredLocale()); };
    region.writeBlock = ^(id value) { if (value != nil) StoreLocale(value, CalendarOf(StoredLocale())); };
    [items addObject:region];

    NSArray *calendars = @[@"gregorian", @"buddhist", @"chinese", @"hebrew", @"indian", @"islamic", @"iso8601", @"japanese", @"persian", @"roc"];
    PKItem *calendar = [PKItem popup:@"LocalizationCalendar" title:@"Calendar:" key:nil values:calendars
                              titles:@[@"Gregorian", @"Buddhist", @"Chinese", @"Hebrew", @"Indian", @"Islamic", @"ISO 8601", @"Japanese", @"Persian", @"Republic of China"]
                        defaultValue:@"gregorian"];
    calendar.readBlock = ^id{ return CalendarOf(StoredLocale()); };
    calendar.writeBlock = ^(id value) { StoreLocale(BaseLocale(StoredLocale()), value != nil ? value : @"gregorian"); };
    [items addObject:calendar];

    PKItem *weekday = [PKItem popup:@"LocalizationFirstWeekday" title:@"First day of week:" key:nil
                             values:@[[NSNull null], @1, @2, @3, @4, @5, @6, @7]
                             titles:@[@"Default for the region", @"Sunday", @"Monday", @"Tuesday", @"Wednesday", @"Thursday", @"Friday", @"Saturday"]
                       defaultValue:nil];
    weekday.readBlock = ^id{
        id stored = [PKPane preferenceForKey:@"AppleFirstWeekday" domain:nil currentHost:NO];
        return [stored isKindOfClass:[NSDictionary class]] ? stored[@"gregorian"] : nil;
    };
    weekday.writeBlock = ^(id value) {
        NSMutableDictionary *dictionary = [NSMutableDictionary dictionary];
        id stored = [PKPane preferenceForKey:@"AppleFirstWeekday" domain:nil currentHost:NO];
        if ([stored isKindOfClass:[NSDictionary class]])
            [dictionary addEntriesFromDictionary:stored];
        if (value != nil)
            dictionary[@"gregorian"] = value;
        else
            [dictionary removeObjectForKey:@"gregorian"];
        [PKPane setPreference:([dictionary count] != 0 ? dictionary : nil) forKey:@"AppleFirstWeekday" domain:nil currentHost:NO];
    };
    [items addObject:weekday];

    [items addObject:[PKItem check:@"LocalizationTime24" title:@"24-Hour Time" key:@"AppleICUForce24HourTime" defaultValue:NO]];

    [items addObject:[PKItem popup:@"LocalizationTemperature" title:@"Temperature:" key:@"AppleTemperatureUnit"
                            values:@[[NSNull null], @"Celsius", @"Fahrenheit"]
                            titles:@[@"Default for the region", @"Celsius (°C)", @"Fahrenheit (°F)"] defaultValue:nil]];

    PKItem *units = [PKItem popup:@"LocalizationMeasurement" title:@"Measurement units:" key:nil
                           values:@[[NSNull null], @"Centimeters", @"Inches"]
                           titles:@[@"Default for the region", @"Metric", @"US"] defaultValue:nil];
    units.readBlock = ^id{ return [PKPane preferenceForKey:@"AppleMeasurementUnits" domain:nil currentHost:NO]; };
    units.writeBlock = ^(id value) {
        [PKPane setPreference:value forKey:@"AppleMeasurementUnits" domain:nil currentHost:NO];
        [PKPane setPreference:(value != nil ? @([value isEqual:@"Centimeters"]) : nil) forKey:@"AppleMetricUnits" domain:nil currentHost:NO];
    };
    [items addObject:units];

    [items addObject:[PKItem heading:@"Preview"]];
    [items addObject:[PKItem staticText:@"LocalizationPreviewDate" title:@"Date:" text:^NSString *{
        NSDateFormatter *f = [NSDateFormatter new];
        [f setLocale:[NSLocale currentLocale]];
        [f setDateStyle:NSDateFormatterFullStyle];
        return [f stringFromDate:[NSDate dateWithTimeIntervalSince1970:1700000000]];
    }]];
    [items addObject:[PKItem staticText:@"LocalizationPreviewTime" title:@"Time:" text:^NSString *{
        NSDateFormatter *f = [NSDateFormatter new];
        [f setLocale:[NSLocale currentLocale]];
        [f setTimeStyle:NSDateFormatterShortStyle];
        [f setTimeZone:[NSTimeZone timeZoneForSecondsFromGMT:0]];
        return [f stringFromDate:[NSDate dateWithTimeIntervalSince1970:1700000000]];
    }]];
    [items addObject:[PKItem staticText:@"LocalizationPreviewNumber" title:@"Number:" text:^NSString *{
        NSNumberFormatter *f = [NSNumberFormatter new];
        [f setLocale:[NSLocale currentLocale]];
        [f setNumberStyle:NSNumberFormatterDecimalStyle];
        [f setMaximumFractionDigits:2];
        [f setMinimumFractionDigits:2];
        return [f stringFromNumber:@1234567.89];
    }]];
    [items addObject:[PKItem staticText:@"LocalizationPreviewCurrency" title:@"Currency:" text:^NSString *{
        NSNumberFormatter *f = [NSNumberFormatter new];
        [f setLocale:[NSLocale currentLocale]];
        [f setNumberStyle:NSNumberFormatterCurrencyStyle];
        return [f stringFromNumber:@1234.56];
    }]];
    return items;
}

- (void)itemDidChange:(PKItem *)item toValue:(id)value
{
    [self languagesChanged];
}

- (void)willSelect
{
    [super willSelect];
    [_languages reloadData];
    [self updateLanguageButtons];
}

@end
