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
#include <string.h>
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

+ (CIVector *) vectorWithCGRect: (CGRect) rect {
    return [[[self alloc] initWithCGRect: rect] autorelease];
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

/* The four values are x, y, width and height (observed on macOS 26.7.3: a vector made from
   CGRectMake(1, 2, 3, 4) holds 1, 2, 3, 4 and gives that rectangle back). */
- (instancetype) initWithCGRect: (CGRect) rect {
    return [self initWithX: rect.origin.x Y: rect.origin.y Z: rect.size.width W: rect.size.height];
}

- (instancetype) initWithCGAffineTransform: (CGAffineTransform) transform {
    CGFloat values[6] = {transform.a, transform.b, transform.c, transform.d, transform.tx, transform.ty};
    return [self initWithValues: values count: 6];
}

/* Numbers are read through strtod_l with the "C" locale, so the process's locale does not
   affect the result. */
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

/* QUIRK: the documentation shows only well-formed strings such as "[10.0 23.0]" and does not say
   what any other string gives. Black-box observation of macOS 26.7.3 (tier 2): an opening
   bracket is optional and is recognised only as the very first character (leading white space
   before it makes the whole string unreadable); numbers separated by white space are then read
   until the first thing that is not a number, which ends the list without an error, so a
   closing bracket, trailing text or a comma simply stops it; a string with no number, and nil,
   give a vector with no values. No exception is raised and no nil is returned.
   Test: the "string reader" section of tests/coreimage-civector.m. */
- (instancetype) initWithString: (NSString *) representation {
    NSMutableData *values = [NSMutableData data];
    const char *text = [representation UTF8String];

    if (text != NULL) {
        locale_t locale = CIVectorCLocale();
        const char *cursor = text;

        if (*cursor == '[') {
            cursor++;
        }
        for (;;) {
            cursor = CIVectorSkipSpace(cursor);

            char *end;
            double value = strtod_l(cursor, &end, locale);
            if (end == cursor) {
                break;
            }
            CGFloat component = value;
            [values appendBytes: &component length: sizeof(component)];
            cursor = end;
        }
        freelocale(locale);
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

- (CGRect) CGRectValue {
    return CGRectMake([self valueAtIndex: 0], [self valueAtIndex: 1], [self valueAtIndex: 2],
                      [self valueAtIndex: 3]);
}

/* QUIRK: the documentation's examples show every value with a decimal point ("[1.0 0.5 0.3]",
   "[10.0 23.0]"). Black-box observation of macOS 26.7.3 (tier 2) gives "[1 0.5 0.3]" and
   "[10 23]": each value is written the way printf's %g writes it (six significant digits, so
   1234567.0 is "1.23457e+06" and 0.1 + 0.2 is "0.3"), separated by one space inside square
   brackets, with "nan", "inf" and "-0" spelt like that. A string made from this one therefore
   does not always give the same values back.
   Test: the "string representation" section of tests/coreimage-civector.m. */
- (NSString *) stringRepresentation {
    NSMutableString *result = [NSMutableString stringWithString: @"["];

    for (size_t i = 0; i < _count; i++) {
        if (i > 0) {
            [result appendString: @" "];
        }
        [result appendFormat: @"%g", (double)_values[i]];
    }
    [result appendString: @"]"];
    return result;
}

/* QUIRK: the documentation lists no equality for CIVector. Observed on macOS 26.7.3 (tier 2):
   two vectors are equal when they have the same number of values and the values are the same
   bit for bit, so 0.0 and -0.0 differ and a vector holding a NaN is equal to another with the
   same NaN. Test: the "equality" section of tests/coreimage-civector.m. */
- (BOOL) isEqual: (id) other {
    if (other == self) {
        return YES;
    }
    if (![other isKindOfClass: [CIVector class]] || [other count] != _count) {
        return NO;
    }
    for (size_t i = 0; i < _count; i++) {
        CGFloat theirs = [other valueAtIndex: i];

        if (memcmp(&theirs, &_values[i], sizeof(CGFloat)) != 0) {
            return NO;
        }
    }
    return YES;
}

- (NSUInteger) hash {
    NSUInteger result = _count;

    for (size_t i = 0; i < _count; i++) {
        uint64_t bits = 0;
        double value = _values[i];

        memcpy(&bits, &value, sizeof(bits));
        result = result * 31 + (NSUInteger)(bits ^ (bits >> 32));
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
