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

#import <Foundation/Foundation.h>
#import <Vision/VNFaceLandmarkRegion.h>

NS_ASSUME_NONNULL_BEGIN

// The points of a region in two dimensions. The Swift overlay wraps the two
// pointer accessors (refined for Swift) into array-returning ones.
API_AVAILABLE(macos(10.13))
@interface VNFaceLandmarkRegion2D : VNFaceLandmarkRegion

// pointCount points, normalized to the face bounding box (each coordinate 0 to 1).
@property (readonly, assign) const CGPoint *normalizedPoints NS_REFINED_FOR_SWIFT;

// The same points converted to a coordinate system of the given image size. Swift sees it under the
// double-underscore name the overlay wraps.
- (const CGPoint *)pointsInImageOfSize:(CGSize)imageSize NS_SWIFT_NAME(__pointsInImage(imageSize:));

@end

NS_ASSUME_NONNULL_END
