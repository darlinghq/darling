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

#import <CoreImage/CIColor.h>

@implementation CIColor

+ (CIColor *) colorWithCGColor: (CGColorRef) cgColor {
    return [[[self alloc] initWithCGColor: cgColor] autorelease];
}

+ (CIColor *) colorWithRed: (CGFloat) red
                     green: (CGFloat) green
                      blue: (CGFloat) blue
{
    return [self colorWithRed: red green: green blue: blue alpha: 1.0];
}

+ (CIColor *) colorWithRed: (CGFloat) red
                     green: (CGFloat) green
                      blue: (CGFloat) blue
                     alpha: (CGFloat) alpha
{
    CGColorRef cgColor = CGColorCreateGenericRGB(red, green, blue, alpha);
    CIColor *result = [self colorWithCGColor: cgColor];

    CGColorRelease(cgColor);

    return result;
}

- (instancetype) initWithCGColor: (CGColorRef) cgColor {
    if ((self = [super init])) {
        _cgColor = CGColorRetain(cgColor);
    }
    return self;
}

- (void) dealloc {
    CGColorRelease(_cgColor);
    [super dealloc];
}

- (size_t) numberOfComponents {
    return _cgColor ? CGColorGetNumberOfComponents(_cgColor) : 0;
}

- (CGColorSpaceRef) colorSpace {
    return _cgColor ? CGColorGetColorSpace(_cgColor) : NULL;
}

- (const CGFloat *) components {
    return _cgColor ? CGColorGetComponents(_cgColor) : NULL;
}

- (CGFloat) red {
    if (!_cgColor) return 0.0;
    const CGFloat *c = CGColorGetComponents(_cgColor);
    return c ? c[0] : 0.0;
}

- (CGFloat) green {
    if (!_cgColor) return 0.0;
    size_t count = CGColorGetNumberOfComponents(_cgColor);
    const CGFloat *c = CGColorGetComponents(_cgColor);
    if (!c) return 0.0;
    if (count >= 3) return c[1];
    if (count == 2) return c[0]; // grayscale: r = g = b = gray
    return c[0];
}

- (CGFloat) blue {
    if (!_cgColor) return 0.0;
    size_t count = CGColorGetNumberOfComponents(_cgColor);
    const CGFloat *c = CGColorGetComponents(_cgColor);
    if (!c) return 0.0;
    if (count >= 3) return c[2];
    if (count == 2) return c[0]; // grayscale: r = g = b = gray
    return c[0];
}

- (CGFloat) alpha {
    if (!_cgColor) return 0.0;
    size_t count = CGColorGetNumberOfComponents(_cgColor);
    const CGFloat *c = CGColorGetComponents(_cgColor);
    if (!c) return 0.0;
    if (count >= 4) return c[3];
    if (count == 2) return c[1]; // grayscale: gray, alpha
    return 1.0;
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation {
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end
