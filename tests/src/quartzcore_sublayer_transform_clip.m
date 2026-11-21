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
 * A layer's sublayerTransform and masksToBounds, as drawn and as seen by
 * -convertPoint: / -convertRect:.
 *
 * Observed on macOS 26.7.3 (x86_64), with -renderInContext: and the
 * conversion methods:
 * - sublayerTransform applies to the sublayers about the layer's anchor
 *   point, in the layer's own coordinates (bounds origin included); a
 *   geometryFlipped layer applies it in its flipped coordinates, and the
 *   flip comes after it;
 * - the conversion methods follow the same geometry, a nil layer standing
 *   for the space the tree's root is placed in;
 * - a masksToBounds layer clips its sublayers to its bounds, whatever its
 *   sublayerTransform; nested clips intersect, and a rotated layer clips to
 *   its rotated bounds.
 * The offscreen CARenderer draws nothing on that machine (no Metal device),
 * so where -renderInContext: exists and the renderer draws nothing, the
 * drawing cases read the tree back through -renderInContext: instead.
 *
 * The last cases publish a tree whose root has a sublayerTransform through
 * a CAContext (private API, declared the way applications declare it) and
 * show it with a CALayerHost: what the context publishes must cover where
 * the transform moves the content. They need the in-process renderer and
 * are skipped where it draws nothing.
 *
 * Needs a display (X11 or Wayland) and OpenGL. No ARC.
 */
#define GL_GLEXT_PROTOTYPES 1
#define GL_SILENCE_DEPRECATION 1
#import <AppKit/AppKit.h>
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

typedef uint32_t CAContextID;

// Private API, declared the way applications declare it.
@interface CAContext : NSObject
+ (instancetype)contextWithCGSConnection:(CAContextID)connection
                                 options:(NSDictionary *)options;
@property(readonly) CAContextID contextId;
@property(retain) CALayer *layer;
- (void)invalidate;
@end

@interface CALayerHost : CALayer
@property CAContextID contextId;
@end

@interface CARenderer (UpdateRect)
- (void)addUpdateRect:(CGRect)rect;
@end

// Public API; declared here too so that the test builds against headers
// that lack it, and reports it missing at run time.
@interface CALayer (Conversion)
- (CGPoint)convertPoint:(CGPoint)point fromLayer:(CALayer *)layer;
- (CGPoint)convertPoint:(CGPoint)point toLayer:(CALayer *)layer;
- (CGRect)convertRect:(CGRect)rect fromLayer:(CALayer *)layer;
- (CGRect)convertRect:(CGRect)rect toLayer:(CALayer *)layer;
@end

#define SIZE 400

enum { RED, GREEN, BLUE, WHITE, COLORS };
static const char *const colorNames[COLORS] = { "red", "green", "blue", "white" };

typedef struct {
	int color;
	int x0, x1, y0, y1;
	int count; // 0: not checked
} Box;

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
	// Colour only: clipping must not depend on a stencil or depth buffer.
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
	CGLSetCurrentContext(context);
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

static void runLoopFor(NSTimeInterval seconds)
{
	[[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow: seconds]];
}

static void render(CALayer *root)
{
	if (useBitmap) {
		renderBitmap(root);
		return;
	}
	for (int attempt = 0; attempt < 10; attempt++) {
		runLoopFor(0.02);
		renderOnce(root);
		if (anythingDrawn())
			return;
	}
}

static int classify(const uint8_t *p)
{
	if (p[3] < 128)
		return -1;
	BOOL r = p[0] > 128, g = p[1] > 128, b = p[2] > 128;
	if (r && g && b) return WHITE;
	if (r && !g && !b) return RED;
	if (!r && g && !b) return GREEN;
	if (!r && !g && b) return BLUE;
	return -1;
}

static BOOL near(int a, int b)
{
	return abs(a - b) <= 1;
}

