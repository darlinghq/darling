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

typedef NS_ENUM(NSUInteger, NSTextEncapsulationScale) {
    NSTextEncapsulationScaleMedium = 0,
    NSTextEncapsulationScaleSmall = 1,
    NSTextEncapsulationScaleLarge = 2,
};

typedef NS_ENUM(NSUInteger, NSTextEncapsulationShape) {
    NSTextEncapsulationShapeRoundedRectangle = 0,
    NSTextEncapsulationShapeRectangle = 1,
    NSTextEncapsulationShapeCapsule = 2,
};

typedef NS_ENUM(NSUInteger, NSTextEncapsulationStyle) {
    NSTextEncapsulationStyleOutline = 0,
    NSTextEncapsulationStyleFill = 1,
};

typedef NS_ENUM(NSUInteger, NSTextEncapsulationPlatterSize) {
    NSTextEncapsulationPlatterSizeRegular = 0,
    NSTextEncapsulationPlatterSizeLarge = 1,
};

FOUNDATION_EXPORT NSString *const NSTextEncapsulationAttributeName;

@interface NSTextEncapsulation : NSObject <NSCopying, NSSecureCoding>

@property (nonatomic) NSTextEncapsulationScale scale;
@property (nonatomic) NSTextEncapsulationShape shape;
@property (nonatomic) NSTextEncapsulationStyle style;
@property (nonatomic) NSTextEncapsulationPlatterSize platterSize;
@property (nonatomic) CGFloat lineWeight;
@property (nonatomic, retain) NSObject *color;
@property (nonatomic) CGFloat minimumWidth;

@end

#endif
