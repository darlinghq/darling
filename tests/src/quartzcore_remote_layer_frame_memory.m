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
 * A CALayerHost that shows a CAContext (private API that applications
 * declare themselves) whose tree changes on every frame must not use more
 * memory the longer it runs: each frame replaces the last one, so the
 * process's memory stays bounded however many frames go by.
 *
 * A context's tree has a layer whose contents is an IOSurface, replaced by
 * another surface of a small pool on every frame, as a compositor that
 * renders into a ring of surfaces does. Cases:
 * - "cpu": the pool's surfaces are written by the CPU;
 * - "gl": another OpenGL context renders into them through
 *   CGLTexImageIOSurface2D (as ANGLE does);
 * - "resize": as "cpu", and the layer also changes size on every frame, so
 *   the context's frames change size too.
 * The host's tree is drawn on every frame, either by an offscreen CARenderer
 * ("offscreen") or in a window ("window"), as an application that hosts
 * another process's layers in a window does.
 *
 * Each case runs for a few seconds after a warm-up, then compares the
 * number of mappings (/proc/self/maps of the host process) and the resident
 * size (task_info) with those after the warm-up. A process that keeps
 * something per frame grows by many mappings or megabytes in that time.
 *
 * Arguments: [offscreen|window] [cpu|gl|resize ...] [-s seconds]
 * With no case named, all three run. Needs OpenGL; "window" needs a display
 * (X11 or Wayland), "offscreen" does not. No ARC.
 */
#define GL_GLEXT_PROTOTYPES 1
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#import <IOSurface/IOSurface.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#import <OpenGL/CGLIOSurface.h>
#include <fcntl.h>
#include <mach/mach.h>
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

#define SIZE 400
#define POOL 3
#define WIDTH 320
#define HEIGHT 240
#define WARMUP_SECONDS 2.0
// Growth allowed between the end of the warm-up and the end of a case.
#define MAX_NEW_MAPPINGS 64
#define MAX_NEW_RESIDENT_MB 64

enum mode { CPU, GL, RESIZE };

static int failures;
static BOOL inWindow;
static double seconds = 8;

static void watchdog(int sig)
{
	static const char msg[] = "FAIL: timed out\n";
	(void) sig;
	write(1, msg, sizeof(msg) - 1);
	_exit(1);
}

static long mappingCount(void)
{
	// Darling shows the host's file system under /Volumes/SystemRoot.
	FILE *maps = fopen("/Volumes/SystemRoot/proc/self/maps", "r");
	if (maps == NULL)
		return -1;
	long lines = 0;
	int c;
	while ((c = fgetc(maps)) != EOF) {
		if (c == '\n')
			lines++;
	}
	fclose(maps);
	return lines;
}

static double residentMB(void)
{
	mach_task_basic_info_data_t info;
	mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
	if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t) &info, &count) != KERN_SUCCESS)
		return -1;
	return info.resident_size / (1024.0 * 1024.0);
}

static long descriptorCount(void)
{
	long count = 0;
	for (int fd = 0; fd < 4096; fd++) {
		if (fcntl(fd, F_GETFD) != -1)
			count++;
	}
	return count;
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

// The offscreen host: an OpenGL framebuffer that an offscreen CARenderer
// draws the host's tree into.
static CGLContextObj hostContext;
static GLuint hostFramebuffer;

static BOOL setUpOffscreenHost(void)
{
	hostContext = createContext();
	if (hostContext == NULL)
		return NO;
	CGLSetCurrentContext(hostContext);
	GLuint renderbuffer;
	glGenFramebuffers(1, &hostFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, hostFramebuffer);
	glGenRenderbuffers(1, &renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, SIZE, SIZE);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);
	BOOL complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	CGLSetCurrentContext(NULL);
	return complete;
}

static CARenderer *offscreenRenderer;

