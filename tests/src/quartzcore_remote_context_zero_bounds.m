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
 * Where a CALayerHost shows the layer tree of a CAContext (private API that
 * applications declare themselves, one process publishing a tree and
 * another hosting it). Core Animation clips a layer's sublayers only when it masks to its
 * bounds, so a tree whose root layer has zero bounds still shows everything
 * its sublayers draw.
 *
 * Each case builds a context's tree and a host at (200, 200) in a 400x400
 * tree, renders the host's tree with an offscreen CARenderer into an OpenGL
 * framebuffer and reads it back. It prints where the coloured content lies,
 * relative to the host's position on the picture, y up, and compares that
 * with what macOS shows.
 *
 * The expected places were observed on macOS 26.7.3 (x86_64) the same way,
 * with a context from +[CAContext localContext]: there a context made with
 * +contextWithCGSConnection:options: is composited by the window server, not
 * by an in-process CARenderer. Observed:
 * - The context's root layer is placed in the host's coordinates as a
 *   sublayer would be (position, anchor point, bounds). The tree under it
 *   keeps the orientation it has in its own context: a flipped host or a
 *   flipped ancestor of the host does not flip it again. In flipped host
 *   coordinates the root's frame is mirrored into place and the tree drawn
 *   upright in it.
 * - Content outside the root's bounds, or at negative coordinates, shows.
 * - A sublayer that masks to its (zero) bounds hides what is under it.
 * - The host's own bounds neither clip nor move the hosted tree.
 *
 * The last cases put an IOSurface that another OpenGL context rendered into
 * (through CGLTexImageIOSurface2D) into the hosted tree as a layer's contents.
 *
 * Needs a display (X11 or Wayland) and OpenGL. No ARC.
 */
#define GL_GLEXT_PROTOTYPES 1
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#import <IOSurface/IOSurface.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#import <OpenGL/CGLIOSurface.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
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

#define SIZE 400
#define HOST_X 200
#define HOST_Y 200

enum { RED, GREEN, BLUE };

static int failures;

static void watchdog(int sig)
{
	static const char msg[] = "FAIL: timed out\n";
	(void) sig;
	write(1, msg, sizeof(msg) - 1);
	_exit(1);
}

static CGLContextObj createContext(void)
{
	CGLPixelFormatAttribute attributes[] = { 0 };
	CGLPixelFormatObj pixelFormat = NULL;
	GLint screens = 0;
	CGLContextObj context = NULL;

	if (CGLChoosePixelFormat(attributes, &pixelFormat, &screens) != kCGLNoError || pixelFormat == NULL)
		return NULL;
	if (CGLCreateContext(pixelFormat, NULL, &context) != kCGLNoError)
		context = NULL;
	CGLReleasePixelFormat(pixelFormat);
	return context;
}

static CGColorRef createColor(int color)
{
	return CGColorCreateGenericRGB(color == RED, color == GREEN, color == BLUE, 1);
}

static CALayer *solidLayer(CGRect frame, int color)
{
	CALayer *layer = [CALayer layer];
	layer.anchorPoint = CGPointZero;
	layer.frame = frame;
	CGColorRef cgColor = createColor(color);
	layer.backgroundColor = cgColor;
	CGColorRelease(cgColor);
	return layer;
}

// Reads the host's tree back through an offscreen CARenderer.
static CGLContextObj readbackContext;
static GLuint readbackFramebuffer;
static uint8_t pixels[SIZE * SIZE * 4];

static BOOL setUpReadback(void)
{
	readbackContext = createContext();
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

static void renderTree(CALayer *top)
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
	renderer.layer = top;
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
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	CGLSetCurrentContext(NULL);
}

// The box around the pixels of the colour, relative to the host's position,
// y up; NO when there are none.
static BOOL findColor(int color, int box[4])
{
	int minX = SIZE, minY = SIZE, maxX = -1, maxY = -1;
	for (int y = 0; y < SIZE; y++) {
		for (int x = 0; x < SIZE; x++) {
			const uint8_t *p = pixels + (y * SIZE + x) * 4;
			BOOL match = p[3] > 200 && p[color] > 200 &&
			             p[(color + 1) % 3] < 60 && p[(color + 2) % 3] < 60;
			if (!match)
				continue;
			minX = MIN(minX, x);
			maxX = MAX(maxX, x);
			minY = MIN(minY, y);
			maxY = MAX(maxY, y);
		}
	}
	if (maxX < 0)
		return NO;
	box[0] = minX - HOST_X;
	box[1] = maxX + 1 - HOST_X;
	box[2] = minY - HOST_Y;
	box[3] = maxY + 1 - HOST_Y;
	return YES;
}