// Compares the box around each colour (and, where given, its pixel count
// within 3%) with the expected boxes; a colour not listed must not show.
static BOOL matches(const Box *expected, int count, char *shows, size_t size)
{
	int minX[COLORS], minY[COLORS], maxX[COLORS], maxY[COLORS], n[COLORS];
	for (int c = 0; c < COLORS; c++) {
		minX[c] = minY[c] = SIZE;
		maxX[c] = maxY[c] = -1;
		n[c] = 0;
	}
	for (int y = 0; y < SIZE; y++)
		for (int x = 0; x < SIZE; x++) {
			int c = classify(pixels + (y * SIZE + x) * 4);
			if (c < 0)
				continue;
			n[c]++;
			minX[c] = MIN(minX[c], x);
			maxX[c] = MAX(maxX[c], x);
			minY[c] = MIN(minY[c], y);
			maxY[c] = MAX(maxY[c], y);
		}

	BOOL ok = YES;
	shows[0] = 0;
	for (int c = 0; c < COLORS; c++) {
		const Box *want = NULL;
		for (int i = 0; i < count; i++)
			if (expected[i].color == c)
				want = &expected[i];
		BOOL drawn = n[c] > 0;
		if (drawn) {
			size_t used = strlen(shows);
			snprintf(shows + used, size - used, " %s x[%d,%d) y[%d,%d) n%d", colorNames[c], minX[c], maxX[c] + 1,
			         minY[c], maxY[c] + 1, n[c]);
		}
		if (want == NULL) {
			if (drawn)
				ok = NO;
			continue;
		}
		if (!drawn || !near(minX[c], want->x0) || !near(maxX[c] + 1, want->x1) || !near(minY[c], want->y0) ||
		    !near(maxY[c] + 1, want->y1))
			ok = NO;
		if (want->count > 0 && fabs((double) n[c] - want->count) > want->count * 0.03)
			ok = NO;
	}
	return ok;
}

static void report(const char *name, BOOL ok, const char *shows, const Box *expected, int count)
{
	if (ok) {
		printf("PASS: %s\n", name);
		return;
	}
	failures++;
	printf("FAIL: %s: shows%s; expected", name, shows[0] ? shows : " nothing");
	for (int i = 0; i < count; i++)
		printf(" %s x[%d,%d) y[%d,%d)", colorNames[expected[i].color], expected[i].x0, expected[i].x1,
		       expected[i].y0, expected[i].y1);
	printf("%s\n", count == 0 ? " nothing" : "");
}

static void expectBoxes(const char *name, CALayer *root, const Box *expected, int count)
{
	char shows[512];
	render(root);
	report(name, matches(expected, count, shows, sizeof(shows)), shows, expected, count);
}

static CALayer *rootLayer(void)
{
	CALayer *root = [CALayer layer];
	root.anchorPoint = CGPointZero;
	root.frame = CGRectMake(0, 0, SIZE, SIZE);
	return root;
}

static CALayer *solid(CGRect frame, int color)
{
	CALayer *layer = [CALayer layer];
	layer.anchorPoint = CGPointZero;
	layer.frame = frame;
	if (color >= 0) {
		CGColorRef cgColor = CGColorCreateGenericRGB(color == RED || color == WHITE, color == GREEN || color == WHITE,
		                                             color == BLUE || color == WHITE, 1);
		layer.backgroundColor = cgColor;
		CGColorRelease(cgColor);
	}
	return layer;
}

static BOOL nearlyEqual(CGFloat a, CGFloat b)
{
	return fabs(a - b) < 0.001;
}

static void checkPoint(const char *name, CGPoint got, CGFloat x, CGFloat y)
{
	if (nearlyEqual(got.x, x) && nearlyEqual(got.y, y)) {
		printf("PASS: %s\n", name);
		return;
	}
	failures++;
	printf("FAIL: %s: (%g,%g), expected (%g,%g)\n", name, got.x, got.y, x, y);
}

