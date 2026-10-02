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
#import <CoreImage/CIImage.h>

#import <Foundation/NSString.h>

NSString * const kCIInputAngleKey = @"inputAngle";
NSString * const kCIInputBackgroundImageKey = @"inputBackgroundImage";
NSString * const kCIInputBrightnessKey = @"inputBrightness";
NSString * const kCIInputColorKey = @"inputColor";
NSString * const kCIInputContrastKey = @"inputContrast";
NSString * const kCIInputExtentKey = @"inputExtent";
NSString * const kCIInputImageKey = @"inputImage";
NSString * const kCIInputSaturationKey = @"inputSaturation";
NSString *const kCIInputRadiusKey = @"inputRadius";
NSString * const kCIOutputImageKey = @"outputImage";
NSString * const kCIApplyOptionDefinition = @"definition";

NSString * const kCIAttributeClass = @"CIAttributeClass";
NSString * const kCIAttributeDefault = @"CIAttributeDefault";
NSString * const kCIAttributeDisplayName = @"CIAttributeDisplayName";
NSString * const kCIAttributeFilterDisplayName = @"CIAttributeFilterDisplayName";
NSString * const kCIAttributeFilterName = @"CIAttributeFilterName";
NSString * const kCIAttributeMax = @"CIAttributeMax";
NSString * const kCIAttributeMin = @"CIAttributeMin";
NSString * const kCIAttributeSliderMin = @"CIAttributeSliderMin";
NSString * const kCIAttributeSliderMax = @"CIAttributeSliderMax";
NSString * const kCIAttributeType = @"CIAttributeType";

const CIFormat kCIFormatARGB8 = 0;
const CIFormat kCIFormatRGBA8 = 24;
const CIFormat kCIFormatBGRA8 = 27;
const CIFormat kCIFormatABGR8 = 28;
const CIFormat kCIFormatRGBAh = 31;
const CIFormat kCIFormatRGBA16 = 33;
const CIFormat kCIFormatRGBAf = 34;

NSString * const kCIAttributeTypeAngle = @"CIAttributeTypeAngle";
NSString * const kCIAttributeTypeBoolean = @"CIAttributeTypeBoolean";
NSString * const kCIAttributeTypeDistance = @"CIAttributeTypeDistance";
NSString * const kCIAttributeTypeOffset = @"CIAttributeTypeOffset";
NSString * const kCIAttributeTypePosition = @"CIAttributeTypePosition";
NSString * const kCIAttributeTypePosition3 = @"CIAttributeTypePosition3";
NSString * const kCIAttributeTypeRectangle = @"CIAttributeTypeRectangle";
NSString * const kCIAttributeTypeScalar = @"CIAttributeTypeScalar";
NSString * const kCIAttributeTypeTime = @"CIAttributeTypeTime";

NSString * const kCICategoryCompositeOperation = @"CICategoryCompositeOperation";
NSString * const kCICategoryGenerator = @"CICategoryGenerator";
NSString * const kCICategoryGradient = @"CICategoryGradient";
NSString * const kCICategoryReduction = @"CICategoryReduction";
NSString * const kCICategoryTransition = @"CICategoryTransition";

@implementation CIFilter

/* No attributes and no input parameters, so there are no defaults to restore. Apple
   documents this as setting the default value of every input parameter, which with no
   parameters already holds. */
- (void) setDefaults {
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation {
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end
