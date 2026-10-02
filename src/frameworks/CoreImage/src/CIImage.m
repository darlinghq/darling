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

#import <CoreImage/CIImage.h>

@implementation CIImage

+ (CIImage *) emptyImage {
    return [[[self alloc] initWithCGImage: NULL] autorelease];
}

- initWithCGImage: (CGImageRef) cgImage {
    _cgImage = CGImageRetain(cgImage);
    return self;
}

- (void) dealloc {
    CGImageRelease(_cgImage);
    [super dealloc];
}

/* An image with no CGImage behind it has no bounding box. CGRectZero is
   indistinguishable from a genuinely 0x0 image, so the infinite rect is used for "no
   bounds", which is Core Image's own idiom: its working space is "in theory infinite"
   in the Core Image Programming Guide. Apple never states the empty image's extent
   outright, so this one value is an inference. See the commit body. */
- (CGRect) extent {
    if (_cgImage == NULL) {
        return CGRectInfinite;
    }

    return CGRectMake(0, 0, CGImageGetWidth(_cgImage), CGImageGetHeight(_cgImage));
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation {
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end