static void checkRect(const char *name, CGRect got, CGFloat x, CGFloat y, CGFloat w, CGFloat h)
{
	if (nearlyEqual(got.origin.x, x) && nearlyEqual(got.origin.y, y) && nearlyEqual(got.size.width, w) && nearlyEqual(got.size.height, h)) {
		printf("PASS: %s\n", name);
		return;
	}
	failures++;
	printf("FAIL: %s: (%g,%g %gx%g), expected (%g,%g %gx%g)\n", name, got.origin.x, got.origin.y, got.size.width,
	       got.size.height, x, y, w, h);
}

static CATransform3D namedTransform(const char *name)
{
	if (strcmp(name, "translate(30,40)") == 0)
		return CATransform3DMakeTranslation(30, 40, 0);
	if (strcmp(name, "scale(2,2)") == 0)
		return CATransform3DMakeScale(2, 2, 1);
	if (strcmp(name, "rotate90") == 0)
		return CATransform3DMakeRotation(M_PI_2, 0, 0, 1);
	if (strcmp(name, "scale(.5,.5)+translate(20,0)") == 0)
		return CATransform3DConcat(CATransform3DMakeScale(0.5, 0.5, 1), CATransform3DMakeTranslation(20, 0, 0));
	return CATransform3DIdentity;
}

static BOOL canConvert(void)
{
	static int known = -1;
	if (known < 0) {
		known = [CALayer instancesRespondToSelector: @selector(convertPoint:fromLayer:)] &&
		        [CALayer instancesRespondToSelector: @selector(convertRect:toLayer:)];
		if (!known) {
			printf("FAIL: CALayer has no convertPoint:/convertRect: methods\n");
			failures++;
		}
	}
	return known;
}

