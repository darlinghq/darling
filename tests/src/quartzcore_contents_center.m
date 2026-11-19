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
 * contentsCenter: nine-part stretching of a layer's contents.
 *
 * The contents are a 30x30 picture of 3x3 cells of 10 pixels, each cell in
 * its own colour. contentsCenter is a rectangle in the unit coordinates of
 * the part of the contents that contentsRect selects (y up like contentsRect,
 * measured from the top edge on a flipped layer). The part outside it, the
 * four corners and the four edges, keeps its size (pixels divided by
 * contentsScale), and the centre stretches to take up the rest of the
 * rectangle that contentsGravity gives the contents; edges stretch along
 * their length only. Each case builds a 400x400 tree, draws it with an
 * offscreen CARenderer, reads it back and compares each pixel with the cell
 * the nine-part mapping puts there. Pixels near the seams of the picture's
 * cells and near the edges of the nine parts are not compared, as the
 * stretched picture is filtered there.
 *
 * Observed on macOS 26.7.3 (x86_64) with -renderInContext: (the offscreen
 * CARenderer draws nothing there, so that is what the test uses where it
 * has to):
 * - the rule above, for gravity resize and resizeAspect (the contents are
 *   first fitted to the layer, then stretched in nine parts); gravities that
 *   place the contents at their own size show them unstretched;
 * - contentsScale 2 halves the corners; a corner pair that does not fit in
 *   the layer shrinks in proportion; a zero-size centre repeats the pixel
 *   line at its position; the default (0, 0, 1, 1) stretches everything.
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
#define CELL 10
#define IMG (3 * CELL)

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


static const uint8_t palette[9][3] = {
	{ 255, 0, 0 }, { 0, 255, 0 }, { 0, 0, 255 }, { 255, 255, 0 }, { 0, 255, 255 },
	{ 255, 0, 255 }, { 255, 255, 255 }, { 255, 128, 0 }, { 128, 0, 255 }
};

static CGImageRef createImage(void)
{
	static uint8_t data[IMG * IMG * 4];
	for (int y = 0; y < IMG; y++)
		for (int x = 0; x < IMG; x++) {
			const uint8_t *c = palette[(y / CELL) * 3 + x / CELL];
			uint8_t *p = data + (y * IMG + x) * 4;
			p[0] = c[0];
			p[1] = c[1];
			p[2] = c[2];
			p[3] = 255;
		}
	CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
	CGContextRef bitmap = CGBitmapContextCreate(data, IMG, IMG, 8, IMG * 4, space,
	                                            (CGBitmapInfo) kCGImageAlphaPremultipliedLast);
	CGImageRef image = CGBitmapContextCreateImage(bitmap);
	CGContextRelease(bitmap);
	CGColorSpaceRelease(space);
	return image;
}

// The palette index of an opaque pixel, -1 for an empty one, -2 for a
// colour that is not in the palette.
static int cellOf(const uint8_t *p)
{
	// The renderer clears to opaque black, so black is as empty as transparent.
	if (p[3] < 5 || p[0] + p[1] + p[2] < 60)
		return -1;
	if (p[3] < 250)
		return -2;
	int best = -2, bestDistance = 90;
	for (int c = 0; c < 9; c++) {
		int d = abs(p[0] - palette[c][0]) + abs(p[1] - palette[c][1]) + abs(p[2] - palette[c][2]);
		if (d < bestDistance) {
			bestDistance = d;
			best = c;
		}
	}
	return best;
}

typedef struct {
	const char *name;
	CGRect frame;          // in the 400x400 picture, y up
	CGRect center;         // contentsCenter
	CGRect rect;           // contentsRect
	NSString *gravity;     // nil: resize
	CGFloat scale;         // contentsScale
	BOOL flipped;          // geometryFlipped
} Case;

// One axis of the nine-part mapping: the destination range [d0, d1] of the
// contents, the source part [s0, s1] in pixels and the centre [c0, c1] in
// source pixels (c0 + ... ). Returns the source coordinate of the
// destination coordinate d, and the distance of d from the nearest edge
// between two of the parts.
typedef struct {
	double d0, d1, s0, s1, c0, c1, scale;
} Axis;

