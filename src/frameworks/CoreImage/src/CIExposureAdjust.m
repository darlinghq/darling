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

@implementation CIExposureAdjust

// Default of inputEV is 0 and inputImage has none, which -[CIFilter setDefaults] leaves unchanged.
// Evidence: black-box observation of the attributes dictionary and of -setDefaults on macOS 26.7.3 and 10.14.4
// (tier 2; the old filter reference page says 0.5, which neither OS uses).
// Test: coreimage_exposure_adjust_coding
- (void) setDefaults {
    [self setValue: [NSNumber numberWithDouble: 0] forKey: kCIInputEVKey];
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
