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
 * The scalar types of the Core Animation properties are part of the ABI: a
 * caller compiled against the public headers passes a float in a floating
 * point register, and the setter must read a float, not a double.
 *
 * - CALayer opacity, shadowOpacity and minificationFilterBias, CAMediaTiming
 *   speed and repeatCount, CATransition startProgress and endProgress and the
 *   CAEmitterLayer rates are float; the other layer metrics are CGFloat and the
 *   times CFTimeInterval.
 * - The type encodings below were read on macOS 26.7.3 (x86_64) with
 *   method_getTypeEncoding.
 *
 * The test checks the encodings, set/get round trips through the declared
 * API and through key-value coding, and that a CARenderer draws a layer at
 * the coverage of an opacity set from a float. It must be built against the SDK
 * headers, as applications see them. The renderer part needs OpenGL (a
 * display on Darling) and prints SKIP without it. No ARC.
 */
#define GL_GLEXT_PROTOTYPES 1
#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#import <objc/runtime.h>
#include <ctype.h>
#include <stdlib.h>
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
	static const char msg[] = "FAIL: timed out\n";
	(void) sig;
	write(1, msg, sizeof(msg) - 1);
	_exit(1);
}

#define F "f"
#define D "d"
#define G @encode(CGFloat)
#define U "I"

typedef struct {
	const char *className;
	const char *name;
	const char *type; // the value type of the getter's result and the setter's argument
} Property;

static const Property properties[] = {
	{ "CALayer", "opacity", F },
	{ "CALayer", "shadowOpacity", F },
	{ "CALayer", "minificationFilterBias", F },
	{ "CALayer", "zPosition", G },
	{ "CALayer", "anchorPointZ", G },
	{ "CALayer", "contentsScale", G },
	{ "CALayer", "borderWidth", G },
	{ "CALayer", "cornerRadius", G },
	{ "CALayer", "shadowRadius", G },
	{ "CALayer", "rasterizationScale", G },
	{ "CABasicAnimation", "beginTime", D },
	{ "CABasicAnimation", "duration", D },
	{ "CABasicAnimation", "speed", F },
	{ "CABasicAnimation", "timeOffset", D },
	{ "CABasicAnimation", "repeatCount", F },
	{ "CABasicAnimation", "repeatDuration", D },
	{ "CAEmitterLayer", "birthRate", F },
	{ "CAEmitterLayer", "lifetime", F },
	{ "CAEmitterLayer", "velocity", F },
	{ "CAEmitterLayer", "scale", F },
	{ "CAEmitterLayer", "spin", F },
	{ "CAEmitterLayer", "emitterZPosition", G },
	{ "CAEmitterLayer", "emitterDepth", G },
	{ "CAEmitterLayer", "seed", U },
	{ "CATransition", "startProgress", F },
	{ "CATransition", "endProgress", F },
	{ "CAShapeLayer", "strokeStart", G },
	{ "CAShapeLayer", "strokeEnd", G },
	{ "CAShapeLayer", "lineWidth", G },
	{ "CAShapeLayer", "miterLimit", G },
	{ "CAShapeLayer", "lineDashPhase", G },
	{ "CATextLayer", "fontSize", G },
	{ "CASpringAnimation", "mass", G },
	{ "CASpringAnimation", "stiffness", G },
	{ "CASpringAnimation", "damping", G },
	{ "CASpringAnimation", "initialVelocity", G },
};

static NSString *setterName(const char *name)
{
	return [NSString stringWithFormat: @"set%c%s:", toupper(name[0]), name + 1];
}

