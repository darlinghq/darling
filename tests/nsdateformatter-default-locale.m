// Copyright (C) 2026 Lubos Dolezel
// A process that configured no locale of its own gets the system's AppleLocale and AppleLanguages
// (in a prefix, seeded from the host's locale when the prefix starts), not the root locale:
// the current locale and the preferred languages come from them, and the month name pattern gives
// the month name of that locale, not "M11".
#import <Foundation/Foundation.h>
#include <stdio.h>

int main(void)
{
    @autoreleasepool
    {
        int failures = 0;
        NSString *appleLocale = CFBridgingRelease(CFPreferencesCopyValue(CFSTR("AppleLocale"),
            kCFPreferencesAnyApplication, kCFPreferencesAnyUser, kCFPreferencesAnyHost));
        NSArray *appleLanguages = CFBridgingRelease(CFPreferencesCopyValue(CFSTR("AppleLanguages"),
            kCFPreferencesAnyApplication, kCFPreferencesAnyUser, kCFPreferencesAnyHost));

        if (![appleLocale isKindOfClass:[NSString class]] || [appleLocale length] == 0)
        {
            printf("FAIL: AppleLocale is not set in the global domain: %s\n", [[appleLocale description] UTF8String]);
            failures++;
        }
        if (![appleLanguages isKindOfClass:[NSArray class]] || [appleLanguages count] == 0
            || ![[appleLanguages objectAtIndex:0] isKindOfClass:[NSString class]])
        {
            printf("FAIL: AppleLanguages is not a non-empty array of strings: %s\n", [[appleLanguages description] UTF8String]);
            failures++;
        }

        NSString *identifier = [[NSLocale currentLocale] localeIdentifier];
        if ([identifier length] == 0)
        {
            printf("FAIL: the current locale has no identifier (root locale)\n");
            failures++;
        }
        else if ([appleLocale length] != 0 && ![identifier isEqualToString:appleLocale])
        {
            // A canonical form of the same locale (for example a region keyword) is fine as long as the language agrees
            NSString *language = [[NSLocale componentsFromLocaleIdentifier:appleLocale] objectForKey:NSLocaleLanguageCode];
            if (![[[NSLocale currentLocale] objectForKey:NSLocaleLanguageCode] isEqual:language])
            {
                printf("FAIL: current locale %s does not follow AppleLocale %s\n", [identifier UTF8String], [appleLocale UTF8String]);
                failures++;
            }
        }

        if ([appleLanguages count] != 0 && [[NSLocale preferredLanguages] count] == 0)
        {
            printf("FAIL: AppleLanguages is set but preferredLanguages is empty\n");
            failures++;
        }

        NSDate *november = [NSDate dateWithTimeIntervalSince1970:1763208000]; // 2025-11-15 12:00:00 UTC
        NSDateFormatter *formatter = [[[NSDateFormatter alloc] init] autorelease];
        [formatter setTimeZone:[NSTimeZone timeZoneForSecondsFromGMT:0]];
        [formatter setDateFormat:@"MMM"];
        NSString *month = [formatter stringFromDate:november];
        if ([month length] == 0 || [month isEqualToString:@"M11"])
        {
            printf("FAIL: abbreviated month of 15 November is \"%s\"\n", [month UTF8String]);
            failures++;
        }
        if ([appleLocale length] != 0)
        {
            // The month name must be what the seeded locale gives, not just any name
            NSLocale *seeded = [[[NSLocale alloc] initWithLocaleIdentifier:appleLocale] autorelease];
            NSDateFormatter *reference = [[[NSDateFormatter alloc] init] autorelease];
            [reference setLocale:seeded];
            [reference setTimeZone:[NSTimeZone timeZoneForSecondsFromGMT:0]];
            [reference setDateFormat:@"MMM"];
            NSString *expected = [reference stringFromDate:november];
            if (![month isEqualToString:expected])
            {
                printf("FAIL: month name \"%s\" differs from the one of AppleLocale %s (\"%s\")\n",
                    [month UTF8String], [appleLocale UTF8String], [expected UTF8String]);
                failures++;
            }
        }
        if (failures == 0)
            printf("PASS\n");
        return failures != 0;
    }
}
