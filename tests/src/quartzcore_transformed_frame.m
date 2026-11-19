/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * CALayer geometry of a layer with a transform: -frame, -setFrame:, the
 * autoresizing of a transformed sublayer, key paths into struct properties, and
 * the conversion methods.
 *
 * The frame of a transformed layer is the bounding box of its transformed
 * bounds (the bounds origin does not take part; anchorPointZ and perspective
 * do). Setting the frame of a transformed layer is documented as undefined; the
 * expected values below were read on macOS 26.7.3 (x86_64) with probes that are
 * not part of this test: the bounds size is the frame size taken through the
 * inverse of an affine transform's 2x2 part (sign dropped), a non-affine
 * transform keeps the frame size, and the position is the frame origin plus the
 * anchor point's share of the frame size. Autoresizing of a transformed
 * sublayer works on its transformed frame. The rendering part needs OpenGL (a
 * display on Darling) and prints SKIP where an offscreen CARenderer draws
 * nothing. No ARC.
 */
#define GL_GLEXT_PROTOTYPES 1
#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int failures;

#define CHECK(cond, ...) \
	do { \
		if (cond) { \
			printf("PASS: " __VA_ARGS__); \
		} else { \
			printf("FAIL: " __VA_ARGS__); \
			failures++; \
		} \
		printf("\n"); \
	} while (0)

static void watchdog(int sig)
{
	(void) sig;
	printf("FAIL: watchdog\n");
	_exit(2);
}

static BOOL near(double a, double b)
{
	return fabs(a - b) < 1e-3;
}

static BOOL rectNear(CGRect r, double x, double y, double w, double h)
{
	return near(r.origin.x, x) && near(r.origin.y, y) && near(r.size.width, w) && near(r.size.height, h);
}

#define RECT_FMT "(%g,%g %gx%g)"
#define RECT_ARGS(r) (r).origin.x, (r).origin.y, (r).size.width, (r).size.height

static CATransform3D transformNumber(int k)
{
	switch (k) {
	case 0: return CATransform3DMakeRotation(M_PI / 2, 0, 0, 1);
	case 1: return CATransform3DMakeRotation(M_PI / 6, 0, 0, 1);
	case 2: return CATransform3DMakeScale(2, 0.5, 1);
	case 3: return CATransform3DMakeTranslation(10, 20, 0);
	case 4: return CATransform3DRotate(CATransform3DMakeScale(2, 1, 1), M_PI / 4, 0, 0, 1);
	case 5: return CATransform3DMakeScale(-1, 1, 1);
	}
	return CATransform3DIdentity;
}

static const char *transformName[] = { "rot90", "rot30", "scale2x.5", "trans", "scale2rot45", "flipx" };
static CGPoint anchors[3] = { { 0.5, 0.5 }, { 0, 0 }, { 1, 0.25 } };
static CGRect boundsChoices[2] = { { { 0, 0 }, { 100, 50 } }, { { 7, 9 }, { 100, 50 } } };

static const struct { int transform, anchor, boundsOrigin; double x, y, w, h; } frameCases[] = {
	{ 0, 0, 0, 175, 100, 50, 100 },
	{ 0, 0, 1, 175, 100, 50, 100 },
	{ 0, 1, 0, 150, 150, 50, 100 },
	{ 0, 1, 1, 150, 150, 50, 100 },
	{ 0, 2, 0, 162.5, 50, 50, 100 },
	{ 0, 2, 1, 162.5, 50, 50, 100 },
	{ 1, 0, 0, 144.199, 103.349, 111.603, 93.3013 },
	{ 1, 0, 1, 144.199, 103.349, 111.603, 93.3013 },
	{ 1, 1, 0, 175, 150, 111.603, 93.3013 },
	{ 1, 1, 1, 175, 150, 111.603, 93.3013 },
	{ 1, 2, 0, 94.6475, 89.1747, 111.603, 93.3013 },
	{ 1, 2, 1, 94.6475, 89.1747, 111.603, 93.3013 },
	{ 2, 0, 0, 100, 137.5, 200, 25 },
	{ 2, 0, 1, 100, 137.5, 200, 25 },
	{ 2, 1, 0, 200, 150, 200, 25 },
	{ 2, 1, 1, 200, 150, 200, 25 },
	{ 2, 2, 0, 0, 143.75, 200, 25 },
	{ 2, 2, 1, 0, 143.75, 200, 25 },
	{ 3, 0, 0, 160, 145, 100, 50 },
	{ 3, 0, 1, 160, 145, 100, 50 },
	{ 3, 1, 0, 210, 170, 100, 50 },
	{ 3, 1, 1, 210, 170, 100, 50 },
	{ 4, 0, 0, 93.934, 96.967, 212.132, 106.066 },
	{ 4, 0, 1, 93.934, 96.967, 212.132, 106.066 },
	{ 4, 1, 0, 129.289, 150, 212.132, 106.066 },
	{ 4, 1, 1, 129.289, 150, 212.132, 106.066 },
	{ 4, 2, 0, 5.54564, 70.4505, 212.132, 106.066 },
	{ 4, 2, 1, 5.54564, 70.4505, 212.132, 106.066 },
	{ 5, 0, 0, 150, 125, 100, 50 },
	{ 5, 0, 1, 150, 125, 100, 50 },
	{ 5, 1, 0, 100, 150, 100, 50 },
	{ 5, 1, 1, 100, 150, 100, 50 },
};