static void checkEncodings(void)
{
	for (size_t i = 0; i < sizeof(properties) / sizeof(properties[0]); i++) {
		const Property *p = &properties[i];
		Class cls = objc_getClass(p->className);
		Method getter = class_getInstanceMethod(cls, sel_registerName(p->name));
		Method setter = class_getInstanceMethod(cls, NSSelectorFromString(setterName(p->name)));
		if (getter == NULL || setter == NULL) {
			CHECK(0, "-[%s %s] has a getter and a setter", p->className, p->name);
			continue;
		}
		char *result = method_copyReturnType(getter);
		char *argument = method_copyArgumentType(setter, 2);
		CHECK(strcmp(result, p->type) == 0 && strcmp(argument, p->type) == 0,
		      "-[%s %s] getter returns %s, setter takes %s (expected %s)",
		      p->className, p->name, result, argument, p->type);
		free(result);
		free(argument);
	}

	Method duration = class_getClassMethod(objc_getClass("CATransaction"), @selector(animationDuration));
	char *result = method_copyReturnType(duration);
	CHECK(strcmp(result, D) == 0, "+[CATransaction animationDuration] returns %s (expected %s)", result, D);
	free(result);

	Method points = class_getInstanceMethod(objc_getClass("CAMediaTimingFunction"),
	                                        @selector(getControlPointAtIndex:values:));
	char *argument = points ? method_copyArgumentType(points, 3) : strdup("?");
	CHECK(strcmp(argument, "[2f]") == 0, "-[CAMediaTimingFunction getControlPointAtIndex:values:] takes %s (expected [2f])", argument);
	free(argument);

	Method creator = class_getClassMethod(objc_getClass("CAMediaTimingFunction"),
	                                      @selector(functionWithControlPoints::::));
	BOOL floats = creator != NULL;
	for (unsigned i = 2; creator != NULL && i < 6; i++) {
		char *type = method_copyArgumentType(creator, i);
		floats = floats && strcmp(type, F) == 0;
		free(type);
	}
	CHECK(floats, "+[CAMediaTimingFunction functionWithControlPoints::::] takes four floats");
}

static BOOL sameFloat(float a, float b) { return memcmp(&a, &b, sizeof(float)) == 0; }

// 0.1 is not representable in a float: a float property returns 0.1f, a
// CGFloat property on a 64-bit build keeps the double.
#define VALUE 0.1

