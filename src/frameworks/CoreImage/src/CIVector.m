/*
 This file is part of Darling.

 Copyright (C) 2017 Lubos Dolezel

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

#import <CoreImage/CIVector.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <xlocale.h>
#import <darling-stub-forwarding.h>

@implementation CIVector

+ (CIVector *) vectorWithValues: (const CGFloat *) values count: (size_t) count {
    return [[[self alloc] initWithValues: values count: count] autorelease];
}

+ (CIVector *) vectorWithX: (CGFloat) x {
    return [[[self alloc] initWithX: x] autorelease];
}

+ (CIVector *) vectorWithX: (CGFloat) x Y: (CGFloat) y {
    return [[[self alloc] initWithX: x Y: y] autorelease];
}

+ (CIVector *) vectorWithX: (CGFloat) x Y: (CGFloat) y Z: (CGFloat) z {
    return [[[self alloc] initWithX: x Y: y Z: z] autorelease];
}

+ (CIVector *) vectorWithX: (CGFloat) x
                         Y: (CGFloat) y
                         Z: (CGFloat) z
                         W: (CGFloat) w
{
    return [[[self alloc] initWithX: x Y: y Z: z W: w] autorelease];
}

+ (CIVector *) vectorWithString: (NSString *) representation {
    return [[[self alloc] initWithString: representation] autorelease];
}

+ (CIVector *) vectorWithCGPoint: (CGPoint) point {
    return [[[self alloc] initWithCGPoint: point] autorelease];
}

+ (CIVector *) vectorWithCGAffineTransform: (CGAffineTransform) transform {
    return [[[self alloc] initWithCGAffineTransform: transform] autorelease];
}

- (instancetype) initWithValues: (const CGFloat *) values count: (size_t) count {
    if ((values == NULL && count > 0) || count > SIZE_MAX / sizeof(CGFloat)) {
        [self release];
        [NSException raise: NSInvalidArgumentException format: @"CIVector: cannot hold %zu values taken from %p", count, values];
    }
    if ((self = [super init])) {
        // calloc(0, n) may return NULL, which -dealloc and -valueAtIndex: both tolerate.
        _values = calloc(count, sizeof(CGFloat));
        if (_values == NULL && count > 0) {
            [self release];
            [NSException raise: NSMallocException format: @"CIVector: no memory for %zu values", count];
        }
        _count = count;
        if (count > 0) {
            memcpy(_values, values, sizeof(CGFloat) * count);
        }
    }
    return self;
}

- (instancetype) initWithX: (CGFloat) x {
    return [self initWithValues: &x count: 1];
}

- (instancetype) initWithX: (CGFloat) x Y: (CGFloat) y {
    CGFloat values[2] = {x, y};
    return [self initWithValues: values count: 2];
}

- (instancetype) initWithX: (CGFloat) x Y: (CGFloat) y Z: (CGFloat) z {
    CGFloat values[3] = {x, y, z};
    return [self initWithValues: values count: 3];
}

- (instancetype) initWithX: (CGFloat) x Y: (CGFloat) y Z: (CGFloat) z W: (CGFloat) w {
    CGFloat values[4] = {x, y, z, w};
    return [self initWithValues: values count: 4];
}

- (instancetype) initWithCGPoint: (CGPoint) point {
    return [self initWithX: point.x Y: point.y];
}

- (instancetype) initWithCGAffineTransform: (CGAffineTransform) transform {
    CGFloat values[6] = {transform.a, transform.b, transform.c, transform.d, transform.tx, transform.ty};
    return [self initWithValues: values count: 6];
}

/* Reads and writes of numbers go through strtod_l with the "C" locale, so neither the process's
   locale nor Foundation's number scanner (which does not read every double back exactly) affects
   the result. */
static locale_t CIVectorCLocale(void) {
    locale_t locale = newlocale(LC_ALL_MASK, "C", NULL);

    if (locale == NULL) {
        [NSException raise: NSMallocException format: @"CIVector: no memory for the C locale"];
    }
    return locale;
}

static const char *CIVectorSkipSpace(const char *text) {
    while (isspace((unsigned char)*text)) {
        text++;
    }
    return text;
}

/* The documentation says the string is in one of the formats stringRepresentation returns and
   shows only bracketed, space separated lists such as "[10.0 23.0]". It says nothing about a
   string that is not in one of them, and the initializer returns "an initialized CIVector", so
   this gives an empty vector for one (GUESS). */
