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

#ifndef _GCTypes_H_
#define _GCTypes_H_

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

struct GCPoint2 {
	float x;
	float y;
};
typedef struct GCPoint2 GCPoint2;

extern const GCPoint2 GCPoint2Zero;

static inline GCPoint2 GCPoint2Make(float x, float y)
{
	GCPoint2 p;
	p.x = x;
	p.y = y;
	return p;
}

static inline bool GCPoint2Equal(GCPoint2 point1, GCPoint2 point2)
{
	return point1.x == point2.x && point1.y == point2.y;
}

extern NSString *NSStringFromGCPoint2(GCPoint2 point);

@interface NSValue (GCTypes)
+ (instancetype)valueWithGCPoint2:(GCPoint2)point;
@property (nonatomic, readonly) GCPoint2 GCPoint2Value;
@end

NS_ASSUME_NONNULL_END

#endif
