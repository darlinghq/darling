/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

#ifndef _UIFoundation_NSTextGraphicsContext_H_
#define _UIFoundation_NSTextGraphicsContext_H_

#import <Foundation/Foundation.h>
#include <CoreGraphics/CoreGraphics.h>

@protocol NSTextGraphicsContext <NSObject>

@required
@property (readonly) CGContextRef CGContext;
@property (readonly, getter=isFlipped) BOOL flipped;
@property (readonly, getter=isDrawingToScreen) BOOL drawingToScreen;
+ (id<NSTextGraphicsContext>)graphicsContextForApplicationFrameworkContext:(id)context;

- (CGContextRef)CGContext;
- (BOOL)isFlipped;
- (BOOL)isDrawingToScreen;

@end

#endif