- (instancetype) initWithString: (NSString *) representation {
    NSMutableData *values = [NSMutableData data];
    const char *text = [representation UTF8String];
    BOOL valid = NO;

    if (text != NULL) {
        locale_t locale = CIVectorCLocale();
        const char *cursor = CIVectorSkipSpace(text);

        if (*cursor == '[') {
            valid = YES;
            cursor++;
            for (;;) {
                cursor = CIVectorSkipSpace(cursor);
                if (*cursor == ']') {
                    cursor++;
                    break;
                }

                char *end;
                double value = strtod_l(cursor, &end, locale);
                if (end == cursor) {
                    valid = NO;
                    break;
                }
                CGFloat component = value;
                [values appendBytes: &component length: sizeof(component)];
                cursor = end;
            }
            valid = valid && *CIVectorSkipSpace(cursor) == '\0';
        }
        freelocale(locale);
    }
    if (!valid) {
        [values setLength: 0];
    }
    return [self initWithValues: values.bytes count: values.length / sizeof(CGFloat)];
}

- (void) dealloc {
    free(_values);
    [super dealloc];
}

- (id) copyWithZone: (NSZone *) zone {
    return [self retain];
}

- (size_t) count {
    return _count;
}

- (CGFloat) valueAtIndex: (size_t) index {
    return index < _count ? _values[index] : 0.0;
}

- (CGFloat) X {
    return [self valueAtIndex: 0];
}

- (CGFloat) Y {
    return [self valueAtIndex: 1];
}

- (CGFloat) Z {
    return [self valueAtIndex: 2];
}

- (CGFloat) W {
    return [self valueAtIndex: 3];
}

- (CGPoint) CGPointValue {
    return CGPointMake([self X], [self Y]);
}

- (CGAffineTransform) CGAffineTransformValue {
    return CGAffineTransformMake([self valueAtIndex: 0], [self valueAtIndex: 1], [self valueAtIndex: 2],
                                 [self valueAtIndex: 3], [self valueAtIndex: 4], [self valueAtIndex: 5]);
}

/* The documentation gives the shape (square brackets, values separated by a space) and two
   examples that show a whole number with one decimal ("10.0"), and says a vector can be rebuilt
   from the string. How other values are written is not documented. GUESS: the shortest %g form
   that strtod_l reads back as the same double, so the round trip loses nothing, with ".0" added
   to a whole number. A value that is infinite or not a number is written as inf or nan (GUESS);
   the string reader above accepts those spellings through strtod_l. */
static NSString *CIVectorComponentText(double value, locale_t locale) {
    NSString *text = nil;

    for (int digits = 15; digits <= 17; digits++) {
        text = [NSString stringWithFormat: @"%.*g", digits, value];
        if (value != value || strtod_l([text UTF8String], NULL, locale) == value) {
            break;
        }
    }
    BOOL plainInteger = [text rangeOfCharacterFromSet: [NSCharacterSet characterSetWithCharactersInString: @".eEnN"]].location == NSNotFound;

    return plainInteger ? [text stringByAppendingString: @".0"] : text;
}

- (NSString *) stringRepresentation {
    NSMutableString *result = [NSMutableString stringWithString: @"["];
    locale_t locale = CIVectorCLocale();

    for (size_t i = 0; i < _count; i++) {
        if (i > 0) {
            [result appendString: @" "];
        }
        [result appendString: CIVectorComponentText(_values[i], locale)];
    }
    freelocale(locale);
    [result appendString: @"]"];
    return result;
}

/* The documentation lists no equality for CIVector. A vector with the same values in the same
   order is treated as equal (GUESS), as for the other small value classes. A NaN value is not
   equal to anything, so a vector holding one is equal only to itself (GUESS). */
- (BOOL) isEqual: (id) other {
    if (other == self) {
        return YES;
    }
    if (![other isKindOfClass: [CIVector class]] || [other count] != _count) {
        return NO;
    }
    for (size_t i = 0; i < _count; i++) {
        if ([other valueAtIndex: i] != _values[i]) {
            return NO;
        }
    }
    return YES;
}

- (NSUInteger) hash {
    NSUInteger result = _count;

    for (size_t i = 0; i < _count; i++) {
        /* 0.0 and -0.0 are equal, so they must hash alike. */
        result = result * 31 + (NSUInteger)[[NSNumber numberWithDouble: _values[i] == 0 ? 0.0 : (double)_values[i]] hash];
    }
    return result;
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation {
    DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation {
    DarlingStubForwardInvocation(self, anInvocation);
}

@end
