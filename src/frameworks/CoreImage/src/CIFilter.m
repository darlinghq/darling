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
#import <CoreImage/CIColor.h>
#import <CoreImage/CIImage.h>
#import <CoreImage/CIVector.h>

#import <Foundation/NSString.h>
#import <objc/runtime.h>
#include <stdlib.h>
#include <string.h>
#import <darling-stub-forwarding.h>

NSString * const kCIInputAngleKey = @"inputAngle";
NSString * const kCIInputBackgroundImageKey = @"inputBackgroundImage";
NSString * const kCIInputBrightnessKey = @"inputBrightness";
NSString * const kCIInputColorKey = @"inputColor";
NSString * const kCIInputContrastKey = @"inputContrast";
NSString * const kCIInputEVKey = @"inputEV";
NSString * const kCIInputExtentKey = @"inputExtent";
NSString * const kCIInputImageKey = @"inputImage";
// The documentation gives no string value for this key. The value follows every other input key in
// this file ("input" plus the key name without the kCIInput prefix and Key suffix); macOS 26.7.3 and
// 10.14.4 report the same string (black-box observation). Test: tests/src/coreimage_input_intensity_key.m
NSString * const kCIInputIntensityKey = @"inputIntensity";
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

const CIFormat kCIFormatARGB8 = 265;
const CIFormat kCIFormatRGBA8 = 264;
const CIFormat kCIFormatBGRA8 = 266;
const CIFormat kCIFormatABGR8 = 267;
const CIFormat kCIFormatRGBAh = 2056;
const CIFormat kCIFormatRGBA16 = 1800;
const CIFormat kCIFormatRGBAf = 2312;

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

// An input parameter of a filter is an object instance variable whose name starts with "input", the
// convention the documentation gives for CIFilter subclasses.
static BOOL isInputParameter(Ivar ivar)
{
    const char *name = ivar_getName(ivar);
    const char *type = ivar_getTypeEncoding(ivar);
    return name != NULL && strncmp(name, "input", 5) == 0 && type != NULL && type[0] == '@' && type[1] != '?';
}

@implementation CIFilter

// Source: https://developer.apple.com/documentation/coreimage/cifilter-swift.class (the class conforms
// to NSSecureCoding and "automatically manages input parameters when archiving, copying, and
// deallocating filters"; subclasses keep them in instance variables prefixed with "input").
+ (BOOL) supportsSecureCoding {
    return YES;
}

- (NSArray<NSString *> *) inputKeys {
    NSMutableArray<Class> *chain = [NSMutableArray array];
    for (Class cls = [self class]; cls != Nil && cls != [CIFilter class]; cls = class_getSuperclass(cls)) {
        [chain insertObject: cls atIndex: 0];
    }
    NSMutableArray<NSString *> *keys = [NSMutableArray array];
    for (Class cls in chain) {
        unsigned int count = 0;
        Ivar *ivars = class_copyIvarList(cls, &count);
        for (unsigned int i = 0; i < count; i++) {
            if (isInputParameter(ivars[i])) {
                [keys addObject: [NSString stringWithUTF8String: ivar_getName(ivars[i])]];
            }
        }
        free(ivars);
    }
    return keys;
}

// QUIRK: each input parameter that has a value is archived under its name with the prefix "CI_" (for example
// "CI_inputEV"); a parameter without a value is not stored. The archive layout is not documented. Evidence:
// black-box observation of a keyed archive of a CIExposureAdjust on macOS 26.7.3 and 10.14.4 (tier 2).
// Test: coreimage_filter_coding ("archive keys").
static NSString *archiveKey(NSString *parameter) {
    return [@"CI_" stringByAppendingString: parameter];
}

- (void) encodeWithCoder: (NSCoder *) coder {
    for (NSString *key in [self inputKeys]) {
        id value = [self valueForKey: key];
        if (value != nil) {
            [coder encodeObject: value forKey: archiveKey(key)];
        }
    }
}

// Decoding applies no defaults, a parameter that is not in the archive stays without a value (as on macOS 10.14
// and 26: the default of a new filter is only applied by -setDefaults). The value classes are the ones the
// filter reference lists for parameters
// (https://developer.apple.com/library/archive/documentation/GraphicsImaging/Reference/CoreImageFilterReference/index.html);
// only a secure coder enforces them. A coder that is not keyed raises, as NSCoder does.
- (instancetype) initWithCoder: (NSCoder *) coder {
    if ((self = [super init])) {
        NSSet *valueClasses = [NSSet setWithObjects: [NSNumber class], [NSString class], [NSData class],
            [NSValue class], [NSAffineTransform class], [CIImage class], [CIVector class], [CIColor class], nil];
        for (NSString *key in [self inputKeys]) {
            id value = [coder decodeObjectOfClasses: valueClasses forKey: archiveKey(key)];
            if (value != nil) {
                [self setValue: value forKey: key];
            }
        }
    }
    return self;
}

// Documented: the CIFilter implementation clears the input parameters, so subclasses must not release
// their input instance variables.
- (void) dealloc {
    for (NSString *key in [self inputKeys]) {
        [self setValue: nil forKey: key];
    }
    [super dealloc];
}

/* No attributes and no input parameters, so there are no defaults to restore. Apple
   documents this as setting the default value of every input parameter, which with no
   parameters already holds. */
- (void) setDefaults {
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
