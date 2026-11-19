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
 * Rounded corners of layers drawn by CARenderer: cornerRadius with
 * masksToBounds clips the contents and the sublayers to a rounded rectangle,
 * cornerCurve selects circular or continuous corners, maskedCorners selects
 * the rounded corners, and the border covers the sublayers.
 *
 * Each case builds a 400x400 tree, draws it with an offscreen CARenderer,
 * reads it back and compares every pixel with the expected shape. The curve
 * is anti-aliased, so pixels closer than 1.5 pixels to an edge are not
 * compared; the others must be fully opaque in the expected colour inside,
 * and transparent outside.
 *
 * Observed on macOS 26.7.3 (x86_64) with -renderInContext: (the offscreen
 * CARenderer draws nothing there, so that is what the test uses where it
 * has to): the clip is anti-aliased circular quarter-circle corners, nested
 * rounded clips intersect, a rotated layer clips to a rotated rounded
 * rectangle, a layer without masksToBounds does not round its sublayers or
 * contents (its own background is rounded), the border is drawn over the
 * sublayers, and continuous corners leave the straight edges later (about
 * 1.53 times the radius from the corner) and cut slightly more off the
 * corner. -renderInContext: ignores maskedCorners, so that case only runs
 * with the renderer.
 *
 * Needs OpenGL. No ARC.
 */
#define GL_GLEXT_PROTOTYPES 1
#define GL_SILENCE_DEPRECATION 1
#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

@interface CARenderer (UpdateRect)
- (void)addUpdateRect:(CGRect)rect;
@end

#define SIZE 400

enum { NONE, RED, GREEN, BLUE };

static CGLContextObj context;
static GLuint framebuffer;
static uint8_t pixels[SIZE * SIZE * 4];
static BOOL useBitmap;
static int failures;


static void watchdog(int sig)
{
	static const char msg[] = "FAIL: timed out\n";
	(void) sig;
	write(1, msg, sizeof(msg) - 1);
	_exit(1);
}


static BOOL setUpGL(void)
{
	CGLPixelFormatAttribute attributes[] = { 0 };
	CGLPixelFormatObj format = NULL;
	GLint screens = 0;
	if (CGLChoosePixelFormat(attributes, &format, &screens) != kCGLNoError || format == NULL)
		return NO;
	if (CGLCreateContext(format, NULL, &context) != kCGLNoError)
		return NO;
	CGLReleasePixelFormat(format);
	CGLSetCurrentContext(context);
	GLuint renderbuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glGenRenderbuffers(1, &renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, SIZE, SIZE);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);
	return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

static BOOL anythingDrawn(void)
{
	for (int i = 0; i < SIZE * SIZE; i++)
		if (pixels[i * 4 + 3] > 128)
			return YES;
	return NO;
}

static void renderOnce(CALayer *root)
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glViewport(0, 0, SIZE, SIZE);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, SIZE, 0, SIZE, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	CARenderer *renderer = [CARenderer rendererWithCGLContext: context options: nil];
	[CATransaction flush];
	renderer.layer = root;
	renderer.bounds = CGRectMake(0, 0, SIZE, SIZE);
	[renderer beginFrameAtTime: CACurrentMediaTime() timeStamp: NULL];
	if ([renderer respondsToSelector: @selector(addUpdateRect:)])
		[renderer addUpdateRect: renderer.bounds];
	[renderer render];
	[renderer endFrame];
	renderer.layer = nil;
	glFinish();
	// Row 0 is y = 0, the bottom of the picture.
	glReadPixels(0, 0, SIZE, SIZE, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}

// -renderInContext: into a bitmap whose rows are copied bottom first, so
// that pixels[] is y up in both paths.
static void renderBitmap(CALayer *root)
{
	static uint8_t data[SIZE * SIZE * 4];
	memset(data, 0, sizeof(data));
	CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
	CGContextRef bitmap = CGBitmapContextCreate(data, SIZE, SIZE, 8, SIZE * 4, space,
	                                            (CGBitmapInfo) kCGImageAlphaPremultipliedLast);
	[root performSelector: @selector(renderInContext:) withObject: (id) bitmap];
	CGContextRelease(bitmap);
	CGColorSpaceRelease(space);
	for (int y = 0; y < SIZE; y++)
		memcpy(pixels + y * SIZE * 4, data + (SIZE - 1 - y) * SIZE * 4, SIZE * 4);
}