static void runLoopFor(NSTimeInterval seconds)
{
	[[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow: seconds]];
}

static BOOL near(int a, int b)
{
	return abs(a - b) <= 1;
}

// Renders until the colour shows where expected (or, with expected NULL,
// for a while, then checks that it does not show).
static void expectColor(const char *name, CALayer *top, int color, const int *expected)
{
	int box[4] = { 0, 0, 0, 0 };
	BOOL found = NO, matches = NO;

	for (int attempt = 0; attempt < (expected != NULL ? 40 : 10) && !matches; attempt++) {
		runLoopFor(0.05);
		renderTree(top);
		found = findColor(color, box);
		matches = expected != NULL && found && near(box[0], expected[0]) && near(box[1], expected[1]) &&
		          near(box[2], expected[2]) && near(box[3], expected[3]);
	}

	if (expected == NULL ? !found : matches) {
		printf("PASS: %s\n", name);
		return;
	}
	failures++;
	if (found)
		printf("FAIL: %s: shows x [%d,%d) y [%d,%d)", name, box[0], box[1], box[2], box[3]);
	else
		printf("FAIL: %s: shows nothing", name);
	if (expected != NULL)
		printf(", expected x [%d,%d) y [%d,%d)\n", expected[0], expected[1], expected[2], expected[3]);
	else
		printf(", expected nothing\n");
}

typedef struct {
	const char *name;
	CGRect rootBounds;
	BOOL rootFlipped;
	CGPoint rootAnchorPoint;
	CGPoint rootPosition;
	BOOL middleMasksToBounds;
	CGRect content;
	BOOL hostInFlippedLayer;
	BOOL hostFlipped;
	CGSize hostSize;
	BOOL showsNothing;
	int expected[4];
} Case;

// The host's tree: a white 400x400 layer with the host at (200, 200), maybe
// under a flipped layer that fills it.
static CALayer *hostTree(CAContextID contextId, BOOL inFlippedLayer, BOOL hostFlipped, CGSize hostSize)
{
	CALayer *top = solidLayer(CGRectMake(0, 0, SIZE, SIZE), RED);
	CGColorRef white = CGColorCreateGenericRGB(1, 1, 1, 1);
	top.backgroundColor = white;
	CGColorRelease(white);

	CALayer *parent = top;
	if (inFlippedLayer) {
		parent = [CALayer layer];
		parent.anchorPoint = CGPointZero;
		parent.frame = CGRectMake(0, 0, SIZE, SIZE);
		parent.geometryFlipped = YES;
		[top addSublayer: parent];
	}

	CALayerHost *host = [CALayerHost layer];
	host.anchorPoint = CGPointZero;
	// In the flipped layer, y = 200 is also 200 from the bottom.
	host.frame = CGRectMake(HOST_X, HOST_Y, hostSize.width, hostSize.height);
	host.geometryFlipped = hostFlipped;
	host.contextId = contextId;
	[parent addSublayer: host];
	return top;
}

// A context whose root has a zero-frame sublayer that holds the content.
static CAContext *contextWithContent(const Case *c, CALayer *content)
{
	CALayer *root = [CALayer layer];
	root.geometryFlipped = c->rootFlipped;
	root.anchorPoint = c->rootAnchorPoint;
	root.bounds = c->rootBounds;
	root.position = c->rootPosition;

	CALayer *middle = [CALayer layer];
	middle.anchorPoint = CGPointZero;
	middle.frame = CGRectZero;
	middle.masksToBounds = c->middleMasksToBounds;
	[root addSublayer: middle];
	[middle addSublayer: content];

	CAContext *context = [CAContext contextWithCGSConnection: 0 options: @{}];
	context.layer = root;
	return context;
}