static const struct { int transform, anchor; double bw, bh, px, py; } setFrameCases[] = {
	{ 0, 0, 100, 200, 150, 110 },
	{ 0, 1, 100, 200, 50, 60 },
	{ 1, 0, 223.205, 13.3975, 150, 110 },
	{ 1, 1, 223.205, 13.3975, 50, 60 },
	{ 2, 0, 100, 200, 150, 110 },
	{ 2, 1, 100, 200, 50, 60 },
	{ 3, 0, 200, 100, 140, 90 },
	{ 3, 1, 200, 100, 40, 40 },
	{ 5, 0, 200, 100, 150, 110 },
	{ 5, 1, 200, 100, 50, 60 },
};

static const struct { int transform; unsigned mask; double bw, bh, px, py, fx, fy, fw, fh; } resizeCases[] = {
	{ 0, 0, 20, 10, 50, 40, 30, 37.5, 40, 5 },
	{ 0, 1, 20, 12, 110, 40, 90, 37, 40, 6 },
	{ 0, 2, 50, 12, 80, 40, 30, 37, 100, 6 },
	{ 0, 4, 20, 12, 50, 40, 30, 37, 40, 6 },
	{ 0, 5, 20, 12, 80, 40, 60, 37, 40, 6 },
	{ 0, 8, 20, 12, 50, 160, 30, 157, 40, 6 },
	{ 0, 9, 20, 12, 110, 160, 90, 157, 40, 6 },
	{ 0, 16, 20, 252, 50, 100, 30, 37, 40, 126 },
	{ 0, 18, 50, 252, 80, 100, 30, 37, 100, 126 },
	{ 0, 32, 20, 12, 50, 40, 30, 37, 40, 6 },
	{ 0, 58, 50, 92, 80, 100, 30, 77, 100, 46 },
	{ 0, 63, 30, 92, 80, 100, 50, 77, 60, 46 },
	{ 1, 0, 20, 10, 50, 40, 45, 30, 10, 20 },
	{ 1, 1, 20, 10, 110, 40, 105, 30, 10, 20 },
	{ 1, 2, 20, 70, 80, 40, 45, 30, 70, 20 },
	{ 1, 4, 20, 10, 50, 40, 45, 30, 10, 20 },
	{ 1, 5, 20, 10, 80, 40, 75, 30, 10, 20 },
	{ 1, 8, 20, 10, 50, 160, 45, 150, 10, 20 },
	{ 1, 9, 20, 10, 110, 160, 105, 150, 10, 20 },
	{ 1, 16, 140, 10, 50, 100, 45, 30, 10, 140 },
	{ 1, 18, 140, 70, 80, 100, 45, 30, 70, 140 },
	{ 1, 32, 20, 10, 50, 40, 45, 30, 10, 20 },
	{ 1, 58, 60, 70, 80, 100, 45, 70, 70, 60 },
	{ 1, 63, 60, 30, 80, 100, 65, 70, 30, 60 },
	{ 2, 0, 20, 10, 50, 40, 38.8397, 30.6699, 22.3205, 18.6603 },
	{ 2, 1, 30.7846, 5.32051, 110, 40, 95.3397, 30, 29.3205, 20 },
	{ 2, 2, 82.7461, 24.6795, 80, 40, 38, 8.62693, 84, 62.7461 },
	{ 2, 4, 30.7846, 5.32051, 50, 40, 35.3397, 30, 29.3205, 20 },
	{ 2, 5, 30.7846, 5.32051, 80, 40, 65.3397, 30, 29.3205, 20 },
	{ 2, 8, 30.7846, 5.32051, 50, 160, 35.3397, 150, 29.3205, 20 },
	{ 2, 9, 30.7846, 5.32051, 110, 160, 95.3397, 150, 29.3205, 20 },
	{ 2, 16, 90.7846, 109.244, 50, 100, -16.6218, 30, 133.244, 140 },
	{ 2, 18, 142.746, 79.2436, 80, 100, -1.62178, 30, 163.244, 140 },
	{ 2, 32, 30.7846, 5.32051, 50, 40, 35.3397, 30, 29.3205, 20 },
	{ 2, 58, 102.746, 9.96152, 80, 100, 33.0192, 70, 93.9615, 60 },
	{ 2, 63, 68.1051, 29.9615, 80, 100, 43.0192, 70, 73.9615, 60 },
	{ 3, 0, 20, 10, 50, 40, 28.7868, 29.3934, 42.4264, 21.2132 },
	{ 3, 1, 31.1127, 3.55271e-15, 110, 40, 88, 29, 44, 22 },
	{ 3, 2, 52.3259, 21.2132, 80, 40, 28, 14, 104, 52 },
	{ 3, 4, 31.1127, 3.55271e-15, 50, 40, 28, 29, 44, 22 },
	{ 3, 5, 31.1127, 3.55271e-15, 80, 40, 58, 29, 44, 22 },
	{ 3, 8, 31.1127, 3.55271e-15, 50, 160, 28, 149, 44, 22 },
	{ 3, 9, 31.1127, 3.55271e-15, 110, 160, 88, 149, 44, 22 },
	{ 3, 16, 115.966, 84.8528, 50, 100, -92, 29, 284, 142 },
	{ 3, 18, 137.179, 63.6396, 80, 100, -62, 29, 284, 142 },
	{ 3, 32, 31.1127, 3.55271e-15, 50, 40, 28, 29, 44, 22 },
	{ 3, 58, 80.6102, 7.07107, 80, 100, 18, 69, 124, 62 },
	{ 3, 63, 66.468, 21.2132, 80, 100, 18, 69, 124, 62 },
};

