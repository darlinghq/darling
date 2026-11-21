// Copyright (C) 2026 Darling Developers
//
// SCNMatrix4 and SCNVector3/4 helpers, with expected values derived from the documented contract
// (https://developer.apple.com/documentation/scenekit, pages for each function): SCNMatrix4Invert
// returns its argument when the matrix is not invertible, SCNMatrix4Rotate and SCNMatrix4Scale
// describe the new transformation followed by the given matrix, rotation is counterclockwise about
// the axis. SCNMatrix4 is the Core Animation matrix on macOS, so points are row vectors and the
// translation is in the fourth row (the documented CATransform3DMakeTranslation matrix has tx, ty,
// tz there: https://developer.apple.com/documentation/quartzcore/catransform3dmaketranslation(_:_:_:)).
#import <SceneKit/SceneKit.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static int gFailures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); gFailures++; } } while (0)

// CGFloat is float on i386, so the rounding error of a trigonometric result is much larger there.
static bool near(CGFloat a, CGFloat b) { return fabs(a - b) < (sizeof(CGFloat) == 4 ? 1e-5 : 1e-9); }

// Row-vector transform of the point (x, y, z, 1).
static SCNVector3 apply(SCNMatrix4 m, CGFloat x, CGFloat y, CGFloat z)
{
    SCNVector3 r = {
        x * m.m11 + y * m.m21 + z * m.m31 + m.m41,
        x * m.m12 + y * m.m22 + z * m.m32 + m.m42,
        x * m.m13 + y * m.m23 + z * m.m33 + m.m43,
    };
    return r;
}

#define CHECK_POINT(p, ex, ey, ez) CHECK(near((p).x, ex) && near((p).y, ey) && near((p).z, ez))

