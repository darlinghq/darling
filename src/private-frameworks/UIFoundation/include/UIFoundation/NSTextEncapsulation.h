/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

#ifndef _UIFoundation_NSTextEncapsulation_H_
#define _UIFoundation_NSTextEncapsulation_H_

#import <Foundation/Foundation.h>
#include <CoreGraphics/CoreGraphics.h>

FOUNDATION_EXPORT NSString *const NSTextEncapsulationAttributeName;

// The properties have the NSUInteger / CGFloat types the system class exposes; the system
// does not publish names for the values.
@interface NSTextEncapsulation : NSObject <NSCopying, NSSecureCoding> {
@private
    NSUInteger _scale;
    NSUInteger _shape;
    NSUInteger _style;
    NSUInteger _platterSize;
    CGFloat _lineWeight;
    id _color;
    CGFloat _minimumWidth;
}

- (instancetype)initWithShape:(NSUInteger)shape;

@property (nonatomic) NSUInteger scale;
@property (nonatomic) NSUInteger shape;
@property (nonatomic) NSUInteger style;
@property (nonatomic) NSUInteger platterSize;
@property (nonatomic) CGFloat lineWeight;
@property (nonatomic, copy) id color;
@property (nonatomic) CGFloat minimumWidth;

@end

#endif
