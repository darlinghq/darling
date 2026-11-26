// Copyright (C) 2026 Lubos Dolezel
//
// Contract tests of CGContextAddArc and CGContextDrawPath, from the documented behaviour:
// - an arc whose sweep is a full circle (0 to 2*pi, also when the end angle is a hair above 2*pi because it was
//   computed in single precision) adds the whole circle, and is not an empty arc;
// - kCGPathFillStroke strokes the path also when the fill colour is fully transparent.
#include <ApplicationServices/ApplicationServices.h>
#include <stdio.h>
#include <math.h>

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

enum { kSize = 100 };

static CGContextRef makeContext(void)
{
	CGColorSpaceRef rgb = CGColorSpaceCreateDeviceRGB();
	CGContextRef context = CGBitmapContextCreate(NULL, kSize, kSize, 8, kSize * 4, rgb, kCGImageAlphaPremultipliedLast);

	CGColorSpaceRelease(rgb);
	return context;
}

// Alpha of the pixel at x, y (y counted from the top row of the bitmap)
static unsigned alphaAt(CGContextRef context, int x, int y)
{
	const unsigned char* pixels = CGBitmapContextGetData(context);

	return pixels[y * kSize * 4 + x * 4 + 3];
}

static void strokeCircle(float endAngle, int expectOnEdge)
{
	CGContextRef context = makeContext();

	CHECK(context != NULL);
	CGContextSetRGBStrokeColor(context, 0, 0, 0, 1);
	CGContextSetLineWidth(context, 4);
	CGContextBeginPath(context);
	CGContextAddArc(context, 50, 50, 30, 0.0, endAngle, 0);
	CGContextStrokePath(context);
	// the circle has the radius 30 around (50, 50): the points at the four axis directions are on the stroke
	CHECK((alphaAt(context, 80, 50) > 200) == expectOnEdge);
	CHECK((alphaAt(context, 20, 50) > 200) == expectOnEdge);
	CHECK((alphaAt(context, 50, 80) > 200) == expectOnEdge);
	CHECK((alphaAt(context, 50, 20) > 200) == expectOnEdge);
	// the centre is not on it
	CHECK(alphaAt(context, 50, 50) == 0);
	CGContextRelease(context);
}

int main(void)
{
	strokeCircle((float) (2 * M_PI), 1);	// the end angle in single precision is above 2*pi
	strokeCircle(2 * M_PI, 1);
	strokeCircle(4 * M_PI, 1);	// a sweep over a full circle is a full circle

	// a path that is filled with a transparent colour and stroked still has its stroke
	{
		CGContextRef context = makeContext();

		CGContextSetRGBFillColor(context, 1, 1, 1, 0);
		CGContextSetRGBStrokeColor(context, 0, 0, 0, 1);
		CGContextSetLineWidth(context, 6);
		CGContextAddRect(context, CGRectMake(20, 20, 60, 60));
		CGContextDrawPath(context, kCGPathFillStroke);
		CHECK(alphaAt(context, 50, 20) == 255);
		CHECK(alphaAt(context, 20, 50) == 255);
		CHECK(alphaAt(context, 50, 50) == 0);
		CGContextRelease(context);
	}
	printf(g_failures ? "FAIL coregraphics_arc_fill_stroke\n" : "PASS coregraphics_arc_fill_stroke\n");
	return g_failures != 0;
}
