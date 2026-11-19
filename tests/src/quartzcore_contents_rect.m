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
 * Which part of a layer's contents is drawn, and where, for a given
 * contentsRect and contentsGravity.
 *
 * contentsRect selects a part of the contents in the unit coordinates of
 * the contents; contentsGravity places that part in the layer's bounds:
 * resize fills them, resizeAspect and resizeAspectFill keep the aspect
 * ratio, and the other gravities place the part at its own size (pixel size
 * divided by contentsScale). A masksToBounds layer clips its contents to
 * its bounds.
 *
 * The contents are a 64x64 picture with four coloured quadrants: top left
 * red, top right green, bottom left blue, bottom right white. Each case
 * builds a 400x400 tree, draws it with an offscreen CARenderer, reads it
 * back and compares where each colour lies (y up) with what macOS shows.
 *
 * Observed on macOS 26.7.3 (x86_64) with -renderInContext:, the same for
 * CGImage and IOSurface contents:
 * - contentsRect y = 0 is the image edge at the layer's bounds origin: the
 *   bottom of the image for a plain layer, the top for a geometryFlipped
 *   layer or a layer inside a flipped superlayer; the image stays upright;
 * - gravity names refer to the layer's coordinates: "top" is the maxY edge,
 *   which a flipped layer shows at the bottom of the screen;
 * - contents placed beyond the bounds show unless the layer masks to them.
 * The offscreen CARenderer draws nothing on that machine (no Metal device),
 * so where -renderInContext: exists and the renderer draws nothing, the
 * test reads the tree back through -renderInContext: instead. The cases
 * whose contentsRect reaches outside the unit square only run with the
 * renderer: the contentsRect documentation says the edge pixels are
 * extended outwards, which -renderInContext: does not do.
 *
 * Needs OpenGL. No ARC.
 */
#define GL_GLEXT_PROTOTYPES 1
#define GL_SILENCE_DEPRECATION 1
#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>
#import <IOSurface/IOSurface.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

@interface CARenderer (UpdateRect)
- (void)addUpdateRect:(CGRect)rect;
@end

#define SIZE 400
#define IMG 64

enum { RED, GREEN, BLUE, WHITE, COLORS };
static const char *const colorNames[COLORS] = { "red", "green", "blue", "white" };

