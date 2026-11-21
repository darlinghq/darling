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

#include <Foundation/Foundation.h>
#include <CoreGraphics/CoreGraphics.h>

@interface CIVector : NSObject <NSCopying> {
    /* Same names and order as the ivars of <QuartzCore/CIVector.h> from cocotron. */
    size_t _count;
    CGFloat *_values;
}

+ (CIVector *) vectorWithValues: (const CGFloat *) values count: (size_t) count;
+ (CIVector *) vectorWithX: (CGFloat) x;
+ (CIVector *) vectorWithX: (CGFloat) x Y: (CGFloat) y;
+ (CIVector *) vectorWithX: (CGFloat) x Y: (CGFloat) y Z: (CGFloat) z;
+ (CIVector *) vectorWithX: (CGFloat) x
                         Y: (CGFloat) y
                         Z: (CGFloat) z
                         W: (CGFloat) w;
+ (CIVector *) vectorWithString: (NSString *) representation;
+ (CIVector *) vectorWithCGPoint: (CGPoint) point;
+ (CIVector *) vectorWithCGRect: (CGRect) rect;
+ (CIVector *) vectorWithCGAffineTransform: (CGAffineTransform) transform;

- (instancetype) initWithValues: (const CGFloat *) values count: (size_t) count;
- (instancetype) initWithX: (CGFloat) x;
- (instancetype) initWithX: (CGFloat) x Y: (CGFloat) y;
- (instancetype) initWithX: (CGFloat) x Y: (CGFloat) y Z: (CGFloat) z;
- (instancetype) initWithX: (CGFloat) x Y: (CGFloat) y Z: (CGFloat) z W: (CGFloat) w;
- (instancetype) initWithString: (NSString *) representation;
- (instancetype) initWithCGPoint: (CGPoint) point;
- (instancetype) initWithCGRect: (CGRect) rect;
- (instancetype) initWithCGAffineTransform: (CGAffineTransform) transform;

- (CGFloat) valueAtIndex: (size_t) index;
- (size_t) count;

- (CGFloat) X;
- (CGFloat) Y;
- (CGFloat) Z;
- (CGFloat) W;

- (CGPoint) CGPointValue;
- (CGRect) CGRectValue;
- (CGAffineTransform) CGAffineTransformValue;

- (NSString *) stringRepresentation;

@end
