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

extern NSString *const kCIInputAngleKey;
extern NSString *const kCIInputBackgroundImageKey;
extern NSString *const kCIInputBrightnessKey;
extern NSString *const kCIInputColorKey;
extern NSString *const kCIInputContrastKey;
extern NSString *const kCIInputExtentKey;
extern NSString *const kCIInputImageKey;
extern NSString *const kCIInputSaturationKey;
extern NSString *const kCIInputRadiusKey;
extern NSString *const kCIOutputImageKey;
extern NSString *const kCIApplyOptionDefinition;

extern NSString *const kCIAttributeClass;
extern NSString *const kCIAttributeDefault;
extern NSString *const kCIAttributeDisplayName;
extern NSString *const kCIAttributeFilterDisplayName;
extern NSString *const kCIAttributeFilterName;
extern NSString *const kCIAttributeMax;
extern NSString *const kCIAttributeMin;
extern NSString *const kCIAttributeSliderMin;
extern NSString *const kCIAttributeSliderMax;
extern NSString *const kCIAttributeType;

extern NSString *const kCIAttributeTypeAngle;
extern NSString *const kCIAttributeTypeBoolean;
extern NSString *const kCIAttributeTypeDistance;
extern NSString *const kCIAttributeTypeOffset;
extern NSString *const kCIAttributeTypePosition;
extern NSString *const kCIAttributeTypePosition3;
extern NSString *const kCIAttributeTypeRectangle;
extern NSString *const kCIAttributeTypeScalar;
extern NSString *const kCIAttributeTypeTime;

extern NSString *const kCICategoryCompositeOperation;
extern NSString *const kCICategoryGenerator;
extern NSString *const kCICategoryGradient;
extern NSString *const kCICategoryReduction;
extern NSString *const kCICategoryTransition;

@interface CIFilter : NSObject

@end
