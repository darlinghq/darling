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

#import <UIFoundation/NSTextEncapsulation.h>

static NSString *const NSTextEncapsulationScaleKey = @"NSTextEncapsulationScale";
static NSString *const NSTextEncapsulationShapeKey = @"NSTextEncapsulationShape";
static NSString *const NSTextEncapsulationStyleKey = @"NSTextEncapsulationStyle";
static NSString *const NSTextEncapsulationPlatterSizeKey = @"NSTextEncapsulationPlatterSize";
static NSString *const NSTextEncapsulationLineWeightKey = @"NSTextEncapsulationLineWeight";
static NSString *const NSTextEncapsulationColorKey = @"NSTextEncapsulationColor";
static NSString *const NSTextEncapsulationMinimumWidthKey = @"NSTextEncapsulationMinimumWidth";

@implementation NSTextEncapsulation

- (void)dealloc
{
    [_color release];
    [super dealloc];
}

- (id)copyWithZone:(NSZone *)zone
{
    NSTextEncapsulation *copy = [[[self class] allocWithZone:zone] init];
    if (copy != nil) {
        copy->_scale = _scale;
        copy->_shape = _shape;
        copy->_style = _style;
        copy->_platterSize = _platterSize;
        copy->_lineWeight = _lineWeight;
        copy->_color = [_color retain];
        copy->_minimumWidth = _minimumWidth;
    }
    return copy;
}

- (BOOL)isEqual:(id)object
{
    if (self == object)
        return YES;
    if (![object isKindOfClass:[NSTextEncapsulation class]])
        return NO;

    NSTextEncapsulation *other = (NSTextEncapsulation *)object;
    return _scale == other->_scale
        && _shape == other->_shape
        && _style == other->_style
        && _platterSize == other->_platterSize
        && _lineWeight == other->_lineWeight
        && _minimumWidth == other->_minimumWidth
        && (_color == other->_color || [_color isEqual:other->_color]);
}

- (NSUInteger)hash
{
    return (NSUInteger)_scale
        ^ ((NSUInteger)_shape << 2)
        ^ ((NSUInteger)_style << 4)
        ^ ((NSUInteger)_platterSize << 6)
        ^ [_color hash];
}

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super init];
    if (self != nil) {
        _scale = [coder decodeIntegerForKey:NSTextEncapsulationScaleKey];
        _shape = [coder decodeIntegerForKey:NSTextEncapsulationShapeKey];
        _style = [coder decodeIntegerForKey:NSTextEncapsulationStyleKey];
        _platterSize = [coder decodeIntegerForKey:NSTextEncapsulationPlatterSizeKey];
        _lineWeight = [coder decodeDoubleForKey:NSTextEncapsulationLineWeightKey];
        _color = [[coder decodeObjectForKey:NSTextEncapsulationColorKey] retain];
        _minimumWidth = [coder decodeDoubleForKey:NSTextEncapsulationMinimumWidthKey];
    }
    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeInteger:_scale forKey:NSTextEncapsulationScaleKey];
    [coder encodeInteger:_shape forKey:NSTextEncapsulationShapeKey];
    [coder encodeInteger:_style forKey:NSTextEncapsulationStyleKey];
    [coder encodeInteger:_platterSize forKey:NSTextEncapsulationPlatterSizeKey];
    [coder encodeDouble:_lineWeight forKey:NSTextEncapsulationLineWeightKey];
    if (_color != nil)
        [coder encodeObject:_color forKey:NSTextEncapsulationColorKey];
    [coder encodeDouble:_minimumWidth forKey:NSTextEncapsulationMinimumWidthKey];
}

@end
