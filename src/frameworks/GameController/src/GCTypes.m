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

#import <GameController/GCTypes.h>

const GCPoint2 GCPoint2Zero = { 0.0f, 0.0f };

NSString *NSStringFromGCPoint2(GCPoint2 point)
{
	return [NSString stringWithFormat:@"{%g, %g}", (double)point.x, (double)point.y];
}

@implementation NSValue (GCTypes)

+ (instancetype)valueWithGCPoint2:(GCPoint2)point
{
	return [NSValue valueWithBytes:&point objCType:@encode(GCPoint2)];
}

- (GCPoint2)GCPoint2Value
{
	GCPoint2 point = GCPoint2Zero;
	[self getValue:&point];
	return point;
}

@end
