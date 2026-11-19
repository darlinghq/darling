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
 * A layer whose sublayers were set to nil still takes sublayers. The public
 * contract: sublayers is nil or an array of the layer's children, setting it
 * replaces them, and every mutator (-addSublayer:, -insertSublayer:atIndex:,
 * -insertSublayer:above:/below:, -replaceSublayer:with:) works on a layer
 * that has none.
 *
 * Observed on macOS 26.7.3 (x86_64), the same calls in a probe:
 * - -sublayers is nil on a new layer, after = nil, after = @[], and after
 *   the last sublayer is removed (never an empty array).
 * - -setSublayers: makes each new layer's superlayer the receiver (taking it
 *   out of its old parent) and clears the superlayer of the layers it drops.
 * - -replaceSublayer:with: puts the new layer at the old one's index and
 *   clears the old one's superlayer; with a nil replacement it removes the layer.
 *
 * The last case renders a layer attached this way, through a CAContext root
 * hosted by a CALayerHost, with an offscreen CARenderer, and also directly.
 * The CAContext part uses private API that applications declare themselves;
 * compile with -DNO_CONTEXT to leave it out (macOS composites such a context
 * in the window server, not in process).
 *
 * Needs OpenGL (a display on Darling). No ARC.
 */
#define GL_GLEXT_PROTOTYPES 1
#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#pragma clang diagnostic ignored "-Wnonnull"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef NO_RENDER
#define NO_CONTEXT 1
#endif

#ifndef NO_CONTEXT
typedef uint32_t CAContextID;

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
#endif

#define SIZE 200

static int failures;

static void expectThat(BOOL ok, const char *what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

static void watchdog(int sig)
{
	static const char msg[] = "FAIL: timed out\n";
	(void) sig;
	write(1, msg, sizeof(msg) - 1);
	_exit(1);
}

static CALayer *newLayer(void)
{
	return [[CALayer alloc] init];
}

// Runs one scenario on a fresh layer and on one whose sublayers were set to nil.
typedef void (*Scenario)(CALayer *, const char *);

static void runScenario(Scenario scenario, CALayer *layer, const char *label)
{
	@try {
		scenario(layer, label);
	} @catch (NSException *e) {
		failures++;
		printf("FAIL: %s raised %s: %s\n", label, [[e name] UTF8String], [[e reason] UTF8String]);
	}
}

static void inBothStates(Scenario scenario, const char *name)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	char label[160];
	CALayer *fresh = newLayer();
	snprintf(label, sizeof(label), "%s, fresh layer", name);
	runScenario(scenario, fresh, label);
	[fresh release];

	CALayer *cleared = newLayer();
	cleared.sublayers = nil;
	snprintf(label, sizeof(label), "%s, after sublayers = nil", name);
	runScenario(scenario, cleared, label);
	[cleared release];
	[pool release];
}

static BOOL order(CALayer *parent, CALayer *a, CALayer *b, CALayer *c)
{
	NSArray *s = parent.sublayers;
	NSUInteger n = (a != nil) + (b != nil) + (c != nil);
	if ([s count] != n)
		return NO;
	CALayer *want[3] = { a, b, c };
	NSUInteger i = 0;
	for (int k = 0; k < 3; k++)
		if (want[k] != nil && [s objectAtIndex: i++] != want[k])
			return NO;
	return YES;
}

static void addScenario(CALayer *l, const char *label)
{
	CALayer *a = newLayer(), *b = newLayer();
	char msg[200];
	[l addSublayer: a];
	[l addSublayer: b];
	snprintf(msg, sizeof(msg), "addSublayer: appends in order (%s)", label);
	expectThat(order(l, a, b, nil), msg);
	snprintf(msg, sizeof(msg), "addSublayer: sets the superlayer (%s)", label);
	expectThat(a.superlayer == l && b.superlayer == l, msg);
	[a release];
	[b release];
}

