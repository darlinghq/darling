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
 * CALayer autoresizing: when a layer's bounds change, its sublayers apply their
 * autoresizingMask synchronously, through -resizeSublayersWithOldSize: and
 * -resizeWithOldSuperlayerSize:, which subclasses may override.
 *
 * The expected values were read on macOS 26.7.3 (x86_64) with probes that are
 * not part of this test. Per axis, the change of the superlayer's extent is
 * shared between the flexible parts of the sublayer: a flexible size takes a
 * third when a margin is flexible too, otherwise all of it; flexible margins
 * share the rest equally; the sizes of the parts do not matter. The result is a
 * whole-number rectangle (origin rounded down, far edge rounded up, negative
 * extent flipped) whenever the mask is not empty. Layout managers and
 * -layoutSublayers play no part. The rendering part needs OpenGL (a display on
 * Darling) and prints SKIP where an offscreen CARenderer draws nothing (macOS
 * without Metal). It must be built against the SDK headers. No ARC.
 */
#define GL_GLEXT_PROTOTYPES 1
#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
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

static NSMutableString *trace;

@interface TracedLayer : CALayer
@end

@implementation TracedLayer
- (void) resizeSublayersWithOldSize: (CGSize) size
{
	[trace appendFormat: @" [sublayers %@ old=%gx%g]", [self name], size.width, size.height];
	[super resizeSublayersWithOldSize: size];
}

- (void) resizeWithOldSuperlayerSize: (CGSize) size
{
	[trace appendFormat: @" [self %@ old=%gx%g]", [self name], size.width, size.height];
	[super resizeWithOldSuperlayerSize: size];
}

- (void) layoutSublayers
{
	[trace appendFormat: @" [layout %@]", [self name]];
	[super layoutSublayers];
}
@end

@interface CountingLayoutManager : NSObject
@end

@implementation CountingLayoutManager
- (void) layoutSublayersOfLayer: (CALayer *) layer
{
	[trace appendFormat: @" [manager %@]", [layer name]];
}
@end

static CALayer *layerWithFrame(CGRect frame, unsigned mask)
{
	CALayer *layer = [CALayer layer];
	layer.anchorPoint = CGPointZero;
	layer.frame = frame;
	layer.autoresizingMask = mask;
	return layer;
}

// One axis: a sublayer at [origin, origin + length) of a superlayer whose width
// goes from from to to. The mask bits are MinX 1, Width 2, MaxX 4.
static void checkX(unsigned mask, CGFloat origin, CGFloat length, CGFloat from, CGFloat to, CGFloat wantOrigin, CGFloat wantLength)
{
	CALayer *parent = [CALayer layer];
	parent.bounds = CGRectMake(0, 0, from, 100);
	CALayer *child = layerWithFrame(CGRectMake(origin, 0, length, 5), mask);
	[parent addSublayer: child];
	parent.bounds = CGRectMake(0, 0, to, 100);
	CGRect frame = child.frame;
	CHECK(frame.origin.x == wantOrigin && frame.size.width == wantLength,
	      "mask %u at %g+%g, superlayer %g -> %g: x %g width %g (want %g, %g)",
	      mask, origin, length, from, to, frame.origin.x, frame.size.width, wantOrigin, wantLength);
}

// The same on the vertical axis, in a flipped or an unflipped superlayer. The
// mask bits are MinY 8, Height 16, MaxY 32.
static void checkY(BOOL flipped, unsigned mask, CGFloat wantOrigin, CGFloat wantLength)
{
	CALayer *parent = [CALayer layer];
	parent.geometryFlipped = flipped;
	parent.bounds = CGRectMake(0, 0, 100, 100);
	CALayer *child = layerWithFrame(CGRectMake(5, 10, 5, 30), mask);
	[parent addSublayer: child];
	parent.bounds = CGRectMake(0, 0, 100, 200);
	CGRect frame = child.frame;
	CHECK(frame.origin.y == wantOrigin && frame.size.height == wantLength,
	      "%s superlayer, mask %u at 10+30, height 100 -> 200: y %g height %g (want %g, %g)",
	      flipped ? "flipped" : "unflipped", mask, frame.origin.y, frame.size.height, wantOrigin, wantLength);
}