static void checkFrame(void)
{
	for (size_t i = 0; i < sizeof frameCases / sizeof frameCases[0]; i++) {
		CALayer *l = [CALayer layer];
		l.bounds = boundsChoices[frameCases[i].boundsOrigin];
		l.anchorPoint = anchors[frameCases[i].anchor];
		l.position = CGPointMake(200, 150);
		l.transform = transformNumber(frameCases[i].transform);
		CGRect f = l.frame;
		CHECK(rectNear(f, frameCases[i].x, frameCases[i].y, frameCases[i].w, frameCases[i].h),
			"frame of %s, anchor %d, bounds origin %d is " RECT_FMT, transformName[frameCases[i].transform],
			frameCases[i].anchor, frameCases[i].boundsOrigin, RECT_ARGS(f));
	}

	// 3D transforms project to the plane; anchorPointZ takes part.
	CALayer *l = [CALayer layer];
	l.bounds = CGRectMake(0, 0, 100, 50);
	l.position = CGPointMake(200, 150);
	l.transform = CATransform3DMakeRotation(M_PI / 3, 0, 1, 0);
	CHECK(rectNear(l.frame, 175, 125, 50, 50), "frame of a 60 degree rotation about y is " RECT_FMT, RECT_ARGS(l.frame));
	CATransform3D perspective = CATransform3DIdentity;
	perspective.m34 = -1.0 / 500;
	l.transform = CATransform3DConcat(l.transform, perspective);
	CHECK(rectNear(l.frame, 172.63, 122.63, 50.3778, 54.7407), "frame with perspective is " RECT_FMT, RECT_ARGS(l.frame));
	l.transform = CATransform3DMakeRotation(M_PI / 3, 0, 1, 0);
	l.anchorPointZ = 30;
	CHECK(rectNear(l.frame, 149.019, 125, 50, 50), "frame with anchorPointZ 30 is " RECT_FMT, RECT_ARGS(l.frame));

	// An identity transform keeps the plain rule.
	CALayer *plain = [CALayer layer];
	plain.bounds = CGRectMake(7, 9, 100, 50);
	plain.anchorPoint = CGPointMake(1, 0.25);
	plain.position = CGPointMake(200, 150);
	CHECK(rectNear(plain.frame, 100, 137.5, 100, 50), "identity transform: frame " RECT_FMT, RECT_ARGS(plain.frame));
}