int main(void)
{
    // Identity: ones on the diagonal, zeros elsewhere.
    CHECK(SCNMatrix4IsIdentity(SCNMatrix4Identity));
    CHECK(SCNMatrix4Identity.m11 == 1 && SCNMatrix4Identity.m22 == 1 && SCNMatrix4Identity.m33 == 1 && SCNMatrix4Identity.m44 == 1);
    CHECK(SCNMatrix4Identity.m12 == 0 && SCNMatrix4Identity.m41 == 0 && SCNMatrix4Identity.m34 == 0);
    SCNMatrix4 notIdentity = SCNMatrix4Identity;
    notIdentity.m41 = 1;
    CHECK(!SCNMatrix4IsIdentity(notIdentity));
    notIdentity = SCNMatrix4Identity;
    notIdentity.m44 = 2;
    CHECK(!SCNMatrix4IsIdentity(notIdentity));

    // EqualToMatrix4 compares every element numerically.
    CHECK(SCNMatrix4EqualToMatrix4(SCNMatrix4Identity, SCNMatrix4Identity));
    CHECK(!SCNMatrix4EqualToMatrix4(SCNMatrix4Identity, notIdentity));
    SCNMatrix4 zero;
    memset(&zero, 0, sizeof(zero));
    SCNMatrix4 negativeZero = zero;
    negativeZero.m12 = -0.0;
    CHECK(SCNMatrix4EqualToMatrix4(zero, negativeZero));

    // Invert: inverse of a scale, and the argument itself when it is not invertible.
    SCNMatrix4 scale = SCNMatrix4Scale(SCNMatrix4Identity, 2, 4, 8);
    SCNMatrix4 inverse = SCNMatrix4Invert(scale);
    SCNVector3 p = apply(inverse, 2, 4, 8);
    CHECK_POINT(p, 1, 1, 1);
    CHECK(SCNMatrix4EqualToMatrix4(SCNMatrix4Invert(SCNMatrix4Identity), SCNMatrix4Identity));
    CHECK(SCNMatrix4EqualToMatrix4(SCNMatrix4Invert(zero), zero));

    // MakeRotation: counterclockwise about +z takes +x to +y.
    SCNMatrix4 quarterTurn = SCNMatrix4MakeRotation(M_PI / 2, 0, 0, 1);
    p = apply(quarterTurn, 1, 0, 0);
    CHECK_POINT(p, 0, 1, 0);
    p = apply(quarterTurn, 0, 1, 0);
    CHECK_POINT(p, -1, 0, 0);
    SCNMatrix4 aboutX = SCNMatrix4MakeRotation(M_PI / 2, 1, 0, 0);
    p = apply(aboutX, 0, 1, 0);
    CHECK_POINT(p, 0, 0, 1);
    p = apply(aboutX, 0, 0, 1);
    CHECK_POINT(p, 0, -1, 0);
    SCNMatrix4 aboutY = SCNMatrix4MakeRotation(M_PI / 2, 0, 1, 0);
    p = apply(aboutY, 0, 0, 1);
    CHECK_POINT(p, 1, 0, 0);
    p = apply(aboutY, 1, 0, 0);
    CHECK_POINT(p, 0, 0, -1);
    CHECK(SCNMatrix4IsIdentity(SCNMatrix4MakeRotation(0, 0, 0, 1)));

    // Rotate and Scale: the new transformation is followed by the given matrix.
    SCNMatrix4 translation = SCNMatrix4Identity;
    translation.m41 = 10;
    p = apply(SCNMatrix4Rotate(translation, M_PI / 2, 0, 0, 1), 1, 0, 0);
    CHECK_POINT(p, 10, 1, 0);
    p = apply(SCNMatrix4Scale(translation, 2, 3, 4), 1, 1, 1);
    CHECK_POINT(p, 12, 3, 4);

    // Vectors compare component by component; the zero constants are all zeros.
    SCNVector3 a = { 1, 2, 3 };
    SCNVector3 b = { 1, 2, 3 };
    CHECK(SCNVector3EqualToVector3(a, b));
    b.z = 3.5;
    CHECK(!SCNVector3EqualToVector3(a, b));
    // Numeric, not bitwise, comparison: -0.0 equals 0.0.
    SCNVector3 positiveZero = { 0, 0, 0 };
    SCNVector3 negativeZeroVector = { -0.0, -0.0, -0.0 };
    CHECK(SCNVector3EqualToVector3(positiveZero, negativeZeroVector));
    SCNVector4 c = { 1, 2, 3, 4 };
    SCNVector4 d = { 1, 2, 3, 4 };
    CHECK(SCNVector4EqualToVector4(c, d));
    d.w = 0;
    CHECK(!SCNVector4EqualToVector4(c, d));
    SCNVector4 positiveZero4 = { 0, 0, 0, 0 };
    SCNVector4 negativeZero4 = { -0.0, -0.0, -0.0, -0.0 };
    CHECK(SCNVector4EqualToVector4(positiveZero4, negativeZero4));
    CHECK(SCNVector3Zero.x == 0 && SCNVector3Zero.y == 0 && SCNVector3Zero.z == 0);
    CHECK(SCNVector4Zero.x == 0 && SCNVector4Zero.y == 0 && SCNVector4Zero.z == 0 && SCNVector4Zero.w == 0);

    // The inline creation helpers.
    SCNVector3 made3 = SCNVector3Make(1, 2, 3);
    CHECK(made3.x == 1 && made3.y == 2 && made3.z == 3);
    SCNVector4 made4 = SCNVector4Make(1, 2, 3, 4);
    CHECK(made4.x == 1 && made4.y == 2 && made4.z == 3 && made4.w == 4);
    SCNMatrix4 moved = SCNMatrix4MakeTranslation(1, 2, 3);
    CHECK(moved.m41 == 1 && moved.m42 == 2 && moved.m43 == 3 && moved.m11 == 1 && moved.m44 == 1);
    SCNMatrix4 scaled = SCNMatrix4MakeScale(2, 3, 4);
    CHECK(scaled.m11 == 2 && scaled.m22 == 3 && scaled.m33 == 4 && scaled.m44 == 1 && scaled.m41 == 0);
    // SCNMatrix4Translate: m, then the translation (observed; CATransform3DTranslate does the opposite).
    p = apply(SCNMatrix4Translate(scaled, 1, 1, 1), 1, 1, 1);
    CHECK_POINT(p, 3, 4, 5);
    SCNMatrix4 turned = SCNMatrix4Translate(SCNMatrix4MakeRotation(M_PI / 2, 0, 0, 1), 1, 0, 0);
    CHECK(near(turned.m41, 1) && near(turned.m42, 0) && near(turned.m43, 0));

    // SCNMatrix4Mult: a is applied first (row vectors): the product of a translation and a scale moves, then scales.
    // The element values are what SCNMatrix4Mult returned on macOS 26.7.3 for these inputs.
    SCNMatrix4 ma = moved, mb = scaled;
    ma.m12 = 0.5;
    mb.m21 = 0.25;
    SCNMatrix4 ab = SCNMatrix4Mult(ma, mb), ba = SCNMatrix4Mult(mb, ma);
    CHECK(near(ab.m11, 2.125) && near(ab.m12, 1.5) && near(ab.m21, 0.25) && near(ab.m22, 3) && near(ab.m33, 4));
    CHECK(near(ab.m41, 2.5) && near(ab.m42, 6) && near(ab.m43, 12) && near(ab.m44, 1));
    CHECK(near(ba.m11, 2) && near(ba.m12, 1) && near(ba.m21, 0.25) && near(ba.m22, 3.125) && near(ba.m33, 4));
    CHECK(near(ba.m41, 1) && near(ba.m42, 2) && near(ba.m43, 3) && near(ba.m44, 1));
    p = apply(SCNMatrix4Mult(moved, scaled), 1, 1, 1);
    CHECK_POINT(p, 4, 9, 16);
    CHECK(SCNMatrix4EqualToMatrix4(SCNMatrix4Mult(ma, SCNMatrix4Identity), ma));

    if (gFailures)
        return 1;
    printf("PASS: SceneKit matrix and vector helpers\n");
    return 0;
}