static void runCase(const Case *c)
{
	CAContext *context = contextWithContent(c, solidLayer(c->content, RED));
	if (context == nil || context.contextId == 0) {
		printf("FAIL: %s: no context\n", c->name);
		failures++;
		return;
	}
	CALayer *top = hostTree(context.contextId, c->hostInFlippedLayer, c->hostFlipped, c->hostSize);
	expectColor(c->name, top, RED, c->showsNothing ? NULL : c->expected);
	[context invalidate];
	[top setSublayers: nil];
}

// Fills the surface with a colour by drawing into a texture bound to it, in
// the given (non-current) OpenGL context, as ANGLE on CGL does. The texture
// is kept, as ANGLE keeps it while the surface is in use; the caller deletes
// it.
static BOOL drawIntoSurface(CGLContextObj context, IOSurfaceRef surface, int color, GLuint *textureOut)
{
	CGLContextObj previous = CGLGetCurrentContext();
	CGLSetCurrentContext(context);

	GLuint texture, framebuffer;
	glGenTextures(1, &texture);
	*textureOut = texture;
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texture);
	CGLError error = CGLTexImageIOSurface2D(context, GL_TEXTURE_RECTANGLE_ARB, GL_RGBA,
	                                        (GLsizei) IOSurfaceGetWidth(surface),
	                                        (GLsizei) IOSurfaceGetHeight(surface),
	                                        GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, surface, 0);
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE_ARB, texture, 0);
	BOOL ok = error == kCGLNoError && glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	if (ok) {
		glViewport(0, 0, (GLsizei) IOSurfaceGetWidth(surface), (GLsizei) IOSurfaceGetHeight(surface));
		glClearColor(color == RED, color == GREEN, color == BLUE, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		glFlush();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &framebuffer);
	CGLSetCurrentContext(previous);
	return ok;
}

static IOSurfaceRef createSurface(int width, int height)
{
	NSDictionary *properties = @{
		(NSString *) kIOSurfaceWidth : @(width),
		(NSString *) kIOSurfaceHeight : @(height),
		(NSString *) kIOSurfaceBytesPerElement : @4,
		(NSString *) kIOSurfacePixelFormat : @((uint32_t) 'BGRA'),
	};
	return IOSurfaceCreate((CFDictionaryRef) properties);
}

// A layer in the hosted tree shows an IOSurface that another OpenGL context
// rendered into; then a second surface, rendered the same way, replaces it.
static void runSurfaceCase(void)
{
	static const Case shape = {
		"", { { 0, 0 }, { 0, 0 } }, YES, { 0.5, 0.5 }, { 0, 0 }, NO, { { 0, 0 }, { 80, 50 } }, YES, NO, { 0, 0 }, NO,
	};
	// In the flipped host layer the zero-bounds root's content hangs down
	// from the host's position.
	static const int expected[4] = { 0, 80, -50, 0 };

	CGLContextObj producer = createContext();
	IOSurfaceRef first = createSurface(80, 50), second = createSurface(80, 50);
	if (producer == NULL || first == NULL || second == NULL) {
		printf("FAIL: IOSurface contents: cannot create a context or surfaces\n");
		failures++;
		return;
	}

	GLuint textures[2] = { 0, 0 };
	if (!drawIntoSurface(producer, first, GREEN, &textures[0])) {
		printf("FAIL: IOSurface contents: cannot draw into the surface\n");
		failures++;
		return;
	}
	CALayer *content = [CALayer layer];
	content.anchorPoint = CGPointZero;
	content.frame = shape.content;
	content.contents = (id) first;

	CAContext *context = contextWithContent(&shape, content);
	CALayer *top = hostTree(context.contextId, YES, NO, CGSizeZero);
	expectColor("IOSurface drawn by another GL context shows", top, GREEN, expected);

	if (!drawIntoSurface(producer, second, BLUE, &textures[1])) {
		printf("FAIL: IOSurface contents: cannot draw into the second surface\n");
		failures++;
	} else {
		content.contents = (id) second;
		expectColor("next IOSurface drawn by another GL context shows", top, BLUE, expected);
	}

	[context invalidate];
	[top setSublayers: nil];
	CGLSetCurrentContext(producer);
	glDeleteTextures(2, textures);
	CGLSetCurrentContext(NULL);
	CFRelease(first);
	CFRelease(second);
	CGLDestroyContext(producer);
}

