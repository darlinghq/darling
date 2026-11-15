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

// Vision declarations: VNFaceLandmarkRegion2D derives from VNFaceLandmarkRegion, which has pointCount.
// The class implementations are forwarding stubs (instances come from face detection), so only the class
// graph is checked at run time; the property and method spellings are checked by compiling this file.
// Prints PASS when everything holds; the exit code is the number of failed checks.

#import <Foundation/Foundation.h>
#import <Vision/Vision.h>
#import <objc/runtime.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)

// Compile-time check of the declarations.
static NSUInteger sum_points(VNFaceLandmarkRegion2D *region, CGSize size)
{
    const CGPoint *normalized = region.normalizedPoints;
    const CGPoint *inImage = [region pointsInImageOfSize:size];
    NSUInteger n = region.pointCount;
    return n + (normalized != NULL) + (inImage != NULL);
}

int main(void)
{
    @autoreleasepool {
        CHECK([VNFaceLandmarkRegion class] != Nil);
        CHECK(class_getSuperclass([VNFaceLandmarkRegion2D class]) == [VNFaceLandmarkRegion class]);
        CHECK(sum_points != NULL);
    }
    if (!failures) printf("PASS\n");
    return failures;
}
