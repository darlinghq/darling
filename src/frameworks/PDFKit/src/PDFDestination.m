/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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

#import <PDFKit/PDFDestination.h>
#import <darling-stub-forwarding.h>
#include <float.h>
#import <objc/runtime.h>

// The documentation says only that this is the value used when a destination's x or y is
// unimportant, and does not state it. macOS 26.7.3 and 10.14.4 report FLT_MAX stored as a CGFloat
// (not CGFLOAT_MAX) (black-box observation). Test: tests/src/pdfkit_constants.m
const CGFloat kPDFDestinationUnspecifiedValue = FLT_MAX;

@implementation PDFDestination

// The page is a weak reference in PDFKit's interface, so the destination does not own its page and the reference is
// cleared when the page is freed (observed on macOS 26.7.3 and 10.14.4: the page's reference count is not raised and
// page is nil after the page is released). The runtime's weak reference functions are used, as the instance variable
// is not compiled as a __weak one.
// Observed on both: initialising without a page returns nil, and a plain -init gives a destination whose point is
// unspecified in both coordinates.
- (instancetype)init
{
    if ((self = [super init]))
        _point = CGPointMake(kPDFDestinationUnspecifiedValue, kPDFDestinationUnspecifiedValue);
    return self;
}

- (instancetype)initWithPage:(PDFPage*)page atPoint:(CGPoint)point
{
    if (page == nil)
    {
        [self release];
        return nil;
    }
    if ((self = [super init]))
    {
        objc_storeWeak(&_page, page);
        _point = point;
    }
    return self;
}

- (void)dealloc
{
    objc_storeWeak(&_page, nil);
    [super dealloc];
}

- (PDFPage*)page
{
    return objc_loadWeak(&_page);
}

- (CGPoint)point
{
    return _point;
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

@end