// A 200x200 parent at (100,100) with a 20x10 red sublayer at (120,130), and
// the parent's sublayerTransform. Expected values from macOS 26.7.3.
static void parentCases(void)
{
	static const struct {
		const char *variant, *transform;
		CGFloat v[12]; // child (5,5) in root; root (200,200) in child; child bounds in root;
		               // parent (120,130) in root; child (0,0) in parent
	} cases[] = {
		{ "anchor.5", "identity", { 225, 235, -20, -30, 220, 230, 20, 10, 220, 230, 120, 130 } },
		{ "anchor.5", "translate(30,40)", { 255, 275, -50, -70, 250, 270, 20, 10, 220, 230, 150, 170 } },
		{ "anchor.5", "scale(2,2)", { 250, 270, -20, -30, 240, 260, 40, 20, 220, 230, 140, 160 } },
		{ "anchor.5", "rotate90", { 165, 225, -20, -30, 160, 220, 10, 20, 220, 230, 70, 120 } },
		{ "anchor.5", "scale(.5,.5)+translate(20,0)", { 232.5, 217.5, -60, -30, 230, 215, 10, 5, 220, 230, 130, 115 } },
		{ "anchor0", "identity", { 225, 235, -20, -30, 220, 230, 20, 10, 220, 230, 120, 130 } },
		{ "anchor0", "translate(30,40)", { 255, 275, -50, -70, 250, 270, 20, 10, 220, 230, 150, 170 } },
		{ "anchor0", "scale(2,2)", { 350, 370, -70, -80, 340, 360, 40, 20, 220, 230, 240, 260 } },
		{ "anchor0", "rotate90", { -35, 225, -20, -230, -40, 220, 10, 20, 220, 230, -130, 120 } },
		{ "anchor0", "scale(.5,.5)+translate(20,0)", { 182.5, 167.5, 40, 70, 180, 165, 10, 5, 220, 230, 80, 65 } },
		{ "boundsorigin(10,20)", "identity", { 215, 215, -10, -10, 210, 210, 20, 10, 210, 210, 120, 130 } },
		{ "boundsorigin(10,20)", "translate(30,40)", { 245, 255, -40, -50, 240, 250, 20, 10, 210, 210, 150, 170 } },
		{ "boundsorigin(10,20)", "scale(2,2)", { 230, 230, -10, -10, 220, 220, 40, 20, 210, 210, 130, 140 } },
		{ "boundsorigin(10,20)", "rotate90", { 185, 215, -10, -10, 180, 210, 10, 20, 210, 210, 100, 130 } },
		{ "boundsorigin(10,20)", "scale(.5,.5)+translate(20,0)", { 227.5, 207.5, -50, -10, 225, 205, 10, 5, 210, 210, 135, 125 } },
		{ "flipped", "identity", { 225, 165, -20, -30, 220, 160, 20, 10, 220, 170, 120, 130 } },
		{ "flipped", "translate(30,40)", { 255, 125, -50, -70, 250, 120, 20, 10, 220, 170, 150, 170 } },
		{ "flipped", "scale(2,2)", { 250, 130, -20, -30, 240, 120, 40, 20, 220, 170, 140, 160 } },
		{ "flipped", "rotate90", { 165, 175, -20, -30, 160, 160, 10, 20, 220, 170, 70, 120 } },
		{ "flipped", "scale(.5,.5)+translate(20,0)", { 232.5, 182.5, -60, -30, 230, 180, 10, 5, 220, 170, 130, 115 } },
	};

	for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		BOOL anchor0 = strcmp(cases[i].variant, "anchor0") == 0;
		BOOL origin = strcmp(cases[i].variant, "boundsorigin(10,20)") == 0;
		CALayer *root = rootLayer();
		CALayer *parent = [CALayer layer];
		parent.anchorPoint = anchor0 ? CGPointZero : CGPointMake(0.5, 0.5);
		parent.bounds = CGRectMake(origin ? 10 : 0, origin ? 20 : 0, 200, 200);
		parent.position = anchor0 ? CGPointMake(100, 100) : CGPointMake(200, 200);
		parent.geometryFlipped = strcmp(cases[i].variant, "flipped") == 0;
		parent.sublayerTransform = namedTransform(cases[i].transform);
		CALayer *child = solid(CGRectMake(120, 130, 20, 10), RED);
		[parent addSublayer: child];
		[root addSublayer: parent];

		const CGFloat *v = cases[i].v;
		char name[160];
		snprintf(name, sizeof(name), "sublayerTransform %s, parent %s: drawn", cases[i].transform, cases[i].variant);
		Box box = { RED, (int) floor(v[4]), (int) ceil(v[4] + v[6]), (int) floor(v[5]), (int) ceil(v[5] + v[7]), 0 };
		BOOL offscreen = box.x1 <= 0 || box.y1 <= 0 || box.x0 >= SIZE || box.y0 >= SIZE;
		expectBoxes(name, root, &box, offscreen ? 0 : 1);

		if (canConvert()) {
			snprintf(name, sizeof(name), "sublayerTransform %s, parent %s: child point in root", cases[i].transform,
			         cases[i].variant);
			checkPoint(name, [root convertPoint: CGPointMake(5, 5) fromLayer: child], v[0], v[1]);
			snprintf(name, sizeof(name), "sublayerTransform %s, parent %s: root point in child", cases[i].transform,
			         cases[i].variant);
			checkPoint(name, [child convertPoint: CGPointMake(200, 200) fromLayer: root], v[2], v[3]);
			snprintf(name, sizeof(name), "sublayerTransform %s, parent %s: child bounds in root", cases[i].transform,
			         cases[i].variant);
			checkRect(name, [root convertRect: child.bounds fromLayer: child], v[4], v[5], v[6], v[7]);
			snprintf(name, sizeof(name), "sublayerTransform %s, parent %s: parent point in root", cases[i].transform,
			         cases[i].variant);
			checkPoint(name, [parent convertPoint: CGPointMake(120, 130) toLayer: root], v[8], v[9]);
			snprintf(name, sizeof(name), "sublayerTransform %s, parent %s: child origin in parent", cases[i].transform,
			         cases[i].variant);
			checkPoint(name, [child convertPoint: CGPointZero toLayer: parent], v[10], v[11]);
		}
		[pool release];
	}
}