static double mapAxis(const Axis *a, double d, double *edge)
{
	double left = (a->c0 - a->s0) / a->scale, right = (a->s1 - a->c1) / a->scale;
	double width = a->d1 - a->d0;
	if (left + right > width) {
		double k = width / (left + right);
		left *= k;
		right *= k;
	}
	double b1 = a->d0 + left, b2 = a->d1 - right;
	*edge = MIN(MIN(fabs(d - a->d0), fabs(d - b1)), MIN(fabs(d - b2), fabs(d - a->d1)));
	if (d < b1)
		return a->s0 + (d - a->d0) * (a->c0 - a->s0) / MAX(left, 1e-9);
	if (d > b2)
		return a->c1 + (d - b2) * (a->s1 - a->c1) / MAX(right, 1e-9);
	if (b2 - b1 < 1e-9)
		return a->c0;
	return a->c0 + (d - b1) * (a->c1 - a->c0) / (b2 - b1);
}

// The destination rectangle of the contents inside the layer for the
// gravities used here.
static CGRect destinationRect(const Case *c, double partWidth, double partHeight)
{
	CGSize layer = c->frame.size;
	if (c->gravity == nil || [c->gravity isEqualToString: kCAGravityResize])
		return CGRectMake(0, 0, layer.width, layer.height);
	if ([c->gravity isEqualToString: kCAGravityResizeAspect]) {
		double k = MIN(layer.width / partWidth, layer.height / partHeight);
		return CGRectMake((layer.width - partWidth * k) / 2, (layer.height - partHeight * k) / 2,
		                  partWidth * k, partHeight * k);
	}
	// center, and the corner and edge gravities
	double x = (layer.width - partWidth) / 2, y = (layer.height - partHeight) / 2;
	if ([c->gravity isEqualToString: kCAGravityTopLeft]) {
		x = 0;
		y = layer.height - partHeight;
	}
	return CGRectMake(x, y, partWidth, partHeight);
}