#define FLOAT_ROUND_TRIP(object, cls, property, setter) \
	do { \
		[object setter VALUE]; \
		CHECK(sameFloat([object property], (float) VALUE), \
		      "%s %s round-trips a float through the setter and getter", #cls, #property); \
		[object setValue: [NSNumber numberWithFloat: 0.5f] forKey: @#property]; \
		NSNumber *kvc = [object valueForKey: @#property]; \
		CHECK(sameFloat([kvc floatValue], 0.5f) && strcmp([kvc objCType], "f") == 0 && \
		      sameFloat([object property], 0.5f), \
		      "%s %s round-trips through key-value coding (value type %s)", #cls, #property, [kvc objCType]); \
	} while (0)

#define CGFLOAT_ROUND_TRIP(object, cls, property, setter) \
	do { \
		[object setter VALUE]; \
		CHECK([object property] == (CGFloat) VALUE, \
		      "%s %s round-trips a CGFloat through the setter and getter", #cls, #property); \
	} while (0)

static void checkRoundTrips(void)
{
	CALayer *layer = [CALayer layer];
	FLOAT_ROUND_TRIP(layer, CALayer, opacity, setOpacity:);
	FLOAT_ROUND_TRIP(layer, CALayer, shadowOpacity, setShadowOpacity:);
	FLOAT_ROUND_TRIP(layer, CALayer, minificationFilterBias, setMinificationFilterBias:);
	layer.opacity = 1.0f;
	CHECK(layer.opacity == 1.0f, "CALayer opacity set to 1.0f reads back as 1.0");
	CGFLOAT_ROUND_TRIP(layer, CALayer, zPosition, setZPosition:);
	CGFLOAT_ROUND_TRIP(layer, CALayer, borderWidth, setBorderWidth:);
	CGFLOAT_ROUND_TRIP(layer, CALayer, cornerRadius, setCornerRadius:);
	CGFLOAT_ROUND_TRIP(layer, CALayer, shadowRadius, setShadowRadius:);

	CABasicAnimation *animation = [CABasicAnimation animation];
	FLOAT_ROUND_TRIP(animation, CAAnimation, speed, setSpeed:);
	FLOAT_ROUND_TRIP(animation, CAAnimation, repeatCount, setRepeatCount:);
	animation.speed = 1.0f;
	CHECK(animation.speed == 1.0f, "CAAnimation speed set to 1.0f reads back as 1.0");
	animation.duration = 0.1;
	CHECK(animation.duration == 0.1, "CAAnimation duration keeps the double");

	CAEmitterLayer *emitter = [CAEmitterLayer layer];
	FLOAT_ROUND_TRIP(emitter, CAEmitterLayer, birthRate, setBirthRate:);
	FLOAT_ROUND_TRIP(emitter, CAEmitterLayer, lifetime, setLifetime:);
	FLOAT_ROUND_TRIP(emitter, CAEmitterLayer, velocity, setVelocity:);
	FLOAT_ROUND_TRIP(emitter, CAEmitterLayer, scale, setScale:);
	FLOAT_ROUND_TRIP(emitter, CAEmitterLayer, spin, setSpin:);

	CATransition *transition = [CATransition animation];
	FLOAT_ROUND_TRIP(transition, CATransition, startProgress, setStartProgress:);
	FLOAT_ROUND_TRIP(transition, CATransition, endProgress, setEndProgress:);

	CAShapeLayer *shape = [CAShapeLayer layer];
	CGFLOAT_ROUND_TRIP(shape, CAShapeLayer, lineWidth, setLineWidth:);
	CGFLOAT_ROUND_TRIP(shape, CAShapeLayer, strokeEnd, setStrokeEnd:);

	CATextLayer *text = [CATextLayer layer];
	CGFLOAT_ROUND_TRIP(text, CATextLayer, fontSize, setFontSize:);

	CAMediaTimingFunction *function = [CAMediaTimingFunction functionWithControlPoints: 0.1f :0.2f :0.3f :0.4f];
	float p1[2] = { 0, 0 }, p2[2] = { 0, 0 };
	[function getControlPointAtIndex: 1 values: p1];
	[function getControlPointAtIndex: 2 values: p2];
	CHECK(sameFloat(p1[0], 0.1f) && sameFloat(p1[1], 0.2f) && sameFloat(p2[0], 0.3f) && sameFloat(p2[1], 0.4f),
	      "CAMediaTimingFunction control points round-trip floats");
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

// The red of the middle of the picture: red layers over black, so it is the
// layer's coverage (the framebuffer's own alpha is not meaningful).
static int renderCoverage(CALayer *top)
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
	const uint8_t *middle = pixels + (SIZE / 2 * SIZE + SIZE / 2) * 4;
	return middle[0];
}

static CALayer *redLayer(void)
{
	CALayer *layer = [CALayer layer];
	layer.anchorPoint = CGPointZero;
	layer.frame = CGRectMake(0, 0, SIZE, SIZE);
	CGColorRef red = CGColorCreateGenericRGB(1, 0, 0, 1);
	layer.backgroundColor = red;
	CGColorRelease(red);
	return layer;
}

// Each picture is of a new tree: the renderer remembers what it has drawn.
static int coverageWithOpacity(float opacity)
{
	CALayer *layer = redLayer();
	layer.opacity = opacity;
	return renderCoverage(layer);
}

static void checkRendering(void)
{
	if (!setUpReadback()) {
		printf("SKIP: no OpenGL context for the CARenderer readback\n");
		return;
	}

	CALayer *plain = redLayer();
	int red = renderCoverage(plain);
	if (red == 0) {
		// On macOS an offscreen CARenderer without a Metal device draws nothing.
		printf("SKIP: the CARenderer draws nothing here, not even a layer with the default opacity\n");
		return;
	}
	CHECK(red >= 250, "a layer with the default opacity draws opaque (red %d)", red);

	red = coverageWithOpacity(1.0f);
	CHECK(red >= 250, "a layer with opacity 1.0f draws opaque (red %d)", red);

	red = coverageWithOpacity(0.5f);
	CHECK(red >= 124 && red <= 132, "a layer with opacity 0.5f draws at half coverage (red %d)", red);

	CALayer *parent = [CALayer layer];
	parent.anchorPoint = CGPointZero;
	parent.frame = CGRectMake(0, 0, SIZE, SIZE);
	parent.opacity = 0.5f;
	CALayer *child = redLayer();
	child.opacity = 0.5f;
	[parent addSublayer: child];
	red = renderCoverage(parent);
	CHECK(red >= 60 && red <= 68, "nested opacities 0.5f multiply to a quarter coverage (red %d)", red);

	red = coverageWithOpacity(0.0f);
	CHECK(red == 0, "a layer with opacity 0.0f draws nothing (red %d)", red);
}

int main(void)
{
	signal(SIGALRM, watchdog);
	alarm(50);
	@autoreleasepool {
		checkEncodings();
		checkRoundTrips();
		checkRendering();
	}
	printf(failures ? "%d FAILED\n" : "ALL PASSED\n", failures);
	return failures ? 1 : 0;
}
