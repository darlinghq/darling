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

#import <CoreImage/CIExposureAdjust.h>
#import <CoreImage/CIImage.h>
#import <darling-stub-forwarding.h>

@implementation CIExposureAdjust {
    // The input parameters (inputImage, inputEV) are instance variables named "input...", which is how
    // CIFilter finds them for archiving. https://developer.apple.com/documentation/coreimage/ciexposureadjust
    CIImage *inputImage;
    NSNumber *inputEV;
}

// The filter reference gives inputEV a default of 0.50 and no default for inputImage, which
// -[CIFilter setDefaults] documents as left unchanged.
// https://developer.apple.com/library/archive/documentation/GraphicsImaging/Reference/CoreImageFilterReference/index.html
- (void) setDefaults {
    [self setValue: [NSNumber numberWithDouble: 0.5] forKey: kCIInputEVKey];
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