static void render(CALayer *root)
{
	if (useBitmap) {
		renderBitmap(root);
		return;
	}
	for (int attempt = 0; attempt < 20; attempt++) {
		[[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.02]];
		renderOnce(root);
		if (anythingDrawn())
			return;
	}
}


static CALayer *rootLayer(void)
{
	CALayer *root = [CALayer layer];
	root.anchorPoint = CGPointZero;
	root.frame = CGRectMake(0, 0, SIZE, SIZE);
	return root;
}

static CGColorRef color(int c)
{
	return CGColorCreateGenericRGB(c == RED, c == GREEN, c == BLUE, 1);
}

// A layer with its anchor at the corner of its frame, filled with a colour.
static CALayer *filled(CGRect frame, int c)
{
	CALayer *layer = [CALayer layer];
	layer.anchorPoint = CGPointZero;
	layer.frame = frame;
	CGColorRef rgb = color(c);
	layer.backgroundColor = rgb;
	CGColorRelease(rgb);
	return layer;
}

static CALayer *clipLayer(CGRect frame, int c, CGFloat radius)
{
	CALayer *layer = filled(frame, c);
	layer.cornerRadius = radius;
	layer.masksToBounds = YES;
	return layer;
}

static CGImageRef redImage(void)
{
	static uint8_t data[4 * 4 * 4];
	for (int i = 0; i < 16; i++) {
		data[i * 4] = 255;
		data[i * 4 + 3] = 255;
	}
	CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
	CGContextRef bitmap = CGBitmapContextCreate(data, 4, 4, 8, 16, space,
	                                            (CGBitmapInfo) kCGImageAlphaPremultipliedLast);
	CGImageRef image = CGBitmapContextCreateImage(bitmap);
	CGContextRelease(bitmap);
	CGColorSpaceRelease(space);
	return image;
}

// A rounded rectangle (y up) with a radius per corner: bottom left, bottom
// right, top left, top right. Rotated by angle about its centre.
typedef struct {
	double x0, y0, x1, y1;
	double r[4];
	double angle;
} Shape;

static Shape shape(double x0, double y0, double x1, double y1, double r)
{
	Shape s = { x0, y0, x1, y1, { r, r, r, r }, 0 };
	return s;
}

// Signed distance to the shape: negative inside.
static double distance(const Shape *s, double px, double py)
{
	double cx = (s->x0 + s->x1) / 2, cy = (s->y0 + s->y1) / 2;
	double dx = px - cx, dy = py - cy;
	double c = cos(-s->angle), sn = sin(-s->angle);
	double lx = dx * c - dy * sn, ly = dx * sn + dy * c;
	double r = s->r[(ly > 0 ? 2 : 0) + (lx > 0 ? 1 : 0)];
	double qx = fabs(lx) - ((s->x1 - s->x0) / 2 - r);
	double qy = fabs(ly) - ((s->y1 - s->y0) / 2 - r);
	double outside = hypot(MAX(qx, 0), MAX(qy, 0));
	return outside + MIN(MAX(qx, qy), 0) - r;
}

// The renderer clears to opaque black, so a black pixel is as empty as a
// transparent one.
static BOOL empty(const uint8_t *p)
{
	return p[3] < 5 || p[0] + p[1] + p[2] < 60;
}

// How much of a pixel is covered, 0 to 255: the strongest channel of a
// premultiplied red, green or blue pixel.
static int coverage(const uint8_t *p)
{
	return MAX(p[0], MAX(p[1], p[2]));
}

static int classify(const uint8_t *p)
{
	if (empty(p))
		return NONE;
	if (p[3] < 250)
		return -1;
	if (p[0] > 150 && p[1] < 120 && p[2] < 120) return RED;
	if (p[1] > 150 && p[0] < 120 && p[2] < 120) return GREEN;
	if (p[2] > 150 && p[0] < 120 && p[1] < 120) return BLUE;
	return -1;
}

static const char *const colorNames[] = { "none", "red", "green", "blue" };

