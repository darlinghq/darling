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

#ifndef _SCENEKIT_SCENEKITTYPES_H_
#define _SCENEKIT_SCENEKITTYPES_H_

#include <stdbool.h>
#import <CoreGraphics/CGBase.h>
#import <QuartzCore/CATransform3D.h>

#ifdef __cplusplus
extern "C" {
#endif

// Public documentation lists SCNMatrix4 as a typealias on macOS, i.e. the Core Animation
// matrix: https://developer.apple.com/documentation/scenekit/scnmatrix4-swift.typealias
typedef CATransform3D SCNMatrix4;

// Fields are CGFloat on macOS (float on iOS):
// https://developer.apple.com/documentation/scenekit/scnvector3
typedef struct SCNVector3 {
    CGFloat x, y, z;
} SCNVector3;

// https://developer.apple.com/documentation/scenekit/scnvector4
typedef struct SCNVector4 {
    CGFloat x, y, z, w;
} SCNVector4;

extern const SCNMatrix4 SCNMatrix4Identity;
extern const SCNVector3 SCNVector3Zero;
extern const SCNVector4 SCNVector4Zero;

// The creation helpers of the public header are inline functions (they are not symbols of the framework).
static inline SCNVector3 SCNVector3Make(CGFloat x, CGFloat y, CGFloat z)
{
    SCNVector3 v = { x, y, z };
    return v;
}

static inline SCNVector4 SCNVector4Make(CGFloat x, CGFloat y, CGFloat z, CGFloat w)
{
    SCNVector4 v = { x, y, z, w };
    return v;
}

// SCNMatrix4 is the Core Animation matrix on macOS. MakeTranslation and MakeScale are the CATransform3D ones.
static inline SCNMatrix4 SCNMatrix4MakeTranslation(CGFloat tx, CGFloat ty, CGFloat tz)
{
    return CATransform3DMakeTranslation(tx, ty, tz);
}

static inline SCNMatrix4 SCNMatrix4MakeScale(CGFloat sx, CGFloat sy, CGFloat sz)
{
    return CATransform3DMakeScale(sx, sy, sz);
}

// Not CATransform3DTranslate: the translation follows m (m, then the translation), which is not the order of the
// Core Animation function (translation, then m), as observed on macOS 26.7.3 (x86_64, tier 2): translating a
// scale by (1, 1, 1) leaves (1, 1, 1) in the fourth row where CATransform3DTranslate gives the scale factors.
// Test: tests/src/scenekit_matrix4.m, "SCNMatrix4Translate".
static inline SCNMatrix4 SCNMatrix4Translate(SCNMatrix4 m, CGFloat tx, CGFloat ty, CGFloat tz)
{
    return CATransform3DConcat(m, CATransform3DMakeTranslation(tx, ty, tz));
}

extern bool SCNVector3EqualToVector3(SCNVector3 a, SCNVector3 b);
extern bool SCNVector4EqualToVector4(SCNVector4 a, SCNVector4 b);

extern bool SCNMatrix4EqualToMatrix4(SCNMatrix4 a, SCNMatrix4 b);
extern bool SCNMatrix4IsIdentity(SCNMatrix4 m);
extern SCNMatrix4 SCNMatrix4Invert(SCNMatrix4 m);
extern SCNMatrix4 SCNMatrix4MakeRotation(CGFloat angle, CGFloat x, CGFloat y, CGFloat z);
extern SCNMatrix4 SCNMatrix4Rotate(SCNMatrix4 m, CGFloat angle, CGFloat x, CGFloat y, CGFloat z);
// The product of a and b with a applied first (CATransform3DConcat: row vectors, translation in the fourth row).
extern SCNMatrix4 SCNMatrix4Mult(SCNMatrix4 a, SCNMatrix4 b);
extern SCNMatrix4 SCNMatrix4Scale(SCNMatrix4 m, CGFloat sx, CGFloat sy, CGFloat sz);

#ifdef __cplusplus
}
#endif

#endif
