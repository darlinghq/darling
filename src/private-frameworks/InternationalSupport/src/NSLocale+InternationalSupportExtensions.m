/*
 This file is part of Darling.

 Copyright (C) 2022 Darling Team

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

#import <InternationalSupport/NSLocale+InternationalSupportExtensions.h>

@implementation NSLocale (InternationalSupportExtensions)

+ (nonnull NSArray<NSString *> *)minimizedLanguagesFromLanguages:(nonnull NSArray<NSString *> *)languages {
    NSLog(@"Stub called: minimizedLanguagesFromLanguages in NSLocale (InternationalSupportExtensions)");
    return languages;
}

// The language code of a language identifier ("zh" for "zh-Hant", "en" for "en_US"), lowercased.
static NSString *languageCode(NSString *language)
{
    NSRange separator = [language rangeOfCharacterFromSet: [NSCharacterSet characterSetWithCharactersInString: @"-_"]];
    NSString *code = separator.location == NSNotFound ? language : [language substringToIndex: separator.location];
    return [code lowercaseString];
}

// For each preferred language in turn, the available language that suits it best, as CFBundle
// would choose a localization for it; a preferred language that no available language shares a
// language code with matches nothing, rather than falling back as CFBundle does. A language is
// listed once. Observed on macOS 26.7.3: "fr-CA" in (en, fr-CA, fr) gives (fr-CA), "en-AU" in
// (en-US, en-GB) gives (en-GB), "ja" in (en, fr) gives an empty array, and (de, fr) in
// (en, fr, de) gives (de, fr). Test: tests/nslocale-matched-languages.m.
+ (nonnull NSArray<NSString *> *)matchedLanguagesFromAvailableLanguages:(nonnull NSArray<NSString *> *)availableLanguages forPreferredLanguages:(nonnull NSArray<NSString *> *)preferredLanguages
{
    NSMutableArray<NSString *> *matched = [NSMutableArray array];
    if ([availableLanguages count] == 0)
        return matched;

    for (NSString *preferred in preferredLanguages) {
        CFArrayRef localizations = CFBundleCopyLocalizationsForPreferences((CFArrayRef)availableLanguages, (CFArrayRef)@[ preferred ]);
        NSString *best = localizations != NULL && CFArrayGetCount(localizations) > 0 ? (NSString *)CFArrayGetValueAtIndex(localizations, 0) : nil;
        if (best != nil && [languageCode(best) isEqualToString: languageCode(preferred)] && ![matched containsObject: best])
            [matched addObject: best];
        if (localizations != NULL)
            CFRelease(localizations);
    }
    return matched;
}

@end
