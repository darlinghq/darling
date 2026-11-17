/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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

#ifndef _HISERVICES_HISHAPE_H_
#define _HISERVICES_HISHAPE_H_

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef const struct __HIShape *HIShapeRef;

CFTypeID HIShapeGetTypeID(void);
HIShapeRef HIShapeCreateEmpty(void);
HIShapeRef HIShapeCreateWithRect(const CGRect *inRect);
HIShapeRef HIShapeCreateCopy(HIShapeRef inShape);
Boolean HIShapeIsEmpty(HIShapeRef inShape);
Boolean HIShapeIsRectangular(HIShapeRef inShape);
CGRect *HIShapeGetBounds(HIShapeRef inShape, CGRect *outRect);
Boolean HIShapeContainsPoint(HIShapeRef inShape, const CGPoint *inPoint);
Boolean HIShapeIntersectsRect(HIShapeRef inShape, const CGRect *inRect);

#ifdef __cplusplus
}
#endif

#endif
