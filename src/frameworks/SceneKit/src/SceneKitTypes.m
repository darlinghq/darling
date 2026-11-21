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

#include <SceneKit/SceneKitTypes.h>

// The matrix functions defer to QuartzCore: SCNMatrix4 is CATransform3D, so the
// documented behaviour (row-vector convention, transformation applied before the
// given matrix, original matrix returned when not invertible) is Core Animation's.

const SCNMatrix4 SCNMatrix4Identity = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1,
};
const SCNVector3 SCNVector3Zero = { 0, 0, 0 };
const SCNVector4 SCNVector4Zero = { 0, 0, 0, 0 };

bool SCNVector3EqualToVector3(SCNVector3 a, SCNVector3 b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

bool SCNVector4EqualToVector4(SCNVector4 a, SCNVector4 b) {
    return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

bool SCNMatrix4EqualToMatrix4(SCNMatrix4 a, SCNMatrix4 b) {
    return CATransform3DEqualToTransform(a, b);
}

bool SCNMatrix4IsIdentity(SCNMatrix4 m) {
    return CATransform3DIsIdentity(m);
}

SCNMatrix4 SCNMatrix4Invert(SCNMatrix4 m) {
    return CATransform3DInvert(m);
}

SCNMatrix4 SCNMatrix4MakeRotation(CGFloat angle, CGFloat x, CGFloat y, CGFloat z) {
    return CATransform3DMakeRotation(angle, x, y, z);
}

SCNMatrix4 SCNMatrix4Rotate(SCNMatrix4 m, CGFloat angle, CGFloat x, CGFloat y, CGFloat z) {
    return CATransform3DRotate(m, angle, x, y, z);
}

// The operand order is the one observed on macOS 26.7.3 (x86_64): SCNMatrix4Mult(a, b) equals
// CATransform3DConcat(a, b), so a is applied first. The documentation page calls `a` the left operand and says
// "b followed by a", which does not settle it for the row-vector CATransform3D layout (tier 2 black-box evidence).
// Test: tests/src/scenekit_matrix4.m, "SCNMatrix4Mult".
SCNMatrix4 SCNMatrix4Mult(SCNMatrix4 a, SCNMatrix4 b) {
    return CATransform3DConcat(a, b);
}

SCNMatrix4 SCNMatrix4Scale(SCNMatrix4 m, CGFloat sx, CGFloat sy, CGFloat sz) {
    return CATransform3DScale(m, sx, sy, sz);
}