static void checkGeometry(void)
{
	[CATransaction begin];
	[CATransaction setDisableActions: YES];

	// The sublayer of a zero-size superlayer, as in a flipped content layer
	// whose container gets its size later.
	CALayer *root = [CALayer layer];
	CALayer *flipped = [CALayer layer];
	flipped.geometryFlipped = YES;
	flipped.anchorPoint = CGPointZero;
	flipped.autoresizingMask = kCALayerWidthSizable | kCALayerHeightSizable;
	[root addSublayer: flipped];
	CALayer *host = [CALayer layer];
	host.anchorPoint = CGPointZero;
	host.autoresizingMask = kCALayerMaxXMargin | kCALayerMaxYMargin;
	[flipped addSublayer: host];
	root.bounds = CGRectMake(0, 0, 700, 440);
	CHECK(CGRectEqualToRect(flipped.frame, CGRectMake(0, 0, 700, 440)),
	      "a sizable sublayer follows setBounds: at once (frame %g,%g %gx%g)",
	      flipped.frame.origin.x, flipped.frame.origin.y, flipped.frame.size.width, flipped.frame.size.height);
	CHECK(CGRectEqualToRect(host.frame, CGRectMake(0, 0, 0, 0)), "max-margin sublayer keeps its frame");
	root.frame = CGRectMake(5, 5, 300, 200);
	CHECK(CGRectEqualToRect(flipped.frame, CGRectMake(0, 0, 300, 200)), "a sizable sublayer follows setFrame:");
	CHECK(CGRectEqualToRect(root.frame, CGRectMake(5, 5, 300, 200)), "the superlayer's own frame is as set");
	CHECK(host.frame.size.width == 0, "a sublayer without flexible size stays empty");

	CALayer *p = [CALayer layer];
	p.bounds = CGRectMake(0, 0, 100, 100);
	CALayer *m = layerWithFrame(CGRectMake(10, 20, 30, 40), kCALayerMinXMargin);
	[p addSublayer: m];
	p.bounds = CGRectMake(0, 0, 200, 150);
	CHECK(CGRectEqualToRect(m.frame, CGRectMake(110, 20, 30, 40)), "a flexible min-x margin moves the sublayer right");

	// Sublayers added later are not resized, and a sizable sublayer of a leaf follows too.
	CALayer *q = [CALayer layer];
	q.bounds = CGRectMake(0, 0, 100, 100);
	CALayer *added = layerWithFrame(CGRectMake(0, 0, 10, 10), kCALayerWidthSizable | kCALayerHeightSizable);
	[q addSublayer: added];
	CHECK(CGRectEqualToRect(added.frame, CGRectMake(0, 0, 10, 10)), "addSublayer: does not resize");

	// The shares along x (probes: 3 = MinX|Width, 6 = Width|MaxX, 7 = all, 2 = Width, 1 = MinX, 5 = both margins).
	checkX(3, 10, 30, 100, 200, 76, 64);
	checkX(3, 0, 0, 100, 200, 66, 34);
	checkX(3, 20, 50, 0, 100, 86, 84);
	checkX(3, 10, 30, 100, 40, -30, 10);
	checkX(3, 10, 30, 100, 130, 30, 40);
	checkX(3, 10, 30, 100, 101, 10, 31);
	checkX(6, 10, 30, 100, 200, 10, 64);
	checkX(7, 10, 30, 100, 200, 43, 64);
	checkX(7, 0, 0, 100, 200, 33, 34);
	checkX(7, 10, 30, 100, 107, 12, 33);
	checkX(7, 10.4, 30.4, 100, 110, 13, 35);
	checkX(2, 10, 30, 100, 200, 10, 130);
	checkX(2, 50, 10, 100, 200, 50, 110);
	checkX(2, 10.5, 30.5, 100, 100.5, 10, 32);
	checkX(2, 20, 50, 100, 40, 10, 10);
	checkX(2, 10, 30, 100, 20, -40, 50);
	checkX(1, 10, 30, 100, 200, 110, 30);
	checkX(1, 10.25, 30, 100, 107.5, 17, 31);
	checkX(5, 10, 30, 100, 200, 60, 30);
	checkX(5, 10, 30, 100, 107, 13, 31);
	checkX(4, 10, 30, 100, 200, 10, 30);
	checkX(0, 10.5, 30.5, 100, 107, 10.5, 30.5);

	for (int flipped = 0; flipped < 2; flipped++) {
		checkY(flipped, kCALayerMinYMargin, 110, 30);
		checkY(flipped, kCALayerHeightSizable, 10, 130);
		checkY(flipped, kCALayerMaxYMargin, 10, 30);
		checkY(flipped, kCALayerMinYMargin | kCALayerHeightSizable, 76, 64);
		checkY(flipped, kCALayerHeightSizable | kCALayerMaxYMargin, 10, 64);
		checkY(flipped, kCALayerMinYMargin | kCALayerHeightSizable | kCALayerMaxYMargin, 43, 64);
	}

	// Both axes, all flexible, from nothing: the old size is 0x0.
	CALayer *z = [CALayer layer];
	CALayer *zc = layerWithFrame(CGRectMake(3, 4, 5, 6), 63);
	[z addSublayer: zc];
	z.bounds = CGRectMake(0, 0, 100, 80);
	CHECK(CGRectEqualToRect(zc.frame, CGRectMake(36, 30, 39, 34)), "old size 0x0, all parts flexible");
	z.bounds = CGRectMake(0, 0, 50, 40);
	CHECK(CGRectEqualToRect(zc.frame, CGRectMake(19, 16, 23, 22)), "then shrinking again");

	// Fractions: a non-empty mask makes the frame whole numbers, even when only the origin of the bounds changed.
	CALayer *f = [CALayer layer];
	f.bounds = CGRectMake(0, 0, 100, 100);
	CALayer *fa = layerWithFrame(CGRectMake(10.5, 20.5, 30.5, 40.5), kCALayerWidthSizable);
	CALayer *fb = layerWithFrame(CGRectMake(10.5, 20.5, 30.5, 40.5), kCALayerNotSizable);
	[f addSublayer: fa];
	[f addSublayer: fb];
	f.bounds = CGRectMake(3, 4, 100, 100);
	CHECK(CGRectEqualToRect(fa.frame, CGRectMake(10, 20, 31, 41)), "origin-only bounds change integralises a masked sublayer");
	CHECK(CGRectEqualToRect(fb.frame, CGRectMake(10.5, 20.5, 30.5, 40.5)), "an empty mask leaves the frame alone");

	// The frame of a sublayer with a centred anchor point is set, not its bounds.
	CALayer *a = [CALayer layer];
	a.bounds = CGRectMake(0, 0, 100, 100);
	CALayer *ac = [CALayer layer];
	ac.frame = CGRectMake(10, 10, 20, 20);
	ac.autoresizingMask = kCALayerHeightSizable;
	[a addSublayer: ac];
	a.bounds = CGRectMake(0, 0, 100, 200);
	CHECK(CGRectEqualToRect(ac.frame, CGRectMake(10, 10, 20, 120)) && ac.position.y == 70,
	      "a centred anchor point: frame 10,10 20x120, position y %g", ac.position.y);

	// Key-value coding goes through the same accessors.
	CALayer *k = [CALayer layer];
	k.bounds = CGRectMake(0, 0, 50, 50);
	CALayer *kc = layerWithFrame(CGRectMake(0, 0, 10, 10), kCALayerWidthSizable | kCALayerHeightSizable);
	[k addSublayer: kc];
	[k setValue: [NSValue valueWithRect: NSMakeRect(0, 0, 70, 60)] forKey: @"bounds"];
	CHECK(CGRectEqualToRect(kc.frame, CGRectMake(0, 0, 30, 20)), "KVC bounds resizes the sublayers");
	[k setValue: [NSValue valueWithRect: NSMakeRect(1, 2, 90, 80)] forKey: @"frame"];
	CHECK(CGRectEqualToRect(kc.frame, CGRectMake(0, 0, 50, 40)), "KVC frame resizes the sublayers");

	// Implicit actions on: the new geometry is in place at once.
	[CATransaction commit];
	CALayer *i = [CALayer layer];
	i.bounds = CGRectMake(0, 0, 100, 100);
	CALayer *ic = layerWithFrame(CGRectMake(0, 0, 100, 100), kCALayerWidthSizable | kCALayerHeightSizable);
	[i addSublayer: ic];
	i.bounds = CGRectMake(0, 0, 300, 300);
	CHECK(CGRectEqualToRect(ic.frame, CGRectMake(0, 0, 300, 300)), "the resize is synchronous with implicit actions enabled");
}