static void insertIndexScenario(CALayer *l, const char *label)
{
	CALayer *a = newLayer(), *b = newLayer(), *c = newLayer();
	char msg[200];
	[l insertSublayer: a atIndex: 0];
	[l insertSublayer: b atIndex: 0];
	[l insertSublayer: c atIndex: 1];
	snprintf(msg, sizeof(msg), "insertSublayer:atIndex: orders b, c, a (%s)", label);
	expectThat(order(l, b, c, a), msg);
	expectThat(a.superlayer == l && b.superlayer == l && c.superlayer == l, "  superlayers set");
	[a release];
	[b release];
	[c release];
}

static void insertSiblingScenario(CALayer *l, const char *label)
{
	CALayer *a = newLayer(), *b = newLayer(), *c = newLayer();
	char msg[200];
	[l addSublayer: a];
	[l insertSublayer: b above: a];
	[l insertSublayer: c below: a];
	snprintf(msg, sizeof(msg), "insertSublayer:above:/below: orders c, a, b (%s)", label);
	expectThat(order(l, c, a, b), msg);
	expectThat(a.superlayer == l && b.superlayer == l && c.superlayer == l, "  superlayers set");
	[a release];
	[b release];
	[c release];
}

static void replaceScenario(CALayer *l, const char *label)
{
	CALayer *a = newLayer(), *b = newLayer(), *c = newLayer();
	char msg[200];
	[l addSublayer: a];
	[l addSublayer: b];
	[l replaceSublayer: a with: c];
	snprintf(msg, sizeof(msg), "replaceSublayer:with: keeps the index (%s)", label);
	expectThat(order(l, c, b, nil), msg);
	expectThat(c.superlayer == l && a.superlayer == nil, "  superlayer of new set, old cleared");
	[l replaceSublayer: c with: a];
	[l replaceSublayer: b with: nil];
	expectThat(order(l, a, nil, nil) && b.superlayer == nil, "  replacing with nil removes");
	[a release];
	[b release];
	[c release];
}

static void removeScenario(CALayer *l, const char *label)
{
	CALayer *a = newLayer(), *b = newLayer();
	char msg[200];
	[l addSublayer: a];
	[l addSublayer: b];
	[b removeFromSuperlayer];
	snprintf(msg, sizeof(msg), "removeFromSuperlayer (%s)", label);
	expectThat(order(l, a, nil, nil) && b.superlayer == nil, msg);
	[a removeFromSuperlayer];
	snprintf(msg, sizeof(msg), "sublayers is nil after the last one is removed (%s)", label);
	expectThat(l.sublayers == nil && a.superlayer == nil, msg);
	// ... and the layer still takes sublayers.
	[l addSublayer: b];
	expectThat(order(l, b, nil, nil), "  and takes one again");
	[a release];
	[b release];
}

static void moveScenario(CALayer *l, const char *label)
{
	CALayer *other = newLayer(), *a = newLayer();
	char msg[200];
	[other addSublayer: a];
	[l addSublayer: a];
	snprintf(msg, sizeof(msg), "adding a layer moves it from its old parent (%s)", label);
	expectThat(order(l, a, nil, nil) && a.superlayer == l && other.sublayers == nil, msg);
	[other release];
	[a release];
}

static void setSublayersChecks(void)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	CALayer *l = newLayer(), *o = newLayer(), *a = newLayer(), *b = newLayer();

	expectThat(l.sublayers == nil, "sublayers is nil on a new layer");
	l.sublayers = nil;
	expectThat(l.sublayers == nil, "sublayers is nil after = nil");
	l.sublayers = [NSArray array];
	expectThat(l.sublayers == nil, "sublayers is nil after = @[]");

	[o addSublayer: a];
	l.sublayers = [NSArray arrayWithObjects: a, b, nil];
	expectThat(order(l, a, b, nil) && a.superlayer == l && b.superlayer == l,
	      "setSublayers: sets each new layer's superlayer");
	expectThat(o.sublayers == nil, "setSublayers: takes the layer out of its old parent");

	l.sublayers = nil;
	expectThat(a.superlayer == nil && b.superlayer == nil && l.sublayers == nil,
	      "setSublayers:nil clears the superlayer of the old ones");
	[l addSublayer: a];
	expectThat(order(l, a, nil, nil), "addSublayer: works after setSublayers:nil");
	l.sublayers = [NSArray arrayWithObject: b];
	expectThat(a.superlayer == nil && b.superlayer == l, "setSublayers: replaces the old set");

	[l release];
	[o release];
	[a release];
	[b release];
	[pool release];
}