static void checkSetFrame(void)
{
	for (size_t i = 0; i < sizeof setFrameCases / sizeof setFrameCases[0]; i++) {
		CALayer *l = [CALayer layer];
		l.bounds = CGRectMake(0, 0, 100, 50);
		l.anchorPoint = anchors[setFrameCases[i].anchor];
		l.position = CGPointMake(200, 150);
		l.transform = transformNumber(setFrameCases[i].transform);
		l.frame = CGRectMake(50, 60, 200, 100);
		CHECK(near(l.bounds.size.width, setFrameCases[i].bw) && near(l.bounds.size.height, setFrameCases[i].bh) &&
				near(l.position.x, setFrameCases[i].px) && near(l.position.y, setFrameCases[i].py),
			"setFrame: of %s, anchor %d: bounds %gx%g, position (%g,%g)", transformName[setFrameCases[i].transform],
			setFrameCases[i].anchor, l.bounds.size.width, l.bounds.size.height, l.position.x, l.position.y);
	}

	// A bounds origin stays; a shear and a 3D rotation (which keeps the size).
	CALayer *l = [CALayer layer];
	l.bounds = CGRectMake(5, 6, 100, 50);
	l.position = CGPointMake(200, 150);
	l.transform = CATransform3DMakeRotation(M_PI / 6, 0, 0, 1);
	l.frame = CGRectMake(30, 40, 70, 20);
	CHECK(rectNear(l.bounds, 5, 6, 70.6218, 17.6795) && near(l.position.x, 65) && near(l.position.y, 50),
		"rot30 setFrame: keeps the bounds origin: bounds " RECT_FMT " position (%g,%g)", RECT_ARGS(l.bounds), l.position.x, l.position.y);
	CATransform3D shear = CATransform3DIdentity;
	shear.m21 = 0.5;
	l.bounds = CGRectMake(5, 6, 100, 50);
	l.transform = shear;
	l.frame = CGRectMake(30, 40, 70, 20);
	CHECK(rectNear(l.bounds, 5, 6, 60, 20) && rectNear(l.frame, 30, 40, 70, 20), "shear setFrame: bounds " RECT_FMT, RECT_ARGS(l.bounds));
	l.bounds = CGRectMake(5, 6, 100, 50);
	l.transform = CATransform3DMakeRotation(M_PI / 3, 0, 1, 0);
	l.frame = CGRectMake(30, 40, 70, 20);
	CHECK(rectNear(l.bounds, 5, 6, 70, 20) && near(l.position.x, 65) && near(l.position.y, 50),
		"rotation about y setFrame: bounds " RECT_FMT, RECT_ARGS(l.bounds));
	l.bounds = CGRectMake(5, 6, 100, 50);
	l.transform = CATransform3DMakeScale(1, -2, 1);
	l.frame = CGRectMake(30, 40, 70, 20);
	CHECK(rectNear(l.bounds, 5, 6, 70, 10) && rectNear(l.frame, 30, 40, 70, 20), "negative scale setFrame: bounds " RECT_FMT, RECT_ARGS(l.bounds));

	CALayer *s = [CALayer layer];
	s.bounds = CGRectMake(0, 0, 100, 50);
	s.position = CGPointMake(200, 150);
	s.transform = CATransform3DMakeScale(2, 2, 1);
	s.frame = CGRectMake(10, 10, 50, 50);
	CHECK(rectNear(s.bounds, 0, 0, 25, 25) && near(s.position.x, 35) && near(s.position.y, 35) && rectNear(s.frame, 10, 10, 50, 50),
		"scale 2 setFrame: bounds " RECT_FMT " position (%g,%g)", RECT_ARGS(s.bounds), s.position.x, s.position.y);

	// The frame round-trips for the identity transform as before.
	CALayer *i = [CALayer layer];
	i.anchorPoint = CGPointMake(0.25, 0.75);
	i.frame = CGRectMake(3, 4, 80, 40);
	CHECK(rectNear(i.frame, 3, 4, 80, 40) && rectNear(i.bounds, 0, 0, 80, 40), "identity transform: setFrame: round-trips, bounds " RECT_FMT, RECT_ARGS(i.bounds));
}

static void checkAutoresizing(void)
{
	[CATransaction begin];
	[CATransaction setDisableActions: YES];
	for (size_t i = 0; i < sizeof resizeCases / sizeof resizeCases[0]; i++) {
		const char *names[] = { "scale2x.5", "rot90", "rot30", "scale2rot45" };
		int t = resizeCases[i].transform;
		CALayer *p = [CALayer layer];
		p.bounds = CGRectMake(0, 0, 100, 100);
		CALayer *c = [CALayer layer];
		c.bounds = CGRectMake(0, 0, 20, 10);
		c.position = CGPointMake(50, 40);
		c.transform = transformNumber(t == 0 ? 2 : t == 1 ? 0 : t == 2 ? 1 : 4);
		c.autoresizingMask = resizeCases[i].mask;
		[p addSublayer: c];
		p.bounds = CGRectMake(0, 0, 160, 220);
		CHECK(near(c.bounds.size.width, resizeCases[i].bw) && near(c.bounds.size.height, resizeCases[i].bh) &&
				near(c.position.x, resizeCases[i].px) && near(c.position.y, resizeCases[i].py) &&
				rectNear(c.frame, resizeCases[i].fx, resizeCases[i].fy, resizeCases[i].fw, resizeCases[i].fh),
			"autoresizing of %s, mask %u: bounds %gx%g position (%g,%g) frame " RECT_FMT, names[t], resizeCases[i].mask,
			c.bounds.size.width, c.bounds.size.height, c.position.x, c.position.y, RECT_ARGS(c.frame));
	}
	[CATransaction commit];
}

static BOOL sameSize(id value, double w, double h)
{
	NSSize s = [value sizeValue];
	return near(s.width, w) && near(s.height, h);
}