static void checkOverrides(void)
{
	[CATransaction begin];
	[CATransaction setDisableActions: YES];
	trace = [[NSMutableString alloc] init];

	TracedLayer *p = [TracedLayer layer];
	p.name = @"P";
	TracedLayer *a = [TracedLayer layer];
	a.name = @"A";
	a.autoresizingMask = kCALayerWidthSizable | kCALayerHeightSizable;
	TracedLayer *b = [TracedLayer layer];
	b.name = @"B";
	TracedLayer *g = [TracedLayer layer];
	g.name = @"G";
	g.autoresizingMask = kCALayerWidthSizable | kCALayerHeightSizable;
	[p addSublayer: a];
	[p addSublayer: b];
	[a addSublayer: g];
	p.bounds = CGRectMake(0, 0, 50, 50);
	a.bounds = CGRectMake(0, 0, 10, 10);

	[trace setString: @""];
	p.bounds = CGRectMake(0, 0, 80, 90);
	CHECK([trace isEqualToString: @" [sublayers P old=50x50] [self B old=50x50] [self A old=50x50] [sublayers A old=10x10] [self G old=10x10] [sublayers G old=10x10]"],
	      "setBounds: calls the overrides, sublayers last to first, nested: %s", [trace UTF8String]);
	CHECK(a.bounds.size.width == 40 && a.bounds.size.height == 50 && g.bounds.size.width == 40 && g.bounds.size.height == 50,
	      "the nested sizable layers got 40x50");
	CHECK([p needsLayout], "the layout is still pending");

	[trace setString: @""];
	[p layoutIfNeeded];
	CHECK([trace isEqualToString: @" [layout P] [layout A] [layout G]"],
	      "layoutIfNeeded lays out the tree and does not resize: %s", [trace UTF8String]);

	[trace setString: @""];
	p.bounds = CGRectMake(5, 5, 80, 90);
	CHECK([trace isEqualToString: @" [sublayers P old=80x90] [self B old=80x90] [self A old=80x90]"],
	      "an origin-only change is passed on with the unchanged size: %s", [trace UTF8String]);

	[trace setString: @""];
	p.bounds = CGRectMake(5, 5, 80, 90);
	CHECK([trace length] == 0, "setting equal bounds calls nothing: %s", [trace UTF8String]);

	[trace setString: @""];
	p.frame = CGRectMake(1, 2, 70, 60);
	CHECK([trace isEqualToString: @" [sublayers P old=80x90] [self B old=80x90] [self A old=80x90] [sublayers A old=40x50] [self G old=40x50] [sublayers G old=40x50]"],
	      "setFrame: resizes through the same calls: %s", [trace UTF8String]);

	[trace setString: @""];
	p.position = CGPointMake(9, 9);
	CHECK([trace length] == 0, "moving the layer calls nothing: %s", [trace UTF8String]);

	[trace setString: @""];
	TracedLayer *leaf = [TracedLayer layer];
	leaf.name = @"L";
	leaf.bounds = CGRectMake(0, 0, 10, 10);
	CHECK([trace isEqualToString: @" [sublayers L old=0x0]"], "a layer without sublayers is called too: %s", [trace UTF8String]);

	// A layout manager does not take the autoresizing over.
	[trace setString: @""];
	p.layoutManager = [[[CountingLayoutManager alloc] init] autorelease];
	p.bounds = CGRectMake(0, 0, 90, 90);
	CHECK(a.bounds.size.width == 50 && a.bounds.size.height == 50, "with a layout manager the sublayers are still autoresized (%gx%g)", a.bounds.size.width, a.bounds.size.height);
	[trace setString: @""];
	[p layoutIfNeeded];
	CHECK([trace isEqualToString: @" [layout P] [manager P] [layout A] [layout G]"], "the layout manager runs in the layout pass: %s", [trace UTF8String]);

	[CATransaction commit];
	[trace release];
	trace = nil;
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

// Red channel of the picture at column x, row y counted from the bottom.
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

static CALayer *redLayer(CGRect frame, unsigned mask)
{
	CALayer *layer = layerWithFrame(frame, mask);
	CGColorRef red = CGColorCreateGenericRGB(1, 0, 0, 1);
	layer.backgroundColor = red;
	CGColorRelease(red);
	return layer;
}

// The shape of a browser's layer tree: a root that only gets its size after
// its sublayers exist, a flipped sizable layer in it, a red layer at the top
// left of that (y 0 to 6 of 16, flipped) with flexible max margins.
static void checkRendering(void)
{
	if (!setUpReadback()) {
		printf("SKIP: no OpenGL context for the CARenderer readback\n");
		return;
	}
	CALayer *plain = redLayer(CGRectMake(0, 0, SIZE, SIZE), kCALayerNotSizable);
	if (redAt(renderPicture(plain), SIZE / 2, SIZE / 2) == 0) {
		printf("SKIP: the CARenderer draws nothing here, not even a plain layer\n");
		return;
	}

	[CATransaction begin];
	[CATransaction setDisableActions: YES];
	CALayer *root = [CALayer layer];
	root.anchorPoint = CGPointZero;
	CALayer *flipped = [CALayer layer];
	flipped.geometryFlipped = YES;
	flipped.anchorPoint = CGPointZero;
	flipped.autoresizingMask = kCALayerWidthSizable | kCALayerHeightSizable;
	[root addSublayer: flipped];
	CALayer *host = redLayer(CGRectMake(0, 0, SIZE, 6), kCALayerMaxXMargin | kCALayerMaxYMargin);
	[flipped addSublayer: host];
	root.bounds = CGRectMake(0, 0, SIZE, SIZE);
	[CATransaction commit];

	const uint8_t *picture = renderPicture(root);
	CHECK(redAt(picture, SIZE / 2, SIZE - 3) >= 250, "the layer at y 0 of the flipped sublayer is drawn at the top (red %d)", redAt(picture, SIZE / 2, SIZE - 3));
	CHECK(redAt(picture, SIZE / 2, 3) == 0, "and not at the bottom (red %d)", redAt(picture, SIZE / 2, 3));
	CHECK(redAt(picture, SIZE / 2, SIZE - 8) < 5, "it covers 6 rows of 16 (red %d at row 8 from the top)", redAt(picture, SIZE / 2, SIZE - 8));
}

int main(void)
{
	signal(SIGALRM, watchdog);
	alarm(50);
	@autoreleasepool {
		checkGeometry();
		checkOverrides();
		checkRendering();
	}
	printf(failures ? "%d FAILED\n" : "ALL PASSED\n", failures);
	return failures ? 1 : 0;
}