// A 200x100 parent whose frame is (100,100) whatever its anchor point, with
// a 40x20 red sublayer at (20,10): the anchor point the sublayerTransform
// uses, also in a flipped layer. Expected values from macOS 26.7.3.
static void anchorCases(void)
{
	static const struct {
		BOOL flipped;
		CGPoint anchor;
		BOOL rotate; // rotation by 180 degrees, otherwise scale by 0.5
		CGFloat v[6]; // child bounds in root; root (150,150) in child
	} cases[] = {
		{ 0, { 0, 0 }, 0, { 110, 105, 20, 10, 80, 90 } },
		{ 0, { 0, 0 }, 1, { 40, 70, 40, 20, -70, -60 } },
		{ 0, { 0.25, 0.25 }, 0, { 135, 117.5, 20, 10, 30, 65 } },
		{ 0, { 0.25, 0.25 }, 1, { 140, 120, 40, 20, 30, -10 } },
		{ 0, { 0.5, 0.5 }, 0, { 160, 130, 20, 10, -20, 40 } },
		{ 0, { 0.5, 0.5 }, 1, { 240, 170, 40, 20, 130, 40 } },
		{ 1, { 0, 0 }, 0, { 110, 185, 20, 10, 80, 90 } },
		{ 1, { 0, 0 }, 1, { 40, 210, 40, 20, -70, -60 } },
		{ 1, { 0.25, 0.25 }, 0, { 135, 172.5, 20, 10, 30, 65 } },
		{ 1, { 0.25, 0.25 }, 1, { 140, 160, 40, 20, 30, -10 } },
		{ 1, { 0.5, 0.5 }, 0, { 160, 160, 20, 10, -20, 40 } },
		{ 1, { 0.5, 0.5 }, 1, { 240, 110, 40, 20, 130, 40 } },
	};

	for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		CGPoint a = cases[i].anchor;
		CALayer *root = rootLayer();
		CALayer *parent = [CALayer layer];
		parent.anchorPoint = a;
		parent.bounds = CGRectMake(0, 0, 200, 100);
		parent.position = CGPointMake(100 + 200 * a.x, 100 + 100 * a.y);
		parent.geometryFlipped = cases[i].flipped;
		parent.sublayerTransform = cases[i].rotate ? CATransform3DMakeRotation(M_PI, 0, 0, 1)
		                                           : CATransform3DMakeScale(0.5, 0.5, 1);
		CALayer *child = solid(CGRectMake(20, 10, 40, 20), RED);
		[parent addSublayer: child];
		[root addSublayer: parent];

		const CGFloat *v = cases[i].v;
		char name[160];
		snprintf(name, sizeof(name), "%s about anchor (%g,%g), %s parent: drawn", cases[i].rotate ? "rotation" : "scale",
		         a.x, a.y, cases[i].flipped ? "flipped" : "plain");
		Box box = { RED, (int) lround(v[0]), (int) lround(v[0] + v[2]), (int) lround(v[1]), (int) lround(v[1] + v[3]), 0 };
		expectBoxes(name, root, &box, 1);
		if (canConvert()) {
			snprintf(name, sizeof(name), "%s about anchor (%g,%g), %s parent: child bounds in root",
			         cases[i].rotate ? "rotation" : "scale", a.x, a.y, cases[i].flipped ? "flipped" : "plain");
			checkRect(name, [root convertRect: child.bounds fromLayer: child], v[0], v[1], v[2], v[3]);
			snprintf(name, sizeof(name), "%s about anchor (%g,%g), %s parent: root point in child",
			         cases[i].rotate ? "rotation" : "scale", a.x, a.y, cases[i].flipped ? "flipped" : "plain");
			checkPoint(name, [child convertPoint: CGPointMake(150, 150) fromLayer: root], v[4], v[5]);
		}
		[pool release];
	}
}