// Draws the host's tree; returns whether the hosted content shows.
static BOOL renderOffscreen(CALayer *top)
{
	static uint8_t pixel[4];

	CGLSetCurrentContext(hostContext);
	glBindFramebuffer(GL_FRAMEBUFFER, hostFramebuffer);
	glViewport(0, 0, SIZE, SIZE);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, SIZE, 0, SIZE, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	if (offscreenRenderer == nil) {
		offscreenRenderer = [[CARenderer rendererWithCGLContext: hostContext options: nil] retain];
		offscreenRenderer.bounds = CGRectMake(0, 0, SIZE, SIZE);
	}
	offscreenRenderer.layer = top;
	[offscreenRenderer beginFrameAtTime: CACurrentMediaTime() timeStamp: NULL];
	if ([offscreenRenderer respondsToSelector: @selector(addUpdateRect:)])
		[offscreenRenderer addUpdateRect: offscreenRenderer.bounds];
	[offscreenRenderer render];
	[offscreenRenderer endFrame];

	glReadPixels(10, 10, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	CGLSetCurrentContext(NULL);
	return pixel[3] > 200;
}

// Writes a colour that changes with the frame into a surface.
static void fillByCPU(IOSurfaceRef surface, unsigned frame)
{
	if (IOSurfaceLock(surface, 0, NULL) != kIOReturnSuccess)
		return;
	uint32_t color = 0xff000000u | ((frame * 7) & 0xff) << 16 | ((frame * 3) & 0xff) << 8 | 0x80;
	uint8_t *base = IOSurfaceGetBaseAddress(surface);
	size_t rowBytes = IOSurfaceGetBytesPerRow(surface);
	for (size_t y = 0; y < IOSurfaceGetHeight(surface); y++) {
		uint32_t *row = (uint32_t *) (base + y * rowBytes);
		for (size_t x = 0; x < IOSurfaceGetWidth(surface); x++)
			row[x] = color;
	}
	IOSurfaceUnlock(surface, 0, NULL);
}

// Renders into the surface through a texture bound to it in the producer's
// context, kept as ANGLE keeps it while the surface is in use.
static CGLContextObj producerContext;
static GLuint producerTextures[POOL];
static GLuint producerFramebuffer;

static BOOL fillByGL(int index, IOSurfaceRef surface, unsigned frame)
{
	CGLContextObj previous = CGLGetCurrentContext();
	CGLSetCurrentContext(producerContext);
	if (producerFramebuffer == 0)
		glGenFramebuffers(1, &producerFramebuffer);
	BOOL ok = YES;
	if (producerTextures[index] == 0) {
		glGenTextures(1, &producerTextures[index]);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, producerTextures[index]);
		ok = CGLTexImageIOSurface2D(producerContext, GL_TEXTURE_RECTANGLE_ARB, GL_RGBA,
		                            (GLsizei) IOSurfaceGetWidth(surface), (GLsizei) IOSurfaceGetHeight(surface),
		                            GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, surface, 0) == kCGLNoError;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, producerFramebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE_ARB,
	                       producerTextures[index], 0);
	ok = ok && glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	if (ok) {
		glViewport(0, 0, (GLsizei) IOSurfaceGetWidth(surface), (GLsizei) IOSurfaceGetHeight(surface));
		glClearColor((frame % 7) / 7.0, (frame % 5) / 5.0, 0.5, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		glFlush();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	CGLSetCurrentContext(previous);
	return ok;
}

static void runLoopFor(NSTimeInterval interval)
{
	[[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow: interval]];
}

static const char *modeName(enum mode mode)
{
	return mode == CPU ? "cpu" : mode == GL ? "gl" : "resize";
}

static void runCase(enum mode mode, NSView *view)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	const char *name = modeName(mode);

	IOSurfaceRef surfaces[POOL];
	for (int i = 0; i < POOL; i++)
		surfaces[i] = createSurface(WIDTH, HEIGHT);
	if (surfaces[0] == NULL || surfaces[1] == NULL || surfaces[2] == NULL) {
		printf("FAIL: %s: cannot create surfaces\n", name);
		failures++;
		return;
	}
	if (mode == GL) {
		producerContext = createContext();
		if (producerContext == NULL) {
			printf("FAIL: %s: no producer OpenGL context\n", name);
			failures++;
			return;
		}
	}

	CALayer *root = [CALayer layer];
	root.anchorPoint = CGPointZero;
	root.frame = CGRectMake(0, 0, WIDTH, HEIGHT);
	CALayer *content = [CALayer layer];
	content.anchorPoint = CGPointZero;
	content.frame = CGRectMake(0, 0, WIDTH, HEIGHT);
	[root addSublayer: content];

	CAContext *context = [CAContext contextWithCGSConnection: 0 options: @{}];
	context.layer = root;
	if (context == nil || context.contextId == 0) {
		printf("FAIL: %s: no context\n", name);
		failures++;
		return;
	}

	CALayer *top = nil;
	CALayerHost *host = [CALayerHost layer];
	host.anchorPoint = CGPointZero;
	host.frame = CGRectMake(0, 0, WIDTH, HEIGHT);
	host.contextId = context.contextId;
	if (view != nil) {
		[view.layer addSublayer: host];
	} else {
		top = [CALayer layer];
		top.anchorPoint = CGPointZero;
		top.frame = CGRectMake(0, 0, SIZE, SIZE);
		[top addSublayer: host];
	}

	unsigned frame = 0, shown = 0;
	long startMappings = -1, startDescriptors = -1;
	double startResident = -1;
	NSDate *start = [NSDate date];
	for (;;) {
		NSAutoreleasePool *framePool = [[NSAutoreleasePool alloc] init];
		double elapsed = -[start timeIntervalSinceNow];
		if (elapsed >= WARMUP_SECONDS + seconds) {
			[framePool release];
			break;
		}
		if (startMappings < 0 && elapsed >= WARMUP_SECONDS) {
			startMappings = mappingCount();
			startResident = residentMB();
			startDescriptors = descriptorCount();
		}

		int index = frame % POOL;
		if (mode == GL)
			fillByGL(index, surfaces[index], frame);
		else
			fillByCPU(surfaces[index], frame);
		[CATransaction begin];
		[CATransaction setDisableActions: YES];
		content.contents = (id) surfaces[index];
		if (mode == RESIZE) {
			// The tree's extent, and so the context's frame, changes size.
			CGFloat width = WIDTH - (frame % 32) * 4;
			content.frame = CGRectMake(0, 0, width, HEIGHT);
			root.frame = CGRectMake(0, 0, width, HEIGHT);
		}
		[CATransaction commit];
		// Lets the context render and send the frame, and the host take it.
		runLoopFor(1.0 / 60);
		if (top != nil && renderOffscreen(top))
			shown++;
		frame++;
		[framePool release];
	}

	long endMappings = mappingCount();
	double endResident = residentMB();
	long endDescriptors = descriptorCount();
	printf("%s %s: %u frames, mappings %ld -> %ld, resident %.1f -> %.1f MB, descriptors %ld -> %ld\n",
	       inWindow ? "window" : "offscreen", name, frame, startMappings, endMappings, startResident,
	       endResident, startDescriptors, endDescriptors);

	BOOL ok = YES;
	if (startMappings < 0 || endMappings < 0) {
		printf("FAIL: %s: cannot read the mappings\n", name);
		ok = NO;
	} else if (endMappings - startMappings > MAX_NEW_MAPPINGS) {
		printf("FAIL: %s: %ld new mappings in %.0f s\n", name, endMappings - startMappings, seconds);
		ok = NO;
	}
	if (startResident < 0 || endResident - startResident > MAX_NEW_RESIDENT_MB) {
		printf("FAIL: %s: resident size grew by %.1f MB in %.0f s\n", name, endResident - startResident,
		       seconds);
		ok = NO;
	}
	if (endDescriptors - startDescriptors > 8) {
		printf("FAIL: %s: %ld new descriptors\n", name, endDescriptors - startDescriptors);
		ok = NO;
	}
	if (top != nil && shown == 0) {
		printf("FAIL: %s: the hosted content never showed\n", name);
		ok = NO;
	}
	if (ok)
		printf("PASS: %s\n", name);
	else
		failures++;

	[context invalidate];
	[host removeFromSuperlayer];
	runLoopFor(0.1);
	if (mode == GL) {
		CGLSetCurrentContext(producerContext);
		glDeleteTextures(POOL, producerTextures);
		memset(producerTextures, 0, sizeof(producerTextures));
		glDeleteFramebuffers(1, &producerFramebuffer);
		producerFramebuffer = 0;
		CGLSetCurrentContext(NULL);
		CGLDestroyContext(producerContext);
		producerContext = NULL;
	}
	for (int i = 0; i < POOL; i++)
		CFRelease(surfaces[i]);
	[pool release];
}

int main(int argc, char **argv)
{
	BOOL modes[3] = { NO, NO, NO }, any = NO;
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "window") == 0)
			inWindow = YES;
		else if (strcmp(argv[i], "offscreen") == 0)
			inWindow = NO;
		else if (strcmp(argv[i], "cpu") == 0)
			modes[CPU] = any = YES;
		else if (strcmp(argv[i], "gl") == 0)
			modes[GL] = any = YES;
		else if (strcmp(argv[i], "resize") == 0)
			modes[RESIZE] = any = YES;
		else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc)
			seconds = atof(argv[++i]);
		else {
			fprintf(stderr, "usage: %s [offscreen|window] [cpu|gl|resize ...] [-s seconds]\n", argv[0]);
			return 2;
		}
	}
	if (!any)
		modes[CPU] = modes[GL] = modes[RESIZE] = YES;

	signal(SIGALRM, watchdog);
	alarm((unsigned) (3 * (WARMUP_SECONDS + seconds + 2)) + 20);

	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	NSView *view = nil;
	if (inWindow) {
		[NSApplication sharedApplication];
		[NSApp setActivationPolicy: NSApplicationActivationPolicyRegular];
		NSWindow *window = [[NSWindow alloc] initWithContentRect: NSMakeRect(100, 100, SIZE, SIZE)
		                                               styleMask: NSWindowStyleMaskTitled
		                                                 backing: NSBackingStoreBuffered
		                                                   defer: NO];
		view = [window contentView];
		[view setWantsLayer: YES];
		[window makeKeyAndOrderFront: nil];
		runLoopFor(0.5);
	} else if (!setUpOffscreenHost()) {
		printf("FAIL: no offscreen OpenGL framebuffer\n");
		return 1;
	}

	for (int mode = CPU; mode <= RESIZE; mode++) {
		if (modes[mode])
			runCase((enum mode) mode, view);
	}

	[pool release];
	printf("%s\n", failures == 0 ? "PASS" : "FAIL");
	return failures == 0 ? 0 : 1;
}