static void runCase(const Case *c, CGImageRef image, int *failures)
{
	CALayer *root = [CALayer layer];
	root.anchorPoint = CGPointZero;
	root.frame = CGRectMake(0, 0, SIZE, SIZE);
	CALayer *layer = [CALayer layer];
	layer.anchorPoint = CGPointZero;
	layer.frame = c->frame;
	layer.contents = (id) image;
	layer.magnificationFilter = kCAFilterNearest;
	layer.minificationFilter = kCAFilterNearest;
	layer.contentsScale = c->scale;
	layer.contentsRect = c->rect;
	layer.contentsCenter = c->center;
	layer.geometryFlipped = c->flipped;
	if (c->gravity != nil)
		layer.contentsGravity = c->gravity;
	[root addSublayer: layer];
	render(root);

	// Source part and centre in pixels, x from the left, v up from the bottom
	// edge of the picture (from the top edge when flipped).
	double partX = c->rect.origin.x * IMG, partWidth = c->rect.size.width * IMG;
	double partV = c->rect.origin.y * IMG, partHeight = c->rect.size.height * IMG;
	CGRect rect = destinationRect(c, partWidth / c->scale, partHeight / c->scale);
	Axis ax = { rect.origin.x, rect.origin.x + rect.size.width, partX, partX + partWidth,
	            partX + c->center.origin.x * partWidth,
	            partX + (c->center.origin.x + c->center.size.width) * partWidth, c->scale };
	Axis ay = { rect.origin.y, rect.origin.y + rect.size.height, partV, partV + partHeight,
	            partV + c->center.origin.y * partHeight,
	            partV + (c->center.origin.y + c->center.size.height) * partHeight, c->scale };

	int wrong = 0, shown = 0, compared = 0;
	for (int y = 0; y < SIZE; y++) {
		for (int x = 0; x < SIZE; x++) {
			double lx = x + 0.5 - c->frame.origin.x;
			double ly = y + 0.5 - c->frame.origin.y;
			if (c->flipped)
				ly = c->frame.size.height - ly;
			double edgeX, edgeY;
			double sx = mapAxis(&ax, lx, &edgeX), sv = mapAxis(&ay, ly, &edgeY);
			if (edgeX < 1.5 || edgeY < 1.5)
				continue;
			int want;
			BOOL inside = lx > ax.d0 && lx < ax.d1 && ly > ay.d0 && ly < ay.d1;
			if (!inside) {
				want = -1;
			} else {
				double row = c->flipped ? sv : IMG - sv;
				double seamX = fabs(sx - CELL * floor(sx / CELL + 0.5));
				double seamY = fabs(row - CELL * floor(row / CELL + 0.5));
				if (seamX < 1.0 || seamY < 1.0)
					continue;
				int col = MAX(0, MIN(2, (int) floor(sx / CELL)));
				int r = MAX(0, MIN(2, (int) floor(row / CELL)));
				want = r * 3 + col;
			}
			compared++;
			int got = cellOf(pixels + (y * SIZE + x) * 4);
			if (got != want) {
				wrong++;
				if (shown++ < 3)
					printf("NOTE: %s: (%d,%d) is cell %d, expected %d\n", c->name, x, y, got, want);
			}
		}
	}
	if (wrong == 0 && compared > 100) {
		printf("PASS: %s\n", c->name);
		return;
	}
	(*failures)++;
	printf("FAIL: %s: %d of %d pixels differ\n", c->name, wrong, compared);
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
	CGImageRef image = createImage();

	CALayer *probe = [CALayer layer];
	probe.anchorPoint = CGPointZero;
	probe.frame = CGRectMake(0, 0, SIZE, SIZE);
	probe.contents = (id) image;
	CALayer *probeRoot = [CALayer layer];
	probeRoot.anchorPoint = CGPointZero;
	probeRoot.frame = CGRectMake(0, 0, SIZE, SIZE);
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

	const CGRect whole = CGRectMake(0, 0, 1, 1);
	const CGRect third = CGRectMake(1.0 / 3, 1.0 / 3, 1.0 / 3, 1.0 / 3);
	const CGRect big = CGRectMake(50, 50, 300, 200);
	const Case cases[] = {
		{ "third, resize", big, third, whole, nil, 1, NO },
		{ "third, contentsScale 2", big, third, whole, nil, 2, NO },
		{ "asymmetric centre", big, CGRectMake(0.2, 0.4, 0.5, 0.3), whole, nil, 1, NO },
		{ "asymmetric centre, geometryFlipped", big, CGRectMake(0.2, 0.4, 0.5, 0.3), whole, nil, 1, YES },
		{ "non-aligned centre", big, CGRectMake(0.3, 0.3, 0.4, 0.4), whole, nil, 1, NO },
		{ "vertical stretch only", big, CGRectMake(0, 1.0 / 3, 1, 1.0 / 3), whole, nil, 1, NO },
		{ "zero-size centre", big, CGRectMake(0.5, 0.5, 0, 0), whole, nil, 1, NO },
		{ "default centre stretches everything", big, whole, whole, nil, 1, NO },
		{ "centre is relative to contentsRect (left half)", big, third, CGRectMake(0, 0, 0.5, 1), nil, 1, NO },
		{ "centre is relative to contentsRect (top right)", big, third, CGRectMake(0.5, 0.5, 0.5, 0.5), nil, 1, NO },
		{ "corners that do not fit shrink", CGRectMake(50, 50, 12, 12), third, whole, nil, 1, NO },
		{ "layer of the contents' size", CGRectMake(50, 50, 30, 30), third, whole, nil, 1, NO },
		{ "resizeAspect fits, then stretches", big, third, whole, kCAGravityResizeAspect, 1, NO },
		{ "gravity center keeps the size", big, third, whole, kCAGravityCenter, 1, NO },
		{ "gravity topLeft keeps the size", big, third, whole, kCAGravityTopLeft, 1, NO },
	};
	int failures = 0;
	for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++)
		runCase(&cases[i], image, &failures);

	CGImageRelease(image);
	CGLSetCurrentContext(NULL);
	[pool release];
	printf("%s\n", failures == 0 ? "PASS" : "FAIL");
	return failures == 0 ? 0 : 1;
}