static void otherGeometryCases(void)
{
	// Nested clip layers whose sublayerTransforms cancel their positions.
	{
		CALayer *root = rootLayer();
		CALayer *outer = solid(CGRectMake(42, 119, 200, 100), -1);
		outer.sublayerTransform = CATransform3DMakeTranslation(-42, -119, 0);
		CALayer *inner = solid(CGRectMake(52, 130, 100, 50), -1);
		inner.sublayerTransform = CATransform3DMakeTranslation(-52, -130, 0);
		CALayer *caret = solid(CGRectMake(62, 135, 4, 17), RED);
		[inner addSublayer: caret];
		[outer addSublayer: inner];
		[root addSublayer: outer];
		static const Box box = { RED, 62, 66, 135, 152, 0 };
		expectBoxes("nested translating sublayerTransforms: drawn", root, &box, 1);
		if (canConvert())
			checkPoint("nested translating sublayerTransforms: caret origin in root",
			           [root convertPoint: CGPointZero fromLayer: caret], 62, 135);
	}

	if (!canConvert())
		return;

	// A layer transform and a sublayerTransform together, in a plain and a
	// flipped root; a nil layer is the space the root is placed in.
	for (int rootFlipped = 0; rootFlipped < 2; rootFlipped++) {
		CALayer *root = rootLayer();
		root.geometryFlipped = rootFlipped;
		CALayer *parent = [CALayer layer];
		parent.bounds = CGRectMake(5, 5, 100, 100);
		parent.position = CGPointMake(200, 200);
		parent.transform = CATransform3DMakeRotation(M_PI_2, 0, 0, 1);
		parent.sublayerTransform = CATransform3DMakeTranslation(10, 0, 0);
		CALayer *child = solid(CGRectMake(5, 5, 30, 10), RED);
		[parent addSublayer: child];
		[root addSublayer: parent];
		const char *which = rootFlipped ? "flipped root" : "plain root";
		char name[160];
		snprintf(name, sizeof(name), "transform and sublayerTransform, %s: child bounds in root", which);
		checkRect(name, [root convertRect: child.bounds fromLayer: child], 240, 160, 10, 30);
		snprintf(name, sizeof(name), "transform and sublayerTransform, %s: child point in parent", which);
		checkPoint(name, [parent convertPoint: CGPointMake(1, 2) fromLayer: child], 16, 7);
		snprintf(name, sizeof(name), "transform and sublayerTransform, %s: child point to parent", which);
		checkPoint(name, [child convertPoint: CGPointMake(1, 2) toLayer: parent], 16, 7);
		snprintf(name, sizeof(name), "transform and sublayerTransform, %s: child rect to nil", which);
		checkRect(name, [child convertRect: CGRectMake(0, 0, 10, 10) toLayer: nil], 240, rootFlipped ? 230 : 160, 10, 10);
	}

	// Layers of different trees are related through their roots' space.
	CALayer *a = solid(CGRectMake(10, 20, 30, 40), RED);
	CALayer *b = solid(CGRectMake(100, 200, 30, 40), RED);
	checkPoint("layers of different trees", [a convertPoint: CGPointMake(1, 1) toLayer: b], -89, -179);
	checkPoint("point from nil", [a convertPoint: CGPointMake(1, 1) fromLayer: nil], -9, -19);
}

