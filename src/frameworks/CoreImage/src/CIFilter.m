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

#import <CoreImage/CIFilter.h>

#import <Foundation/NSString.h>

NSString * const kCIInputAngleKey = @"inputAngle";
NSString * const kCIInputBackgroundImageKey = @"inputBackgroundImage";
NSString * const kCIInputBrightnessKey = @"inputBrightness";
NSString * const kCIInputColorKey = @"inputColor";
NSString * const kCIInputContrastKey = @"inputContrast";
NSString * const kCIInputExtentKey = @"inputExtent";
NSString * const kCIInputImageKey = @"inputImage";
NSString * const kCIInputSaturationKey = @"inputSaturation";
NSString * const kCIOutputImageKey = @"outputImage";
NSString * const kCIApplyOptionDefinition = @"definition";

NSString * const kCIAttributeClass = @"class";
NSString * const kCIAttributeDisplayName = @"displayName";
NSString * const kCIAttributeFilterDisplayName = @"filterDisplayName";
NSString * const kCIAttributeFilterName = @"filterName";
NSString * const kCIAttributeMax = @"max";
NSString * const kCIAttributeMin = @"min";
NSString * const kCIAttributeType = @"type";

NSString * const kCIAttributeTypeAngle = @"angle";
NSString * const kCIAttributeTypeBoolean = @"boolean";
NSString * const kCIAttributeTypeDistance = @"distance";
NSString * const kCIAttributeTypeOffset = @"offset";
NSString * const kCIAttributeTypePosition = @"position";
NSString * const kCIAttributeTypePosition3 = @"position3";
NSString * const kCIAttributeTypeRectangle = @"rectangle";
NSString * const kCIAttributeTypeScalar = @"scalar";
NSString * const kCIAttributeTypeTime = @"time";

NSString * const kCICategoryCompositeOperation = @"kCICategoryCompositeOperation";
NSString * const kCICategoryGenerator = @"kCICategoryGenerator";
NSString * const kCICategoryGradient = @"kCICategoryGradient";
NSString * const kCICategoryReduction = @"kCICategoryReduction";
NSString * const kCICategoryTransition = @"kCICategoryTransition";

@implementation CIFilter

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation {
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end