// A case says which colour a pixel centre has and how far it is from the
// nearest edge of the expected picture.
typedef int (*Expect)(double x, double y, double *margin);

static void compare(const char *name, Expect expect)
{
	int wrong = 0, shown = 0;
	for (int y = 0; y < SIZE; y++) {
		for (int x = 0; x < SIZE; x++) {
			double margin = 0;
			int want = expect(x + 0.5, y + 0.5, &margin);
			if (margin < 1.5)
				continue;
			const uint8_t *p = pixels + (y * SIZE + x) * 4;
			int got = classify(p);
			if (got != want) {
				wrong++;
				if (shown++ < 3)
					printf("NOTE: %s: (%d,%d) is %d %d %d %d, expected %s\n", name, x, y, p[0], p[1],
					       p[2], p[3], colorNames[want]);
			}
		}
	}
	if (wrong == 0) {
		printf("PASS: %s\n", name);
		return;
	}
	failures++;
	printf("FAIL: %s: %d pixels differ\n", name, wrong);
}

static void draw(CALayer *root)
{
	render(root);
}

// Smallest distance to an edge of the listed shapes.
static double edge2(const Shape *a, const Shape *b, double x, double y)
{
	return MIN(fabs(distance(a, x, y)), fabs(distance(b, x, y)));
}

static Shape unit, inner, outerSh, cornerShapes[4], rotated;

static int expectUnit(double x, double y, double *m)
{
	*m = fabs(distance(&unit, x, y));
	return distance(&unit, x, y) < 0 ? RED : NONE;
}

static int expectUnitGreen(double x, double y, double *m)
{
	*m = fabs(distance(&unit, x, y));
	return distance(&unit, x, y) < 0 ? GREEN : NONE;
}

static int expectSquareRed(double x, double y, double *m)
{
	*m = MIN(MIN(fabs(x - 100), fabs(x - 200)), MIN(fabs(y - 100), fabs(y - 200)));
	return x > 100 && x < 200 && y > 100 && y < 200 ? RED : NONE;
}

static int expectCorner(Shape *s, double x, double y, double *m)
{
	*m = fabs(distance(s, x, y));
	return distance(s, x, y) < 0 ? RED : NONE;
}
static int expectCorner0(double x, double y, double *m) { return expectCorner(&cornerShapes[0], x, y, m); }
static int expectCorner1(double x, double y, double *m) { return expectCorner(&cornerShapes[1], x, y, m); }
static int expectCorner2(double x, double y, double *m) { return expectCorner(&cornerShapes[2], x, y, m); }
static int expectCorner3(double x, double y, double *m) { return expectCorner(&cornerShapes[3], x, y, m); }

static int expectRotated(double x, double y, double *m)
{
	*m = fabs(distance(&rotated, x, y));
	return distance(&rotated, x, y) < 0 ? RED : NONE;
}

static int expectNested(double x, double y, double *m)
{
	*m = edge2(&outerSh, &inner, x, y);
	if (distance(&outerSh, x, y) >= 0)
		return NONE;
	return distance(&inner, x, y) < 0 ? RED : GREEN;
}

// A border of 10 over a red sublayer, around a rounded clip.
static Shape borderOuter, borderInner;
static int expectBorder(double x, double y, double *m)
{
	*m = edge2(&borderOuter, &borderInner, x, y);
	if (distance(&borderOuter, x, y) >= 0)
		return NONE;
	return distance(&borderInner, x, y) < 0 ? RED : BLUE;
}

static void run(const char *name, CALayer *root, Expect expect)
{
	draw(root);
	compare(name, expect);
}

static BOOL rendererOnly(const char *name)
{
	if (!useBitmap)
		return YES;
	printf("SKIP: %s (-renderInContext: ignores it)\n", name);
	return NO;
}

// The distance from the left edge of the shapes (x = 100) to the first pixel
// of the row y that is at least half covered.
static int insetAtRow(int y)
{
	int x = 100;
	while (x < 160 && coverage(pixels + (y * SIZE + x) * 4) < 128)
		x++;
	return x - 100;
}

