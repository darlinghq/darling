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

// The Date & Time pane (com.apple.preference.datetime). The clock and the time zone belong to the
// system that hosts the Darling prefix, so they are shown and not set here: the date and time come
// from NSDate, the time zone from NSTimeZone (which follows the system's /etc/localtime). The
// display of the clock is the preferences of the menu bar clock, com.apple.menuextra.clock:
//  - IsAnalog: an analogue clock instead of digits;
//  - ShowSeconds, ShowAMPM, ShowDayOfWeek, FlashDateSeparators: booleans;
//  - ShowDate: 0 shows the date when there is room, 1 always, 2 never;
// and AppleICUForce24HourTime of the global domain, which the Language & Region pane also sets.

static NSString * const kClockDomain = @"com.apple.menuextra.clock";

@interface DateTimePreferencePane : PKPane {
    NSTimer *_timer;
}
@end

@implementation DateTimePreferencePane

static PKItem *ClockCheck(NSString *identifier, NSString *title, NSString *key, BOOL defaultValue)
{
    PKItem *item = [PKItem check:identifier title:title key:key defaultValue:defaultValue];
    item.domain = kClockDomain;
    return item;
}

- (NSArray<PKItem *> *)items
{
    NSMutableArray *items = [NSMutableArray array];
    [items addObject:[PKItem heading:@"Date & Time"]];
    [items addObject:[PKItem staticText:@"DateTimeCurrent" title:@"Current date and time:" text:^NSString *{
        NSDateFormatter *f = [NSDateFormatter new];
        [f setLocale:[NSLocale currentLocale]];
        [f setDateStyle:NSDateFormatterFullStyle];
        [f setTimeStyle:NSDateFormatterMediumStyle];
        return [f stringFromDate:[NSDate date]];
    }]];
    [items addObject:[PKItem staticText:@"DateTimeZone" title:@"Time zone:" text:^NSString *{
        NSTimeZone *zone = [NSTimeZone systemTimeZone];
        NSInteger offset = [zone secondsFromGMT];
        NSString *abbreviation = [zone abbreviation];
        NSString *offsetText = [NSString stringWithFormat:@"GMT%+ld:%02ld", (long)(offset / 3600), (long)(labs(offset) % 3600 / 60)];
        return [NSString stringWithFormat:@"%@ (%@)", [zone name], abbreviation != nil ? [NSString stringWithFormat:@"%@, %@", offsetText, abbreviation] : offsetText];
    }]];
    [items addObject:[PKItem note:@"The date, time and time zone are those of the system that runs Darling; change them there."]];

    [items addObject:[PKItem heading:@"Clock"]];
    [items addObject:ClockCheck(@"DateTimeAnalog", @"Use an analog clock", @"IsAnalog", NO)];
    [items addObject:ClockCheck(@"DateTimeSeconds", @"Display the time with seconds", @"ShowSeconds", NO)];
    [items addObject:ClockCheck(@"DateTimeAMPM", @"Show AM/PM", @"ShowAMPM", YES)];
    [items addObject:ClockCheck(@"DateTimeDayOfWeek", @"Show the day of the week", @"ShowDayOfWeek", YES)];
    PKItem *date = [PKItem popup:@"DateTimeShowDate" title:@"Show the date:" key:@"ShowDate" values:@[@0, @1, @2]
                          titles:@[@"When space allows", @"Always", @"Never"] defaultValue:@0];
    date.domain = kClockDomain;
    [items addObject:date];
    [items addObject:ClockCheck(@"DateTimeFlash", @"Flash the time separators", @"FlashDateSeparators", NO)];
    PKItem *h24 = [PKItem check:@"DateTime24Hour" title:@"Use a 24-hour clock" key:@"AppleICUForce24HourTime" defaultValue:NO];
    [items addObject:h24];
    return items;
}

- (void)didSelect
{
    [super didSelect];
    [_timer invalidate];
    _timer = [NSTimer scheduledTimerWithTimeInterval:1 target:self selector:@selector(tick:) userInfo:nil repeats:YES];
}

- (void)willUnselect
{
    [_timer invalidate];
    _timer = nil;
    [super willUnselect];
}

- (void)tick:(NSTimer *)timer
{
    [self refreshStaticItems];
}

- (void)itemDidChange:(PKItem *)item toValue:(id)value
{
    if ([item.identifier isEqualToString:@"DateTime24Hour"])
        [[NSNotificationCenter defaultCenter] postNotificationName:NSCurrentLocaleDidChangeNotification object:nil];
}

- (void)handleSystemPreferencesAnchor:(NSString *)anchor
{
}

@end
