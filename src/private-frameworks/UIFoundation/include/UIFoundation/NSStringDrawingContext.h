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

#include <Foundation/Foundation.h>
#import <AppKit/AppKitIvarLayout.h>

// Legacy ObjC ABI (i386): instance size must stay 8 bytes, Apple's (macOS
// 10.14.4), because subclasses compiled against Apple's headers put their own
// ivars right after it. Any state added or changed here goes into the extra
// struct, never as an ivar; appkit-i386-ivar-layout checks the size.
@interface NSStringDrawingContext : NSObject {
    NS_EXTRA_IVARS_BEGIN(NSStringDrawingContext)
        CGFloat _minimumScaleFactor;
        CGFloat _minimumTrackingAdjustment;
        CGFloat _actualScaleFactor;
        CGFloat _actualTrackingAdjustment;
        NSRect _totalBounds;
    NS_EXTRA_IVARS_END(NSStringDrawingContext)
}

@property CGFloat minimumScaleFactor;
@property CGFloat minimumTrackingAdjustment;
@property(readonly) CGFloat actualScaleFactor;
@property(readonly) CGFloat actualTrackingAdjustment;
@property(readonly) NSRect totalBounds;

@end