static void curveCases(void)
{
	// Circular corners follow a quarter circle; continuous corners start
	// further from the corner and cut slightly more off it.
	for (int continuous = 0; continuous < 2; continuous++) {
		CALayer *root = rootLayer();
		CALayer *layer = clipLayer(CGRectMake(100, 100, 100, 100), GREEN, 40);
		if (continuous)
			layer.cornerCurve = kCACornerCurveContinuous;
		[layer addSublayer: filled(CGRectMake(-20, -20, 200, 200), RED)];
		[root addSublayer: layer];
		draw(root);
		const int rows[] = { 0, 10, 20, 30 };
		const int circular[] = { 33, 13, 5, 1 }, smooth[] = { 38, 13, 5, 2 };
		BOOL ok = YES;
		for (int i = 0; i < 4; i++) {
			int got = insetAtRow(100 + rows[i]), want = (continuous ? smooth : circular)[i];
			if (abs(got - want) > 2) {
				ok = NO;
				printf("NOTE: %s curve: row %d starts at %d, expected %d\n",
				       continuous ? "continuous" : "circular", rows[i], got, want);
			}
		}
		// Along the left edge the continuous curve is still bending at a
		// distance of the radius from the corner; the circular one is not.
		int alpha = coverage(pixels + (140 * SIZE + 100) * 4);
		if (continuous ? !(alpha > 150 && alpha < 240) : alpha < 250) {
			ok = NO;
			printf("NOTE: edge alpha %d one radius from the corner\n", alpha);
		}
		const char *name = continuous ? "continuous corners (cornerCurve)" : "circular corners";
		if (ok)
			printf("PASS: %s\n", name);
		else {
			failures++;
			printf("FAIL: %s\n", name);
		}
	}
}