typedef struct {
	int color;
	int x0, x1, y0, y1;
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

static void quadrantPixel(int x, int y, uint8_t rgba[4])
{
	BOOL top = y < IMG / 2, left = x < IMG / 2;
	rgba[0] = (top && left) || (!top && !left) ? 255 : 0;
	rgba[1] = (top && !left) || (!top && !left) ? 255 : 0;
	rgba[2] = !top ? 255 : 0;
	rgba[3] = 255;
}

static CGImageRef createImage(void)
{
	static uint8_t data[IMG * IMG * 4];
	for (int y = 0; y < IMG; y++)
		for (int x = 0; x < IMG; x++)
			quadrantPixel(x, y, data + (y * IMG + x) * 4);
	CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
	CGContextRef bitmap = CGBitmapContextCreate(data, IMG, IMG, 8, IMG * 4, space,
	                                            (CGBitmapInfo) kCGImageAlphaPremultipliedLast);
	CGImageRef image = CGBitmapContextCreateImage(bitmap);
	CGContextRelease(bitmap);
	CGColorSpaceRelease(space);
	return image;
}

static IOSurfaceRef createSurface(void)
{
	NSDictionary *props = @{
		(id) kIOSurfaceWidth: @IMG,
		(id) kIOSurfaceHeight: @IMG,
		(id) kIOSurfaceBytesPerElement: @4,
		(id) kIOSurfacePixelFormat: @((uint32_t) 'BGRA'),
	};
	IOSurfaceRef surface = IOSurfaceCreate((CFDictionaryRef) props);
	if (surface == NULL)
		return NULL;
	IOSurfaceLock(surface, 0, NULL);
	uint8_t *base = IOSurfaceGetBaseAddress(surface);
	size_t stride = IOSurfaceGetBytesPerRow(surface);
	for (int y = 0; y < IMG; y++) {
		for (int x = 0; x < IMG; x++) {
			uint8_t rgba[4], *p = base + y * stride + x * 4;
			quadrantPixel(x, y, rgba);
			p[0] = rgba[2];
			p[1] = rgba[1];
			p[2] = rgba[0];
			p[3] = rgba[3];
		}
	}
	IOSurfaceUnlock(surface, 0, NULL);
	return surface;
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

// Compares the box around each colour with the expected boxes; a colour
// that is not listed must not show.
static void expectBoxes(const char *name, const Box *expected, int count)
{
	int minX[COLORS], minY[COLORS], maxX[COLORS], maxY[COLORS];
	for (int c = 0; c < COLORS; c++) {
		minX[c] = minY[c] = SIZE;
		maxX[c] = maxY[c] = -1;
	}
	for (int y = 0; y < SIZE; y++)
		for (int x = 0; x < SIZE; x++) {
			int c = classify(pixels + (y * SIZE + x) * 4);
			if (c < 0)
				continue;
			minX[c] = MIN(minX[c], x);
			maxX[c] = MAX(maxX[c], x);
			minY[c] = MIN(minY[c], y);
			maxY[c] = MAX(maxY[c], y);
		}

	BOOL ok = YES;
	for (int c = 0; c < COLORS; c++) {
		const Box *want = NULL;
		for (int i = 0; i < count; i++)
			if (expected[i].color == c)
				want = &expected[i];
		BOOL shows = maxX[c] >= 0;
		if (want == NULL ? shows
		                 : !shows || !near(minX[c], want->x0) || !near(maxX[c] + 1, want->x1) ||
		                       !near(minY[c], want->y0) || !near(maxY[c] + 1, want->y1))
			ok = NO;
	}
	// Each expected box is a solid area of its colour: a picture repeated
	// instead of extended at its edges has the same boxes, but other
	// colours near their corners.
	for (int i = 0; i < count; i++) {
		const Box *b = &expected[i];
		const int xs[2] = { b->x0 + 2, b->x1 - 3 }, ys[2] = { b->y0 + 2, b->y1 - 3 };
		for (int j = 0; j < 4; j++) {
			int x = xs[j & 1], y = ys[j >> 1];
			if (x < 0 || y < 0 || x >= SIZE || y >= SIZE)
				continue;
			if (classify(pixels + (y * SIZE + x) * 4) != b->color) {
				ok = NO;
				printf("NOTE: %s: (%d,%d) is not %s\n", name, x, y, colorNames[b->color]);
			}
		}
	}
	if (ok) {
		printf("PASS: %s\n", name);
		return;
	}
	failures++;
	printf("FAIL: %s: shows", name);
	for (int c = 0; c < COLORS; c++)
		if (maxX[c] >= 0)
			printf(" %s x[%d,%d) y[%d,%d)", colorNames[c], minX[c], maxX[c] + 1, minY[c], maxY[c] + 1);
	printf("; expected");
	for (int i = 0; i < count; i++)
		printf(" %s x[%d,%d) y[%d,%d)", colorNames[expected[i].color], expected[i].x0, expected[i].x1,
		       expected[i].y0, expected[i].y1);
	printf("\n");
}

static CALayer *rootLayer(void)
{
	CALayer *root = [CALayer layer];
	root.anchorPoint = CGPointZero;
	root.frame = CGRectMake(0, 0, SIZE, SIZE);
	return root;
}

enum { PLAIN, FLIPPED, IN_FLIPPED };
static const char *const flipNames[] = { "plain", "flipped", "in flipped superlayer" };

// A layer with the frame, in a container that fills the root and may be
// flipped. The frames used are centred, so a flipped container leaves them
// in place.
static CALayer *contentsLayer(CALayer *root, CGRect frame, int flip, id contents)
{
	CALayer *container = [CALayer layer];
	container.anchorPoint = CGPointZero;
	container.frame = root.bounds;
	container.geometryFlipped = flip == IN_FLIPPED;
	CALayer *layer = [CALayer layer];
	layer.anchorPoint = CGPointZero;
	layer.frame = frame;
	layer.geometryFlipped = flip == FLIPPED;
	layer.contents = contents;
	layer.magnificationFilter = kCAFilterNearest;
	layer.minificationFilter = kCAFilterNearest;
	[container addSublayer: layer];
	[root addSublayer: container];
	return layer;
}

// The four quadrants of the upright picture drawn in rect (screen
// coordinates, y up), clipped to clip; whole selects the full picture,
// otherwise only its left half (red over blue) is drawn.
static int quadrants(CGRect rect, BOOL whole, CGRect clip, Box *out)
{
	CGFloat midX = CGRectGetMidX(rect), midY = CGRectGetMidY(rect);
	struct { int color; CGRect r; } parts[4];
	int n = 0;
	if (whole) {
		parts[n].color = RED;
		parts[n++].r = CGRectMake(rect.origin.x, midY, rect.size.width / 2, rect.size.height / 2);
		parts[n].color = GREEN;
		parts[n++].r = CGRectMake(midX, midY, rect.size.width / 2, rect.size.height / 2);
		parts[n].color = BLUE;
		parts[n++].r = CGRectMake(rect.origin.x, rect.origin.y, rect.size.width / 2, rect.size.height / 2);
		parts[n].color = WHITE;
		parts[n++].r = CGRectMake(midX, rect.origin.y, rect.size.width / 2, rect.size.height / 2);
	} else {
		parts[n].color = RED;
		parts[n++].r = CGRectMake(rect.origin.x, midY, rect.size.width, rect.size.height / 2);
		parts[n].color = BLUE;
		parts[n++].r = CGRectMake(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height / 2);
	}
	int count = 0;
	for (int i = 0; i < n; i++) {
		CGRect r = CGRectIntersection(parts[i].r, clip);
		if (CGRectIsEmpty(r))
			continue;
		out[count].color = parts[i].color;
		out[count].x0 = (int) lround(CGRectGetMinX(r));
		out[count].x1 = (int) lround(CGRectGetMaxX(r));
		out[count].y0 = (int) lround(CGRectGetMinY(r));
		out[count].y1 = (int) lround(CGRectGetMaxY(r));
		count++;
	}
	return count;
}

static void contentsRectCases(id image, id surface)
{
	// The layer is at (150,150) 100x100; boxes are on screen, y up.
	static const struct {
		const char *name;
		CGRect rect;
		BOOL rendererOnly;
		int count[2];
		Box boxes[2][4]; // [0] plain, [1] flipped (layer or superlayer)
	} cases[] = {
		{ "(0,0,1,1)", { { 0, 0 }, { 1, 1 } }, NO, { 4, 4 },
		  { { { RED, 150, 200, 200, 250 }, { GREEN, 200, 250, 200, 250 }, { BLUE, 150, 200, 150, 200 }, { WHITE, 200, 250, 150, 200 } },
		    { { RED, 150, 200, 200, 250 }, { GREEN, 200, 250, 200, 250 }, { BLUE, 150, 200, 150, 200 }, { WHITE, 200, 250, 150, 200 } } } },
		{ "(0,0,.5,.5)", { { 0, 0 }, { 0.5, 0.5 } }, NO, { 1, 1 },
		  { { { BLUE, 150, 250, 150, 250 } }, { { RED, 150, 250, 150, 250 } } } },
		{ "(.5,.5,.5,.5)", { { 0.5, 0.5 }, { 0.5, 0.5 } }, NO, { 1, 1 },
		  { { { GREEN, 150, 250, 150, 250 } }, { { WHITE, 150, 250, 150, 250 } } } },
		{ "(0,0,1,.5)", { { 0, 0 }, { 1, 0.5 } }, NO, { 2, 2 },
		  { { { BLUE, 150, 200, 150, 250 }, { WHITE, 200, 250, 150, 250 } },
		    { { RED, 150, 200, 150, 250 }, { GREEN, 200, 250, 150, 250 } } } },
		{ "(.25,0,.5,1)", { { 0.25, 0 }, { 0.5, 1 } }, NO, { 4, 4 },
		  { { { RED, 150, 200, 200, 250 }, { GREEN, 200, 250, 200, 250 }, { BLUE, 150, 200, 150, 200 }, { WHITE, 200, 250, 150, 200 } },
		    { { RED, 150, 200, 200, 250 }, { GREEN, 200, 250, 200, 250 }, { BLUE, 150, 200, 150, 200 }, { WHITE, 200, 250, 150, 200 } } } },
		// Outside the unit square the edge pixels extend outwards.
		{ "(.5,0,1,1), edge extended", { { 0.5, 0 }, { 1, 1 } }, YES, { 2, 2 },
		  { { { GREEN, 150, 250, 200, 250 }, { WHITE, 150, 250, 150, 200 } },
		    { { GREEN, 150, 250, 200, 250 }, { WHITE, 150, 250, 150, 200 } } } },
		{ "(-.5,-.5,2,2), edge extended", { { -0.5, -0.5 }, { 2, 2 } }, YES, { 4, 4 },
		  { { { RED, 150, 200, 200, 250 }, { GREEN, 200, 250, 200, 250 }, { BLUE, 150, 200, 150, 200 }, { WHITE, 200, 250, 150, 200 } },
		    { { RED, 150, 200, 200, 250 }, { GREEN, 200, 250, 200, 250 }, { BLUE, 150, 200, 150, 200 }, { WHITE, 200, 250, 150, 200 } } } },
	};

	for (int kind = 0; kind < 2; kind++) {
		if (kind == 1 && surface == nil) {
			printf("FAIL: cannot create an IOSurface\n");
			failures++;
			continue;
		}
		for (int flip = PLAIN; flip <= IN_FLIPPED; flip++) {
			for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
				char name[160];
				snprintf(name, sizeof(name), "%s contents, %s layer, contentsRect %s", kind == 0 ? "CGImage" : "IOSurface",
				         flipNames[flip], cases[i].name);
				if (useBitmap && cases[i].rendererOnly) {
					printf("SKIP: %s (renderer only)\n", name);
					continue;
				}
				NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
				CALayer *root = rootLayer();
				CALayer *layer = contentsLayer(root, CGRectMake(150, 150, 100, 100), flip, kind == 0 ? image : surface);
				layer.contentsRect = cases[i].rect;
				render(root);
				int which = flip == PLAIN ? 0 : 1;
				expectBoxes(name, cases[i].boxes[which], cases[i].count[which]);
				[pool release];
			}
		}
	}
}

// Where the selected part goes in the layer's coordinates (y = 0 at the
// bounds origin), as macOS places it: see the observations above.
static CGRect placed(NSString *gravity, CGSize bounds, CGSize part)
{
	if ([gravity isEqualToString: kCAGravityResize])
		return CGRectMake(0, 0, bounds.width, bounds.height);
	if ([gravity isEqualToString: kCAGravityResizeAspect] || [gravity isEqualToString: kCAGravityResizeAspectFill]) {
		CGFloat sx = bounds.width / part.width, sy = bounds.height / part.height;
		CGFloat s = [gravity isEqualToString: kCAGravityResizeAspect] ? MIN(sx, sy) : MAX(sx, sy);
		part = CGSizeMake(part.width * s, part.height * s);
	}
	const char *g = [gravity UTF8String];
	CGFloat x = (bounds.width - part.width) / 2, y = (bounds.height - part.height) / 2;
	if (strstr(g, "eft") != NULL)
		x = 0;
	if (strstr(g, "ight") != NULL)
		x = bounds.width - part.width;
	if (strncmp(g, "top", 3) == 0)
		y = bounds.height - part.height;
	if (strncmp(g, "bottom", 6) == 0)
		y = 0;
	return CGRectMake(x, y, part.width, part.height);
}

static void gravityCases(id image)
{
	NSString *gravities[] = {
		kCAGravityResize, kCAGravityCenter, kCAGravityTop, kCAGravityBottom,
		kCAGravityLeft, kCAGravityRight, kCAGravityTopLeft, kCAGravityTopRight,
		kCAGravityBottomLeft, kCAGravityBottomRight, kCAGravityResizeAspect,
		kCAGravityResizeAspectFill,
	};
	static const struct { const char *name; CGRect rect; CGFloat scale; } variants[] = {
		{ "whole picture, contentsScale 1", { { 0, 0 }, { 1, 1 } }, 1 },
		{ "whole picture, contentsScale 2", { { 0, 0 }, { 1, 1 } }, 2 },
		{ "contentsRect (0,0,.5,1)", { { 0, 0 }, { 0.5, 1 } }, 1 },
	};
	// A 120x80 layer at (140,160).
	const CGRect frame = CGRectMake(140, 160, 120, 80);

	for (int flip = PLAIN; flip <= IN_FLIPPED; flip++)
		for (size_t v = 0; v < sizeof(variants) / sizeof(variants[0]); v++)
			for (size_t g = 0; g < sizeof(gravities) / sizeof(gravities[0]); g++) {
				NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
				CALayer *root = rootLayer();
				CALayer *layer = contentsLayer(root, frame, flip, image);
				layer.contentsRect = variants[v].rect;
				layer.contentsScale = variants[v].scale;
				layer.contentsGravity = gravities[g];
				render(root);

				CGSize part = CGSizeMake(IMG * variants[v].rect.size.width / variants[v].scale,
				                         IMG * variants[v].rect.size.height / variants[v].scale);
				CGRect r = placed(gravities[g], frame.size, part);
				// Layer coordinates to the screen: a flipped layer (or one in a
				// flipped superlayer) has y = 0 at the top of its frame.
				CGRect screen = flip == PLAIN
				        ? CGRectOffset(r, frame.origin.x, frame.origin.y)
				        : CGRectMake(frame.origin.x + r.origin.x, CGRectGetMaxY(frame) - CGRectGetMaxY(r),
				                     r.size.width, r.size.height);
				Box boxes[4];
				int count = quadrants(screen, v != 2, CGRectInfinite, boxes);
				char name[160];
				snprintf(name, sizeof(name), "gravity %s, %s layer, %s", [gravities[g] UTF8String], flipNames[flip],
				         variants[v].name);
				expectBoxes(name, boxes, count);
				[pool release];
			}

	// The 64x64 picture in a 40x30 layer at (180,185): it reaches beyond
	// the bounds unless the layer masks to them.
	const CGRect small = CGRectMake(180, 185, 40, 30);
	for (int masks = 0; masks < 2; masks++)
		for (size_t g = 0; g < sizeof(gravities) / sizeof(gravities[0]); g++) {
			NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
			CALayer *root = rootLayer();
			CALayer *layer = contentsLayer(root, small, PLAIN, image);
			layer.contentsGravity = gravities[g];
			layer.masksToBounds = masks;
			render(root);
			CGRect r = CGRectOffset(placed(gravities[g], small.size, CGSizeMake(IMG, IMG)), small.origin.x,
			                        small.origin.y);
			Box boxes[4];
			int count = quadrants(r, YES, masks ? small : CGRectInfinite, boxes);
			char name[160];
			snprintf(name, sizeof(name), "gravity %s, picture larger than the layer%s", [gravities[g] UTF8String],
			         masks ? ", masksToBounds" : "");
			expectBoxes(name, boxes, count);
			[pool release];
		}
}

int main(void)
{
	signal(SIGALRM, watchdog);
	alarm(55);

	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	if (!setUpGL()) {
		printf("FAIL: no offscreen OpenGL framebuffer\n");
		return 1;
	}
	CGImageRef image = createImage();
	IOSurfaceRef surface = createSurface();

	// Find out whether the offscreen renderer draws at all here.
	CALayer *probe = rootLayer();
	probe.contents = (id) image;
	render(probe);
	if (!anythingDrawn()) {
		CALayer *layer = [CALayer layer];
		if (![layer respondsToSelector: @selector(renderInContext:)]) {
			printf("FAIL: the offscreen CARenderer draws nothing\n");
			return 1;
		}
		printf("NOTE: the offscreen CARenderer draws nothing; using -renderInContext:\n");
		useBitmap = YES;
	}

	contentsRectCases((id) image, (id) surface);
	gravityCases((id) image);

	if (surface != NULL)
		CFRelease(surface);
	CGImageRelease(image);
	[pool release];
	printf("%s\n", failures == 0 ? "PASS" : "FAIL");
	return failures == 0 ? 0 : 1;
}
