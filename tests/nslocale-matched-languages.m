// Guest test: +[NSLocale matchedLanguagesFromAvailableLanguages:forPreferredLanguages:],
// which WebKit uses to choose a language. On macOS 26.7.3 (x86_64) the method
// comes from InternationalSupport, which UIFoundation links, so it exists in
// any process that links AppKit; this test links Cocoa for that reason.
// Expected values were observed on macOS with the same inputs: one best match
// per preferred language, no fallback when nothing shares the language code.
#import <Cocoa/Cocoa.h>
#include <stdio.h>

@interface NSLocale (InternationalSupportSPI)
+ (NSArray *)matchedLanguagesFromAvailableLanguages:(NSArray *)available forPreferredLanguages:(NSArray *)preferred;
@end

static int failures;

static void expect(NSArray *available, NSArray *preferred, NSArray *expected)
{
    NSArray *result = [NSLocale matchedLanguagesFromAvailableLanguages: available forPreferredLanguages: preferred];
    BOOL ok = [result isEqualToArray: expected];
    printf("%s: (%s) in (%s) -> (%s)%s%s%s\n", ok ? "PASS" : "FAIL",
           [[preferred componentsJoinedByString: @","] UTF8String], [[available componentsJoinedByString: @","] UTF8String],
           [[expected componentsJoinedByString: @","] UTF8String], ok ? "" : " [got ",
           ok ? "" : [[result componentsJoinedByString: @","] UTF8String], ok ? "" : "]");
    if (!ok)
        failures++;
}

int main(void)
{
    @autoreleasepool {
        if (![NSLocale respondsToSelector: @selector(matchedLanguagesFromAvailableLanguages:forPreferredLanguages:)]) {
            printf("FAIL: the method is missing in a process that links AppKit\n");
            return 1;
        }
        expect(@[ @"en", @"fr", @"de" ], @[ @"fr" ], @[ @"fr" ]);
        expect(@[ @"en", @"fr", @"de" ], @[ @"fr-CA" ], @[ @"fr" ]);
        expect(@[ @"en", @"fr-CA", @"fr" ], @[ @"fr-CA" ], @[ @"fr-CA" ]);
        expect(@[ @"en-US", @"en-GB" ], @[ @"en-AU" ], @[ @"en-GB" ]);
        expect(@[ @"en", @"fr" ], @[ @"ja" ], @[]);
        expect(@[], @[ @"en" ], @[]);
        expect(@[ @"zh-Hans", @"zh-Hant" ], @[ @"zh-TW" ], @[ @"zh-Hant" ]);
        expect(@[ @"pt-BR", @"pt-PT" ], @[ @"pt" ], @[ @"pt-BR" ]);
        expect(@[ @"en", @"es" ], @[ @"es-MX" ], @[ @"es" ]);
        expect(@[ @"EN", @"fr" ], @[ @"en" ], @[ @"EN" ]);
        expect(@[ @"en_US", @"fr" ], @[ @"en-US" ], @[ @"en_US" ]);
        expect(@[ @"en", @"fr", @"de" ], @[ @"de", @"fr" ], @[ @"de", @"fr" ]);
        expect(@[ @"en", @"fr" ], @[ @"ja", @"fr" ], @[ @"fr" ]);
        expect(@[ @"en", @"fr" ], @[], @[]);
    }
    return failures != 0;
}