static void maskCases(void)
{
	// A blue 100x100 masking layer at (100,100) with a red sublayer at
	// (50,50) 100x100.
	{
		CALayer *root = rootLayer();
		CALayer *clip = solid(CGRectMake(100, 100, 100, 100), BLUE);
		clip.masksToBounds = YES;
		[clip addSublayer: solid(CGRectMake(50, 50, 100, 100), RED)];
		[root addSublayer: clip];
		static const Box simple[] = { { RED, 150, 200, 150, 200, 2500 }, { BLUE, 100, 200, 100, 200, 7500 } };
		expectBoxes("masksToBounds", root, simple, 2);

		clip.bounds = CGRectMake(20, 10, 100, 100);
		static const Box origin[] = { { RED, 130, 200, 140, 200, 4200 }, { BLUE, 100, 200, 100, 200, 5800 } };
		expectBoxes("masksToBounds, bounds origin (20,10)", root, origin, 2);

		clip.bounds = CGRectMake(0, 0, 100, 100);
		clip.geometryFlipped = YES;
		static const Box flipped[] = { { RED, 150, 200, 100, 150, 2500 }, { BLUE, 100, 200, 100, 200, 7500 } };
		expectBoxes("masksToBounds, flipped", root, flipped, 2);

		clip.geometryFlipped = NO;
		clip.sublayerTransform = CATransform3DMakeTranslation(-30, -30, 0);
		static const Box moved[] = { { RED, 120, 200, 120, 200, 6400 }, { BLUE, 100, 200, 100, 200, 3600 } };
		expectBoxes("masksToBounds, sublayerTransform translation", root, moved, 2);

		// Rotated about the anchor point (0,0): a corner of the rotated
		// square reaches into the bounds.
		clip.sublayerTransform = CATransform3DMakeRotation(M_PI / 4, 0, 0, 1);
		static const Box rotated[] = { { RED, 100, 129, 171, 200, 0 }, { BLUE, 100, 200, 100, 200, 0 } };
		expectBoxes("masksToBounds, sublayerTransform rotation", root, rotated, 2);
	}

	// Nested: blue outer (100,100) 150x150 and white inner at (50,50)
	// 150x150 both mask; a green sublayer of the inner one is far larger.
	{
		CALayer *root = rootLayer();
		CALayer *outer = solid(CGRectMake(100, 100, 150, 150), BLUE);
		outer.masksToBounds = YES;
		CALayer *inner = solid(CGRectMake(50, 50, 150, 150), WHITE);
		inner.masksToBounds = YES;
		[inner addSublayer: solid(CGRectMake(-100, -100, 400, 400), GREEN)];
		[outer addSublayer: inner];
		[root addSublayer: outer];
		static const Box both[] = { { GREEN, 150, 250, 150, 250, 10000 }, { BLUE, 100, 250, 100, 250, 12500 } };
		expectBoxes("nested masksToBounds", root, both, 2);
		inner.masksToBounds = NO;
		static const Box outerOnly[] = { { GREEN, 100, 250, 100, 250, 22500 } };
		expectBoxes("nested, only the outer layer masks", root, outerOnly, 1);
		inner.masksToBounds = YES;
		outer.masksToBounds = NO;
		static const Box innerOnly[] = { { GREEN, 150, 300, 150, 300, 22500 }, { BLUE, 100, 250, 100, 250, 12500 } };
		expectBoxes("nested, only the inner layer masks", root, innerOnly, 2);
	}

	// A masking layer rotated by 45 degrees clips to its rotated bounds;
	// an axis-aligned masking sublayer inside it intersects with that.
	{
		CALayer *root = rootLayer();
		CALayer *clip = [CALayer layer];
		clip.bounds = CGRectMake(0, 0, 100, 100);
		clip.position = CGPointMake(200, 200);
		clip.transform = CATransform3DMakeRotation(M_PI / 4, 0, 0, 1);
		clip.masksToBounds = YES;
		[clip addSublayer: solid(CGRectMake(-200, -200, 500, 500), RED)];
		[root addSublayer: clip];
		static const Box diamond[] = { { RED, 130, 270, 130, 270, 10000 } };
		expectBoxes("rotated masksToBounds", root, diamond, 1);

		CALayer *inner = solid(CGRectMake(50, 0, 100, 100), -1);
		inner.masksToBounds = YES;
		[inner addSublayer: solid(CGRectMake(-200, -200, 500, 500), GREEN)];
		[clip addSublayer: inner];
		static const Box half[] = { { RED, 130, 235, 130, 235, 4935 }, { GREEN, 165, 270, 165, 270, 5005 } };
		expectBoxes("rotated masksToBounds with a masking sublayer", root, half, 2);
	}
}

