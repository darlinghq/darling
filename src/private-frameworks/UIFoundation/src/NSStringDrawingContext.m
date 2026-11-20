/*
 This file is part of Darling.

 Copyright (C) 2025 Darling Developers

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

#import <UIFoundation/NSStringDrawingContext.h>
#import <darling-stub-forwarding.h>

// The layout results are written by AppKit's string drawing, which reports
// them through the context.
@interface NSStringDrawingContext ()
@property CGFloat actualScaleFactor;
@property CGFloat actualTrackingAdjustment;
@property NSRect totalBounds;
@end

@implementation NSStringDrawingContext

NS_EXTRA_SYNTHESIZE_ASSIGN(NSStringDrawingContext, minimumScaleFactor, _minimumScaleFactor, CGFloat, minimumScaleFactor, setMinimumScaleFactor:)
NS_EXTRA_SYNTHESIZE_ASSIGN(NSStringDrawingContext, minimumTrackingAdjustment, _minimumTrackingAdjustment, CGFloat, minimumTrackingAdjustment, setMinimumTrackingAdjustment:)
NS_EXTRA_SYNTHESIZE_ASSIGN(NSStringDrawingContext, actualScaleFactor, _actualScaleFactor, CGFloat, actualScaleFactor, setActualScaleFactor:)
NS_EXTRA_SYNTHESIZE_ASSIGN(NSStringDrawingContext, actualTrackingAdjustment, _actualTrackingAdjustment, CGFloat, actualTrackingAdjustment, setActualTrackingAdjustment:)
NS_EXTRA_SYNTHESIZE_ASSIGN(NSStringDrawingContext, totalBounds, _totalBounds, NSRect, totalBounds, setTotalBounds:)
NS_EXTRA_DEALLOC(NSStringDrawingContext)

// Scaling and tracking adjustment are never applied by our string drawing, so
// the actual factors keep their identity values.
- (instancetype)init
{
    self = [super init];
    if (self != nil) {
        NS_EXTRA(NSStringDrawingContext, self)->_actualScaleFactor = 1.0;
    }
    return self;
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