static void checkKeyPaths(void)
{
	CALayer *l = [CALayer layer];
	l.bounds = CGRectMake(0, 0, 100, 50);
	l.position = CGPointMake(200, 150);

	BOOL ok = YES;
	@try {
		ok = [[l valueForKeyPath: @"bounds.size.width"] doubleValue] == 100 &&
			[[l valueForKeyPath: @"bounds.size.height"] doubleValue] == 50 &&
			[[l valueForKeyPath: @"bounds.origin.x"] doubleValue] == 0 &&
			[[l valueForKeyPath: @"position.x"] doubleValue] == 200 &&
			[[l valueForKeyPath: @"position.y"] doubleValue] == 150 &&
			[[l valueForKeyPath: @"anchorPoint.x"] doubleValue] == 0.5 &&
			[[l valueForKeyPath: @"frame.origin.x"] doubleValue] == 150 &&
			[[l valueForKeyPath: @"frame.origin.y"] doubleValue] == 125 &&
			[[l valueForKeyPath: @"frame.size.width"] doubleValue] == 100 &&
			sameSize([l valueForKeyPath: @"bounds.size"], 100, 50) &&
			[[l valueForKeyPath: @"transform.scale"] doubleValue] == 1 &&
			[[l valueForKeyPath: @"transform.scale.x"] doubleValue] == 1 &&
			[[l valueForKeyPath: @"transform.rotation"] doubleValue] == 0 &&
			[[l valueForKeyPath: @"transform.rotation.z"] doubleValue] == 0 &&
			[[l valueForKeyPath: @"transform.translation.x"] doubleValue] == 0 &&
			sameSize([l valueForKeyPath: @"transform.translation"], 0, 0) &&
			[[l valueForKeyPath: @"contentsRect.size.width"] doubleValue] == 1 &&
			[[l valueForKeyPath: @"shadowOffset.width"] doubleValue] == 0;
	} @catch (NSException *e) {
		ok = NO;
		printf("  exception %s\n", [[e name] UTF8String]);
	}
	CHECK(ok, "struct members read through key paths");

	[CATransaction begin];
	[CATransaction setDisableActions: YES];
	ok = YES;
	@try {
		[l setValue: [NSNumber numberWithDouble: 300] forKeyPath: @"bounds.size.width"];
		ok = rectNear(l.bounds, 0, 0, 300, 50) && rectNear(l.frame, 50, 125, 300, 50);
		CHECK(ok, "bounds.size.width set: bounds " RECT_FMT " frame " RECT_FMT, RECT_ARGS(l.bounds), RECT_ARGS(l.frame));
		l.bounds = CGRectMake(0, 0, 100, 50);
		[l setValue: [NSNumber numberWithDouble: 0.5] forKeyPath: @"bounds.origin.x"];
		CHECK(rectNear(l.bounds, 0.5, 0, 100, 50), "bounds.origin.x set: bounds " RECT_FMT, RECT_ARGS(l.bounds));
		[l setValue: [NSValue valueWithSize: NSMakeSize(30, 40)] forKeyPath: @"bounds.size"];
		CHECK(rectNear(l.bounds, 0.5, 0, 30, 40), "bounds.size set from a size value: bounds " RECT_FMT, RECT_ARGS(l.bounds));
		l.bounds = CGRectMake(0, 0, 100, 50);
		[l setValue: [NSNumber numberWithDouble: 0.5] forKeyPath: @"position.x"];
		[l setValue: [NSNumber numberWithDouble: 7] forKeyPath: @"position.y"];
		CHECK(near(l.position.x, 0.5) && near(l.position.y, 7), "position.x and position.y set: (%g,%g)", l.position.x, l.position.y);
		l.position = CGPointMake(200, 150);
		[l setValue: [NSNumber numberWithDouble: 0.25] forKeyPath: @"anchorPoint.x"];
		CHECK(near(l.anchorPoint.x, 0.25) && near(l.anchorPoint.y, 0.5), "anchorPoint.x set: (%g,%g)", l.anchorPoint.x, l.anchorPoint.y);
		l.anchorPoint = CGPointMake(0.5, 0.5);
		[l setValue: [NSNumber numberWithDouble: 0.5] forKeyPath: @"frame.size.width"];
		CHECK(rectNear(l.bounds, 0, 0, 0.5, 50) && near(l.position.x, 150.25), "frame.size.width set: bounds " RECT_FMT " position.x %g", RECT_ARGS(l.bounds), l.position.x);
		l.bounds = CGRectMake(0, 0, 100, 50);
		l.position = CGPointMake(200, 150);
		[l setValue: [NSNumber numberWithDouble: 0.5] forKeyPath: @"frame.origin.x"];
		CHECK(near(l.position.x, 50.5) && near(l.frame.origin.x, 0.5), "frame.origin.x set: position.x %g", l.position.x);
		l.position = CGPointMake(200, 150);
		[l setValue: [NSNumber numberWithDouble: 0.5] forKeyPath: @"shadowOffset.width"];
		CHECK(near(l.shadowOffset.width, 0.5), "shadowOffset.width set: %g", l.shadowOffset.width);
		[l setValue: [NSNumber numberWithDouble: 0.25] forKeyPath: @"contentsRect.size.width"];
		CHECK(near(l.contentsRect.size.width, 0.25), "contentsRect.size.width set: %g", l.contentsRect.size.width);
		l.contentsRect = CGRectMake(0, 0, 1, 1);

		CATransform3D t;
		l.transform = CATransform3DIdentity;
		[l setValue: [NSNumber numberWithDouble: 0.5] forKeyPath: @"transform.scale"];
		t = l.transform;
		CHECK(near(t.m11, 0.5) && near(t.m22, 0.5) && rectNear(l.frame, 175, 137.5, 50, 25), "transform.scale set: m11 %g m22 %g", t.m11, t.m22);
		l.transform = CATransform3DIdentity;
		[l setValue: [NSNumber numberWithDouble: 0.5] forKeyPath: @"transform.scale.x"];
		CHECK(near(l.transform.m11, 0.5) && near(l.transform.m22, 1), "transform.scale.x set: m11 %g m22 %g", l.transform.m11, l.transform.m22);
		l.transform = CATransform3DIdentity;
		[l setValue: [NSNumber numberWithDouble: 0.5] forKeyPath: @"transform.scale.y"];
		CHECK(near(l.transform.m11, 1) && near(l.transform.m22, 0.5), "transform.scale.y set: m11 %g m22 %g", l.transform.m11, l.transform.m22);
		l.transform = CATransform3DIdentity;
		[l setValue: [NSNumber numberWithDouble: 0.5] forKeyPath: @"transform.rotation"];
		t = l.transform;
		CHECK(near(t.m11, 0.877583) && near(t.m12, 0.479426) && near(t.m21, -0.479426) && near(t.m22, 0.877583) &&
				rectNear(l.frame, 144.135, 104.089, 111.73, 91.8217),
			"transform.rotation set: m11 %g m12 %g, frame " RECT_FMT, t.m11, t.m12, RECT_ARGS(l.frame));
		CHECK(near([[l valueForKeyPath: @"transform.rotation.z"] doubleValue], 0.5), "transform.rotation.z reads back %g",
			[[l valueForKeyPath: @"transform.rotation.z"] doubleValue]);
		l.transform = CATransform3DIdentity;
		[l setValue: [NSNumber numberWithDouble: 0.5] forKeyPath: @"transform.rotation.z"];
		CHECK(near(l.transform.m11, 0.877583) && near(l.transform.m12, 0.479426), "transform.rotation.z set: m11 %g m12 %g", l.transform.m11, l.transform.m12);
		l.transform = CATransform3DIdentity;
		[l setValue: [NSNumber numberWithDouble: 0.5] forKeyPath: @"transform.rotation.x"];
		CHECK(near(l.transform.m11, 1) && near(l.transform.m22, 0.877583) && rectNear(l.frame, 150, 128.06, 100, 43.8791),
			"transform.rotation.x set: m22 %g, frame " RECT_FMT, l.transform.m22, RECT_ARGS(l.frame));
		l.transform = CATransform3DIdentity;
		[l setValue: [NSNumber numberWithDouble: 0.5] forKeyPath: @"transform.rotation.y"];
		CHECK(near(l.transform.m11, 0.877583) && near(l.transform.m22, 1) && rectNear(l.frame, 156.121, 125, 87.7583, 50),
			"transform.rotation.y set: m11 %g, frame " RECT_FMT, l.transform.m11, RECT_ARGS(l.frame));
		l.transform = CATransform3DIdentity;
		[l setValue: [NSNumber numberWithDouble: 0.5] forKeyPath: @"transform.translation.x"];
		[l setValue: [NSNumber numberWithDouble: 0.25] forKeyPath: @"transform.translation.y"];
		CHECK(near(l.transform.m41, 0.5) && near(l.transform.m42, 0.25), "transform.translation.x and .y set: %g, %g", l.transform.m41, l.transform.m42);
		l.transform = CATransform3DIdentity;
		[l setValue: [NSNumber numberWithDouble: 0.5] forKeyPath: @"sublayerTransform.scale"];
		CHECK(near(l.sublayerTransform.m11, 0.5) && near(l.sublayerTransform.m22, 0.5) && CATransform3DIsIdentity(l.transform),
			"sublayerTransform.scale set: %g", l.sublayerTransform.m11);
	} @catch (NSException *e) {
		printf("FAIL: key path set raised %s\n", [[e name] UTF8String]);
		failures++;
	}
	[CATransaction commit];

	// A key path set goes through the accessors, so autoresizing runs.
	[CATransaction begin];
	[CATransaction setDisableActions: YES];
	CALayer *p = [CALayer layer];
	p.bounds = CGRectMake(0, 0, 100, 100);
	CALayer *c = [CALayer layer];
	c.anchorPoint = CGPointMake(0, 0);
	c.frame = CGRectMake(10, 10, 20, 20);
	c.autoresizingMask = kCALayerWidthSizable;
	[p addSublayer: c];
	@try {
		[p setValue: [NSNumber numberWithDouble: 150] forKeyPath: @"bounds.size.width"];
		CHECK(rectNear(c.frame, 10, 10, 70, 20), "bounds.size.width key path autoresizes sublayers: " RECT_FMT, RECT_ARGS(c.frame));
	} @catch (NSException *e) {
		printf("FAIL: key path set raised %s\n", [[e name] UTF8String]);
		failures++;
	}
	[CATransaction commit];
}

