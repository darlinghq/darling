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

#import <CoreImage/CIDetector.h>
#import <darling-stub-forwarding.h>

NSString *const CIDetectorTracking = @"CIDetectorTracking";
NSString *const CIDetectorMinFeatureSize = @"CIDetectorMinFeatureSize";
NSString *const CIDetectorImageOrientation = @"CIDetectorImageOrientation";
NSString *const CIDetectorTypeRectangle = @"CIDetectorTypeRectangle";
NSString *const CIDetectorAccuracy = @"CIDetectorAccuracy";
NSString *const CIDetectorAccuracyHigh = @"CIDetectorAccuracyHigh";
NSString *const CIDetectorAccuracyLow = @"CIDetectorAccuracyLow";
NSString *const CIDetectorTypeFace = @"CIDetectorTypeFace";
NSString *const CIDetectorTypeQRCode = @"CIDetectorTypeQRCode";
NSString *const CIDetectorTypeText = @"CIDetectorTypeText";

// No detector finds anything yet: each type answers with an empty feature list, so callers take
// their nothing-detected path.
@implementation CIDetector

+ (CIDetector *)detectorOfType:(NSString *)type context:(CIContext *)context options:(NSDictionary *)options {
    (void)context;
    (void)options;
    if (![type isEqualToString:CIDetectorTypeFace] && ![type isEqualToString:CIDetectorTypeRectangle] &&
        ![type isEqualToString:CIDetectorTypeQRCode] && ![type isEqualToString:CIDetectorTypeText])
        return nil;
    return [[[self alloc] init] autorelease];
}

- (NSArray *)featuresInImage:(CIImage *)image {
    return [self featuresInImage:image options:nil];
}

- (NSArray *)featuresInImage:(CIImage *)image options:(NSDictionary *)options {
    (void)image;
    (void)options;
    return [NSArray array];
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