// The host's tree: a 400x400 layer with a host at (200,200).
static CALayer *hostTree(CAContextID contextId)
{
	CALayer *top = rootLayer();
	CALayerHost *host = [CALayerHost layer];
	host.anchorPoint = CGPointZero;
	host.frame = CGRectMake(200, 200, 0, 0);
	host.contextId = contextId;
	[top addSublayer: host];
	return top;
}

// What a context shows when its zero-bounds root has a sublayerTransform:
// the area published must follow the transformed content.
static void remoteCases(void)
{
	if (useBitmap) {
		printf("SKIP: remote context cases (the in-process renderer draws nothing)\n");
		return;
	}
	static const struct {
		const char *name;
		int expected[4]; // relative to the host's position, y up
	} cases[] = {
		{ "remote context, root without sublayerTransform", { 0, 100, 0, 60 } },
		{ "remote context, root sublayerTransform translation (40,30)", { 40, 140, 30, 90 } },
		{ "remote context, root sublayerTransform translation (-50,-30)", { -50, 50, -30, 30 } },
		{ "remote context, root sublayerTransform scale 2", { 0, 200, 0, 120 } },
	};
	const CATransform3D transforms[] = {
		CATransform3DIdentity,
		CATransform3DMakeTranslation(40, 30, 0),
		CATransform3DMakeTranslation(-50, -30, 0),
		CATransform3DMakeScale(2, 2, 1),
	};

	for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		CALayer *root = [CALayer layer];
		root.anchorPoint = CGPointZero;
		root.bounds = CGRectZero;
		root.position = CGPointZero;
		root.sublayerTransform = transforms[i];
		[root addSublayer: solid(CGRectMake(0, 0, 100, 60), RED)];
		CAContext *remote = [CAContext contextWithCGSConnection: 0 options: @{}];
		remote.layer = root;
		if (remote == nil || remote.contextId == 0) {
			printf("FAIL: %s: no context\n", cases[i].name);
			failures++;
			[pool release];
			continue;
		}
		CALayer *top = hostTree(remote.contextId);
		const int *e = cases[i].expected;
		Box box = { RED, 200 + e[0], 200 + e[1], 200 + e[2], 200 + e[3], 0 };
		char shows[512];
		BOOL ok = NO;
		for (int attempt = 0; attempt < 40 && !ok; attempt++) {
			runLoopFor(0.05);
			renderOnce(top);
			ok = matches(&box, 1, shows, sizeof(shows));
		}
		report(cases[i].name, ok, shows, &box, 1);
		[remote invalidate];
		[top setSublayers: nil];
		[pool release];
	}
}

int main(void)
{
	signal(SIGALRM, watchdog);
	alarm(55);

	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	[NSApplication sharedApplication];
	if (!setUpGL()) {
		printf("FAIL: no offscreen OpenGL framebuffer\n");
		return 1;
	}

	// Find out whether the offscreen renderer draws at all here.
	CALayer *probe = rootLayer();
	[probe addSublayer: solid(CGRectMake(10, 10, 20, 20), RED)];
	render(probe);
	if (!anythingDrawn()) {
		if (![probe respondsToSelector: @selector(renderInContext:)]) {
			printf("FAIL: the offscreen CARenderer draws nothing\n");
			return 1;
		}
		printf("NOTE: the offscreen CARenderer draws nothing; using -renderInContext:\n");
		useBitmap = YES;
	}

	parentCases();
	anchorCases();
	otherGeometryCases();
	maskCases();
	remoteCases();

	[pool release];
	printf("%s\n", failures == 0 ? "PASS" : "FAIL");
	return failures == 0 ? 0 : 1;
}