static void checkConvert(void)
{
	[CATransaction begin];
	[CATransaction setDisableActions: YES];
	CALayer *root = [CALayer layer];
	root.bounds = CGRectMake(0, 0, 400, 300);
	root.anchorPoint = CGPointMake(0, 0);
	CALayer *a = [CALayer layer];
	a.bounds = CGRectMake(10, 20, 100, 50);
	a.position = CGPointMake(200, 150);
	a.transform = CATransform3DMakeRotation(M_PI / 6, 0, 0, 1);
	[root addSublayer: a];
	CALayer *b = [CALayer layer];
	b.bounds = CGRectMake(0, 0, 40, 40);
	b.position = CGPointMake(30, 20);
	b.anchorPoint = CGPointMake(0, 0);
	b.transform = CATransform3DMakeScale(2, 0.5, 1);
	[a addSublayer: b];
	CALayer *c = [CALayer layer];
	c.bounds = CGRectMake(5, 5, 60, 60);
	c.position = CGPointMake(300, 100);
	[root addSublayer: c];
	[CATransaction commit];

	CGPoint p = [a convertPoint: CGPointMake(10, 20) toLayer: root];
	CHECK(near(p.x, 169.199) && near(p.y, 103.349), "a toLayer root: (10,20) is (%g,%g)", p.x, p.y);
	p = [a convertPoint: CGPointMake(60, 45) toLayer: root];
	CHECK(near(p.x, 200) && near(p.y, 150), "the anchor point of a maps to its position (%g,%g)", p.x, p.y);
	p = [root convertPoint: CGPointMake(200, 150) toLayer: a];
	CHECK(near(p.x, 60) && near(p.y, 45), "root toLayer a (%g,%g)", p.x, p.y);
	p = [b convertPoint: CGPointMake(0, 0) toLayer: root];
	CHECK(near(p.x, 186.519) && near(p.y, 113.349), "b toLayer root (0,0) is (%g,%g)", p.x, p.y);
	p = [b convertPoint: CGPointMake(40, 40) toLayer: root];
	CHECK(near(p.x, 245.801) && near(p.y, 170.67), "b toLayer root (40,40) is (%g,%g)", p.x, p.y);
	p = [b convertPoint: CGPointMake(10, 10) toLayer: c];
	CHECK(near(p.x, -63.6603) && near(p.y, 62.6795), "b toLayer c (%g,%g)", p.x, p.y);
	p = [c convertPoint: CGPointMake(10, 10) fromLayer: b];
	CHECK(near(p.x, -63.6603) && near(p.y, 62.6795), "c fromLayer b (%g,%g)", p.x, p.y);
	CGRect r = [a convertRect: CGRectMake(10, 20, 100, 50) toLayer: root];
	CHECK(rectNear(r, 144.199, 103.349, 111.603, 93.3013), "a rect toLayer root " RECT_FMT, RECT_ARGS(r));
	r = [b convertRect: CGRectMake(0, 0, 40, 40) toLayer: root];
	CHECK(rectNear(r, 176.519, 113.349, 79.282, 57.3205), "b rect toLayer root " RECT_FMT, RECT_ARGS(r));
	r = [b convertRect: CGRectMake(0, 0, 40, 40) toLayer: a];
	CHECK(rectNear(r, 30, 20, 80, 20), "b rect toLayer a " RECT_FMT, RECT_ARGS(r));
	r = [root convertRect: CGRectMake(100, 100, 50, 50) toLayer: b];
	CHECK(rectNear(r, -40.8013, 13.3975, 34.1506, 136.603), "root rect toLayer b " RECT_FMT, RECT_ARGS(r));
	p = [a convertPoint: CGPointMake(10, 20) toLayer: nil];
	CHECK(near(p.x, 169.199) && near(p.y, 103.349), "a toLayer nil (%g,%g)", p.x, p.y);
	p = [c convertPoint: CGPointMake(5, 5) toLayer: nil];
	CHECK(near(p.x, 270) && near(p.y, 70), "c toLayer nil (%g,%g)", p.x, p.y);
}

