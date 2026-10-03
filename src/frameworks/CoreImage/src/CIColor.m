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
        if (!cgColor) {
            _cgColor = NULL;
            return self;
        }

        CGColorSpaceRef srcSpace = CGColorGetColorSpace(cgColor);
        if (srcSpace && CGColorSpaceGetModel(srcSpace) == kCGColorSpaceModelRGB && CGColorGetNumberOfComponents(cgColor) == 4) {
            _cgColor = CGColorRetain(cgColor);
        } else {
            CGColorSpaceRef rgbSpace = CGColorSpaceCreateDeviceRGB();
            CGColorRef converted = CGColorCreateCopyByMatchingToColorSpace(rgbSpace, kCGRenderingIntentDefault, cgColor, NULL);
            CGColorSpaceRelease(rgbSpace);
            if (converted) {
                _cgColor = converted;
            } else {
                size_t count = CGColorGetNumberOfComponents(cgColor);
                const CGFloat *c = CGColorGetComponents(cgColor);
                CGFloat r = 0.0, g = 0.0, b = 0.0, a = 1.0;
                if (c && count > 0) {
                    if (count >= 5) {
                        // CMYK (+ Alpha)
                        r = (1.0 - c[0]) * (1.0 - c[3]);
                        g = (1.0 - c[1]) * (1.0 - c[3]);
                        b = (1.0 - c[2]) * (1.0 - c[3]);
                        a = c[count - 1];
                    } else if (count == 2) {
                        // Grayscale + Alpha
                        r = g = b = c[0];
                        a = c[1];
                    } else if (count == 1) {
                        // Grayscale
                        r = g = b = c[0];
                        a = 1.0;
                    } else {
                        r = c[0];
                        g = (count >= 2) ? c[1] : c[0];
                        b = (count >= 3) ? c[2] : c[0];
                        a = (count >= 4) ? c[count - 1] : 1.0;
                    }
                }
                _cgColor = CGColorCreateGenericRGB(r, g, b, a);
            }
        }
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
    const CGFloat *c = CGColorGetComponents(_cgColor);
    return c ? c[1] : 0.0;
}

- (CGFloat) blue {
    if (!_cgColor) return 0.0;
    const CGFloat *c = CGColorGetComponents(_cgColor);
    return c ? c[2] : 0.0;
}

- (CGFloat) alpha {
    if (!_cgColor) return 0.0;
    const CGFloat *c = CGColorGetComponents(_cgColor);
    return c ? c[3] : 0.0;
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation {
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end