// Renders through an offscreen CARenderer.
static CGLContextObj readbackContext;
static GLuint readbackFramebuffer;
static uint8_t pixels[SIZE * SIZE * 4];

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
	[renderer render];
	[renderer endFrame];
	renderer.layer = nil;

	glFinish();
	glReadPixels(0, 0, SIZE, SIZE, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	CGLSetCurrentContext(NULL);
}

// The red layer is 100x60 at (50, 50) in y-up coordinates.
static BOOL redShowsInPlace(void)
{
	int minX = SIZE, minY = SIZE, maxX = -1, maxY = -1;
	for (int y = 0; y < SIZE; y++) {
		for (int x = 0; x < SIZE; x++) {
			const uint8_t *p = pixels + (y * SIZE + x) * 4;
			if (p[3] > 200 && p[0] > 200 && p[1] < 60 && p[2] < 60) {
				minX = MIN(minX, x);
				maxX = MAX(maxX, x);
				minY = MIN(minY, y);
				maxY = MAX(maxY, y);
			}
		}
	}
	return maxX >= 0 && abs(minX - 50) <= 1 && abs(maxX + 1 - 150) <= 1 &&
	       abs(minY - 50) <= 1 && abs(maxY + 1 - 110) <= 1;
}

static BOOL rendersRed(CALayer *top)
{
	for (int attempt = 0; attempt < 40; attempt++) {
		[[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.05]];
		renderTree(top);
		if (redShowsInPlace())
			return YES;
	}
	return NO;
}

static CALayer *redLayer(void)
{
	CALayer *layer = [CALayer layer];
	layer.anchorPoint = CGPointZero;
	layer.frame = CGRectMake(50, 50, 100, 60);
	CGColorRef red = CGColorCreateGenericRGB(1, 0, 0, 1);
	layer.backgroundColor = red;
	CGColorRelease(red);
	return layer;
}

static CALayer *rootLayer(void)
{
	CALayer *root = [CALayer layer];
	root.anchorPoint = CGPointZero;
	root.frame = CGRectMake(0, 0, SIZE, SIZE);
	return root;
}

static void renderChecks(void)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	CALayer *control = rootLayer();
	[control addSublayer: redLayer()];
	expectThat(rendersRed(control), "control: a layer added to a fresh layer renders");

	CALayer *root = rootLayer();
	root.sublayers = nil;
	[root addSublayer: redLayer()];
	expectThat(rendersRed(root), "a layer added after sublayers = nil renders");

#ifndef NO_CONTEXT
	CALayer *contextRoot = rootLayer();
	contextRoot.sublayers = nil;
	[contextRoot addSublayer: redLayer()];
	CAContext *context = [CAContext contextWithCGSConnection: 0 options: @{}];
	context.layer = contextRoot;
	CALayerHost *host = [CALayerHost layer];
	host.anchorPoint = CGPointZero;
	host.frame = CGRectMake(0, 0, SIZE, SIZE);
	host.contextId = context.contextId;
	CALayer *top = rootLayer();
	top.sublayers = nil;
	[top addSublayer: host];
	expectThat(context.contextId != 0, "a CAContext was created");
	expectThat(rendersRed(top), "a layer added after sublayers = nil under a CAContext root renders");
	[context invalidate];
#endif
	[pool release];
}

int main(void)
{
	signal(SIGALRM, watchdog);
	alarm(50);

	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	inBothStates(addScenario, "add");
	inBothStates(insertIndexScenario, "insert at index");
	inBothStates(insertSiblingScenario, "insert above/below");
	inBothStates(replaceScenario, "replace");
	inBothStates(removeScenario, "remove");
	inBothStates(moveScenario, "move");
	setSublayersChecks();

#ifndef NO_RENDER
	if (!setUpReadback()) {
		printf("FAIL: no offscreen OpenGL framebuffer\n");
		failures++;
	} else {
		renderChecks();
	}
#endif

	[pool release];
	printf("%s\n", failures == 0 ? "PASS" : "FAIL");
	return failures == 0 ? 0 : 1;
}