// The picture of a tree through an offscreen CARenderer.
#define SIZE 16
static CGLContextObj readbackContext;
static GLuint readbackFramebuffer;
static uint8_t pixels[SIZE * SIZE * 4];

static BOOL setUpReadback(void)
{
	CGLPixelFormatAttribute attributes[] = { 0 };
	CGLPixelFormatObj pixelFormat = NULL;
	GLint screens = 0;
	if (CGLChoosePixelFormat(attributes, &pixelFormat, &screens) != kCGLNoError || pixelFormat == NULL)
		return NO;
	if (CGLCreateContext(pixelFormat, NULL, &readbackContext) != kCGLNoError)
		readbackContext = NULL;
	CGLReleasePixelFormat(pixelFormat);
	if (readbackContext == NULL)
		return NO;
	CGLSetCurrentContext(readbackContext);
	GLuint renderbuffer;
	glGenFramebuffers(1, &readbackFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, readbackFramebuffer);
	glGenRenderbuffers(1, &renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, SIZE, SIZE);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);
	BOOL complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	CGLSetCurrentContext(NULL);
	return complete;
}

static const uint8_t *renderPicture(CALayer *top)
{
	CGLSetCurrentContext(readbackContext);
	glBindFramebuffer(GL_FRAMEBUFFER, readbackFramebuffer);
	glViewport(0, 0, SIZE, SIZE);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, SIZE, 0, SIZE, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	CARenderer *renderer = [CARenderer rendererWithCGLContext: readbackContext options: nil];
	[CATransaction flush];
	renderer.layer = top;
	renderer.bounds = CGRectMake(0, 0, SIZE, SIZE);
	[renderer beginFrameAtTime: CACurrentMediaTime() timeStamp: NULL];
	if ([renderer respondsToSelector: @selector(addUpdateRect:)])
		[renderer addUpdateRect: renderer.bounds];
	[renderer render];
	[renderer endFrame];
	renderer.layer = nil;

	glFinish();
	glReadPixels(0, 0, SIZE, SIZE, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	CGLSetCurrentContext(NULL);
	return pixels;
}

// Red channel of the picture at a column and a row counted from the bottom.
static int redAt(const uint8_t *picture, int x, int y)
{
	return picture[(y * SIZE + x) * 4];
}

// A red sublayer with a 2x scale, frame (2,2 4x4) in an 8x8 root, flexible in
// both sizes; the root grows to 16x16. The transformed frame becomes 12x12,
// so the red area covers 2..14 and not the whole picture.
static void checkRendering(void)
{
	if (!setUpReadback()) {
		printf("SKIP: no OpenGL context for the CARenderer readback\n");
		return;
	}
	[CATransaction begin];
	[CATransaction setDisableActions: YES];
	CALayer *root = [CALayer layer];
	root.anchorPoint = CGPointMake(0, 0);
	root.bounds = CGRectMake(0, 0, 8, 8);
	CALayer *child = [CALayer layer];
	child.bounds = CGRectMake(0, 0, 2, 2);
	child.position = CGPointMake(4, 4);
	child.transform = CATransform3DMakeScale(2, 2, 1);
	CGColorRef red = CGColorCreateGenericRGB(1, 0, 0, 1);
	child.backgroundColor = red;
	CGColorRelease(red);
	[root addSublayer: child];
	CALayer *plain = [CALayer layer];
	plain.anchorPoint = CGPointMake(0, 0);
	plain.bounds = CGRectMake(0, 0, SIZE, SIZE);
	CGColorRef red2 = CGColorCreateGenericRGB(1, 0, 0, 1);
	plain.backgroundColor = red2;
	CGColorRelease(red2);
	if (redAt(renderPicture(plain), SIZE / 2, SIZE / 2) == 0) {
		[CATransaction commit];
		printf("SKIP: the CARenderer draws nothing here, not even a plain layer\n");
		return;
	}
	child.autoresizingMask = kCALayerWidthSizable | kCALayerHeightSizable;
	root.bounds = CGRectMake(0, 0, SIZE, SIZE);
	[CATransaction commit];

	CHECK(rectNear(child.frame, 2, 2, 12, 12), "the autoresized frame of the scaled sublayer is " RECT_FMT, RECT_ARGS(child.frame));
	const uint8_t *picture = renderPicture(root);
	CHECK(redAt(picture, SIZE / 2, SIZE / 2) >= 250, "the middle is red (%d)", redAt(picture, SIZE / 2, SIZE / 2));
	CHECK(redAt(picture, 1, SIZE / 2) == 0 && redAt(picture, SIZE - 2, SIZE / 2) == 0, "the left and right edges are not (%d, %d)",
		redAt(picture, 1, SIZE / 2), redAt(picture, SIZE - 2, SIZE / 2));
	CHECK(redAt(picture, SIZE / 2, 1) == 0 && redAt(picture, SIZE / 2, SIZE - 2) == 0, "nor the top and bottom edges (%d, %d)",
		redAt(picture, SIZE / 2, 1), redAt(picture, SIZE / 2, SIZE - 2));
}

int main(void)
{
	signal(SIGALRM, watchdog);
	alarm(50);
	@autoreleasepool {
		[CATransaction begin];
		[CATransaction setDisableActions: YES];
		checkFrame();
		checkSetFrame();
		[CATransaction commit];
		checkAutoresizing();
		checkKeyPaths();
		checkConvert();
		checkRendering();
	}
	printf(failures ? "%d FAILED\n" : "ALL PASSED\n", failures);
	return failures ? 1 : 0;
}
