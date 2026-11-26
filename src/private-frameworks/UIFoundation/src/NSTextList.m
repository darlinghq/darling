/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSRaise.h>
#import <AppKit/NSTextList.h>
#import "UIFoundation-ExtraIvars.h"
#import <Foundation/NSDictionary.h>
#import <Foundation/NSString.h>
#import <dispatch/dispatch.h>

NSString *const NSTextListMarkerDecimal = @"{decimal}";
NSString *const NSTextListMarkerDisc = @"{disc}";
NSString *const NSTextListMarkerHyphen = @"{hyphen}";
// The strings below follow the "{keyword}" form of the three above (which
// were printed on macOS), with the keywords the marker format documentation
// names; they are inferred, not printed on macOS.
NSString *const NSTextListMarkerBox = @"{box}";
NSString *const NSTextListMarkerCheck = @"{check}";
NSString *const NSTextListMarkerCircle = @"{circle}";
NSString *const NSTextListMarkerDiamond = @"{diamond}";
NSString *const NSTextListMarkerSquare = @"{square}";
NSString *const NSTextListMarkerLowercaseHexadecimal = @"{lower-hexadecimal}";
NSString *const NSTextListMarkerUppercaseHexadecimal = @"{upper-hexadecimal}";
NSString *const NSTextListMarkerOctal = @"{octal}";
NSString *const NSTextListMarkerLowercaseAlpha = @"{lower-alpha}";
NSString *const NSTextListMarkerUppercaseAlpha = @"{upper-alpha}";
NSString *const NSTextListMarkerLowercaseLatin = @"{lower-latin}";
NSString *const NSTextListMarkerUppercaseLatin = @"{upper-latin}";
NSString *const NSTextListMarkerLowercaseRoman = @"{lower-roman}";
NSString *const NSTextListMarkerUppercaseRoman = @"{upper-roman}";

// Item numbers in letters, as in CSS's lower-alpha counter style: a ... z,
// aa ... az, ba ...
static NSString *alphabeticMarker(int number, unichar first) {
    unichar letters[16];
    int count = 0;
    while (number > 0 && count < 16) {
        number--;
        letters[15 - count++] = first + number % 26;
        number /= 26;
    }
    return [NSString stringWithCharacters:letters + 16 - count length:count];
}

// Item numbers in roman numerals, as in CSS's lower-roman counter style.
static NSString *romanMarker(int number, BOOL uppercase) {
    static const int values[] = {1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1};
    static const char *const lower[] = {"m", "cm", "d", "cd", "c", "xc", "l", "xl", "x", "ix", "v", "iv", "i"};
    static const char *const upper[] = {"M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX", "V", "IV", "I"};
    NSMutableString *result = [NSMutableString string];
    for (int i = 0; i < 13; i++) {
        while (number >= values[i]) {
            [result appendFormat:@"%s", uppercase ? upper[i] : lower[i]];
            number -= values[i];
        }
    }
    return result;
}

// The marker the keyword of a marker format gives for an item number, or nil
// for a keyword that is not one of the documented ones. Letters and roman
// numerals cover the numbers they can write (from 1, and up to 3999 for
// roman numerals); other numbers are written in decimal, as CSS falls back.
// The bullet characters are not documented and are inferred: the usual
// Unicode characters for each shape.
static NSString *markerForKeyword(NSString *keyword, int number) {
    static NSDictionary *bullets;
    static dispatch_once_t once;
    dispatch_once(&once, ^{
        bullets = [@{
            @"box": @"\u25AB",
            @"check": @"\u2713",
            @"circle": @"\u25E6",
            @"diamond": @"\u25C6",
            @"disc": @"\u2022",
            @"hyphen": @"\u2043",
            @"square": @"\u25AA",
        } retain];
    });
    NSString *bullet = bullets[keyword];
    if (bullet)
        return bullet;
    if ([keyword isEqualToString:@"decimal"])
        return [NSString stringWithFormat:@"%d", number];
    if ([keyword isEqualToString:@"octal"])
        return number >= 0 ? [NSString stringWithFormat:@"%o", number] : [NSString stringWithFormat:@"%d", number];
    if ([keyword isEqualToString:@"lower-hexadecimal"])
        return number >= 0 ? [NSString stringWithFormat:@"%x", number] : [NSString stringWithFormat:@"%d", number];
    if ([keyword isEqualToString:@"upper-hexadecimal"])
        return number >= 0 ? [NSString stringWithFormat:@"%X", number] : [NSString stringWithFormat:@"%d", number];

    BOOL lowerLetters = [keyword isEqualToString:@"lower-alpha"] || [keyword isEqualToString:@"lower-latin"];
    BOOL upperLetters = [keyword isEqualToString:@"upper-alpha"] || [keyword isEqualToString:@"upper-latin"];
    if (lowerLetters || upperLetters)
        return number >= 1 ? alphabeticMarker(number, lowerLetters ? 'a' : 'A') : [NSString stringWithFormat:@"%d", number];

    BOOL lowerRoman = [keyword isEqualToString:@"lower-roman"];
    if (lowerRoman || [keyword isEqualToString:@"upper-roman"])
        return number >= 1 && number <= 3999 ? romanMarker(number, !lowerRoman) : [NSString stringWithFormat:@"%d", number];
    return nil;
}

@implementation NSTextList

- initWithMarkerFormat: (NSString *) format options: (unsigned) options {
    _markerFormat = [format copy];
    _options = options;
    UIF_EXTRA_NSTextList(self)->_startingItemNumber = 1;
    return self;
}

- (void) dealloc {
    [_markerFormat release];
    [super dealloc];
}

- (NSString *) markerFormat {
    return _markerFormat;
}

- (unsigned) listOptions {
    return _options;
}

- (NSInteger) startingItemNumber {
    return UIF_EXTRA_NSTextList(self)->_startingItemNumber;
}

- (void) setStartingItemNumber: (NSInteger) number {
    UIF_EXTRA_NSTextList(self)->_startingItemNumber = number;
}

// The marker format with each "{keyword}" in it replaced by the item's
// marker; the rest of the format is kept as it is, as is a keyword that is
// not a marker one.
- (NSString *) markerForItemNumber: (int) index {
    NSString *format = _markerFormat ?: @"";
    NSMutableString *result = [NSMutableString string];
    NSUInteger position = 0, length = [format length];
    while (position < length) {
        NSRange open = [format rangeOfString:@"{" options:0 range:NSMakeRange(position, length - position)];
        if (open.location == NSNotFound)
            break;
        NSRange close = [format rangeOfString:@"}" options:0 range:NSMakeRange(open.location, length - open.location)];
        if (close.location == NSNotFound)
            break;
        NSString *keyword = [format substringWithRange:NSMakeRange(open.location + 1, close.location - open.location - 1)];
        NSString *marker = markerForKeyword(keyword, index);
        [result appendString:[format substringWithRange:NSMakeRange(position, open.location - position)]];
        [result appendString:marker ?: [format substringWithRange:NSMakeRange(open.location, close.location + 1 - open.location)]];
        position = close.location + 1;
    }
    [result appendString:[format substringFromIndex:position]];
    return result;
}

@end