int main(void)
{
	signal(SIGALRM, watchdog);
	alarm(55);

	setvbuf(stdout, NULL, _IOLBF, 0);
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	if (!setUpGL()) {
		printf("FAIL: no offscreen OpenGL framebuffer\n");
		return 1;
	}

	CALayer *probe = filled(CGRectMake(0, 0, SIZE, SIZE), RED);
	CALayer *probeRoot = rootLayer();
	[probeRoot addSublayer: probe];
	render(probeRoot);
	if (!anythingDrawn()) {
		if (![probe respondsToSelector: @selector(renderInContext:)]) {
			printf("FAIL: the offscreen CARenderer draws nothing\n");
			return 1;
		}
		printf("NOTE: the offscreen CARenderer draws nothing; using -renderInContext:\n");
		useBitmap = YES;
	}

	unit = shape(100, 100, 200, 200, 50);
	// 1. A circle-cornered clip rounds the sublayers.
	{
		CALayer *root = rootLayer();
		CALayer *layer = clipLayer(CGRectMake(100, 100, 100, 100), GREEN, 50);
		[layer addSublayer: filled(CGRectMake(-20, -20, 200, 200), RED)];
		[root addSublayer: layer];
		run("masksToBounds clips sublayers to cornerRadius 50", root, expectUnit);
	}
	unit = shape(100, 100, 200, 200, 20);
	{
		CALayer *root = rootLayer();
		CALayer *layer = clipLayer(CGRectMake(100, 100, 100, 100), GREEN, 20);
		[layer addSublayer: filled(CGRectMake(-20, -20, 200, 200), RED)];
		[root addSublayer: layer];
		run("masksToBounds clips sublayers to cornerRadius 20", root, expectUnit);
	}
	// 2. Contents are clipped too.
	{
		CGImageRef image = redImage();
		CALayer *root = rootLayer();
		CALayer *layer = [CALayer layer];
		layer.anchorPoint = CGPointZero;
		layer.frame = CGRectMake(100, 100, 100, 100);
		layer.contents = (id) image;
		layer.cornerRadius = 20;
		layer.masksToBounds = YES;
		[root addSublayer: layer];
		run("masksToBounds clips contents to cornerRadius", root, expectUnit);
		layer.masksToBounds = NO;
		run("contents are not rounded without masksToBounds", root, expectSquareRed);
		CGImageRelease(image);
	}
	// 3. Without masksToBounds only the layer's own background is rounded.
	{
		CALayer *root = rootLayer();
		CALayer *layer = filled(CGRectMake(100, 100, 100, 100), GREEN);
		layer.cornerRadius = 20;
		[root addSublayer: layer];
		run("background follows cornerRadius", root, expectUnitGreen);
		[layer addSublayer: filled(CGRectMake(0, 0, 100, 100), RED)];
		run("sublayers are not rounded without masksToBounds", root, expectSquareRed);
	}
	// 4. Nested clips intersect.
	outerSh = shape(100, 100, 200, 200, 50);
	inner = shape(130, 130, 230, 230, 40);
	{
		CALayer *root = rootLayer();
		CALayer *outer = clipLayer(CGRectMake(100, 100, 100, 100), GREEN, 50);
		CALayer *in = clipLayer(CGRectMake(30, 30, 100, 100), BLUE, 40);
		[in addSublayer: filled(CGRectMake(-50, -50, 300, 300), RED)];
		[outer addSublayer: in];
		[root addSublayer: outer];
		run("nested rounded clips intersect", root, expectNested);
	}
	// 5. A rotated layer clips to a rotated rounded rectangle.
	rotated = shape(150, 150, 250, 250, 50);
	rotated.angle = 0.5;
	{
		CALayer *root = rootLayer();
		CALayer *layer = clipLayer(CGRectMake(150, 150, 100, 100), GREEN, 50);
		layer.anchorPoint = CGPointMake(0.5, 0.5);
		layer.position = CGPointMake(200, 200);
		layer.transform = CATransform3DMakeRotation(0.5, 0, 0, 1);
		[layer addSublayer: filled(CGRectMake(-30, -30, 160, 160), RED)];
		[root addSublayer: layer];
		run("rotated layer with a rounded clip", root, expectRotated);
	}
	// 6. The border covers the sublayers and follows the corners.
	borderOuter = shape(100, 100, 200, 200, 30);
	borderInner = shape(110, 110, 190, 190, 20);
	{
		CALayer *root = rootLayer();
		CALayer *layer = clipLayer(CGRectMake(100, 100, 100, 100), GREEN, 30);
		CGColorRef blue = color(BLUE);
		layer.borderColor = blue;
		CGColorRelease(blue);
		layer.borderWidth = 10;
		[layer addSublayer: filled(CGRectMake(0, 0, 100, 100), RED)];
		[root addSublayer: layer];
		run("the border is drawn over the sublayers", root, expectBorder);
	}
	curveCases();

	// 7. maskedCorners selects the rounded corners (default: all four).
	BOOL hasMaskedCorners = [[CALayer layer] respondsToSelector: @selector(setMaskedCorners:)];
	{
		CALayer *layer = [CALayer layer];
		NSNumber *def = hasMaskedCorners ? [layer valueForKey: @"maskedCorners"] : nil;
		if ([def unsignedIntegerValue] == 15)
			printf("PASS: maskedCorners defaults to all four corners\n");
		else {
			failures++;
			printf("FAIL: maskedCorners defaults to %lu\n", (unsigned long) [def unsignedIntegerValue]);
		}
	}
	if (hasMaskedCorners && rendererOnly("maskedCorners")) {
		// bit 0 minX/minY (bottom left), 1 maxX/minY, 2 minX/maxY, 3 maxX/maxY
		static const Expect expects[4] = { expectCorner0, expectCorner1, expectCorner2, expectCorner3 };
		for (int bit = 0; bit < 4; bit++) {
			Shape s = shape(100, 100, 200, 200, 0);
			s.r[bit] = 30;
			cornerShapes[bit] = s;
			CALayer *root = rootLayer();
			CALayer *layer = clipLayer(CGRectMake(100, 100, 100, 100), GREEN, 30);
			[layer setValue: @(1 << bit) forKey: @"maskedCorners"];
			[layer addSublayer: filled(CGRectMake(0, 0, 100, 100), RED)];
			[root addSublayer: layer];
			char name[64];
			snprintf(name, sizeof(name), "maskedCorners rounds only corner %d", bit);
			run(name, root, expects[bit]);
		}
	}

	CGLSetCurrentContext(NULL);
	[pool release];
	printf("%s\n", failures == 0 ? "PASS" : "FAIL");
	return failures == 0 ? 0 : 1;
}