int main(void)
{
	signal(SIGALRM, watchdog);
	alarm(50);

	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	[NSApplication sharedApplication];

	if (!setUpReadback()) {
		printf("FAIL: no offscreen OpenGL framebuffer\n");
		return 1;
	}

	static const Case cases[] = {
		{ "zero-bounds flipped root", { { 0, 0 }, { 0, 0 } }, YES, { 0, 0 }, { 0, 0 }, NO,
		  { { 0, 0 }, { 100, 60 } }, NO, NO, { 0, 0 }, NO, { 0, 100, -60, 0 } },
		{ "zero-bounds flipped root, host in a flipped layer", { { 0, 0 }, { 0, 0 } }, YES, { 0.5, 0.5 },
		  { 0, 0 }, NO, { { 0, 0 }, { 100, 60 } }, YES, NO, { 0, 0 }, NO, { 0, 100, -60, 0 } },
		{ "zero-bounds flipped root, flipped host", { { 0, 0 }, { 0, 0 } }, YES, { 0, 0 }, { 0, 0 }, NO,
		  { { 0, 0 }, { 100, 60 } }, NO, YES, { 0, 0 }, NO, { 0, 100, -60, 0 } },
		{ "zero-bounds flipped root, host with bounds", { { 0, 0 }, { 0, 0 } }, YES, { 0, 0 }, { 0, 0 }, NO,
		  { { 0, 0 }, { 100, 60 } }, NO, NO, { 100, 60 }, NO, { 0, 100, -60, 0 } },
		{ "zero-bounds flipped root at (30, 20)", { { 0, 0 }, { 0, 0 } }, YES, { 0, 0 }, { 30, 20 }, NO,
		  { { 0, 0 }, { 100, 60 } }, NO, NO, { 0, 0 }, NO, { 30, 130, -40, 20 } },
		{ "zero-bounds root, content at negative coordinates", { { 0, 0 }, { 0, 0 } }, NO, { 0, 0 },
		  { 0, 0 }, NO, { { -50, -30 }, { 100, 60 } }, NO, NO, { 0, 0 }, NO, { -50, 50, -30, 30 } },
		{ "zero-bounds root, sublayer masks to its bounds", { { 0, 0 }, { 0, 0 } }, YES, { 0, 0 },
		  { 0, 0 }, YES, { { 0, 0 }, { 100, 60 } }, NO, NO, { 0, 0 }, YES, { 0, 0, 0, 0 } },
		// Roots with bounds: unchanged when the content fits.
		{ "100x60 flipped root", { { 0, 0 }, { 100, 60 } }, YES, { 0, 0 }, { 0, 0 }, NO,
		  { { 0, 0 }, { 100, 60 } }, NO, NO, { 0, 0 }, NO, { 0, 100, 0, 60 } },
		{ "100x60 flipped root, host in a flipped layer", { { 0, 0 }, { 100, 60 } }, YES, { 0, 0 },
		  { 0, 0 }, NO, { { 0, 0 }, { 100, 60 } }, YES, NO, { 0, 0 }, NO, { 0, 100, -60, 0 } },
		{ "100x60 root", { { 0, 0 }, { 100, 60 } }, NO, { 0, 0 }, { 0, 0 }, NO, { { 0, 0 }, { 100, 60 } },
		  NO, NO, { 0, 0 }, NO, { 0, 100, 0, 60 } },
		{ "100x60 flipped root, default anchor point", { { 0, 0 }, { 100, 60 } }, YES, { 0.5, 0.5 },
		  { 0, 0 }, NO, { { 0, 0 }, { 100, 60 } }, NO, NO, { 0, 0 }, NO, { -50, 50, -30, 30 } },
		{ "100x60 flipped root, content beyond its bounds", { { 0, 0 }, { 100, 60 } }, YES, { 0, 0 },
		  { 0, 0 }, NO, { { 50, 30 }, { 100, 60 } }, NO, NO, { 0, 0 }, NO, { 50, 150, -30, 30 } },
	};

	for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
		NSAutoreleasePool *casePool = [[NSAutoreleasePool alloc] init];
		runCase(&cases[i]);
		[casePool release];
	}

	runSurfaceCase();

	[pool release];
	printf("%s\n", failures == 0 ? "PASS" : "FAIL");
	return failures == 0 ? 0 : 1;
}
