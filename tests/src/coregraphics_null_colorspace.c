// Copyright (C) 2026 Lubos Dolezel
//
// CGContextSetFillColorSpace and CGContextSetStrokeColorSpace with a NULL colour space: no crash, and the colour of
// the context becomes black. Black-box observation on macOS 10.14.4.
#include <ApplicationServices/ApplicationServices.h>
#include <stdio.h>

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

int main(void)
{
	CGColorSpaceRef rgb = CGColorSpaceCreateDeviceRGB();
	CGContextRef context = CGBitmapContextCreate(NULL, 4, 4, 8, 16, rgb, kCGImageAlphaPremultipliedLast);

	CHECK(context != NULL);
	CGContextSetRGBFillColor(context, 1, 0, 0, 1);
	// QUIRK: a NULL colour space makes the colour black, and is no crash. Evidence: black-box observation on
	// macOS 10.14.4 x86_64 (tier 2). Test: this test (test_quirk_null_colorspace_black).
	CGContextSetFillColorSpace(context, NULL);
	CGContextSetStrokeColorSpace(context, NULL);
	CGContextFillRect(context, CGRectMake(0, 0, 4, 4));
	unsigned char* pixels = CGBitmapContextGetData(context);
	CHECK(pixels && pixels[0] == 0 && pixels[1] == 0 && pixels[2] == 0 && pixels[3] == 255);

	CGContextRelease(context);
	CGColorSpaceRelease(rgb);
	printf(g_failures ? "FAIL coregraphics_null_colorspace\n" : "PASS coregraphics_null_colorspace\n");
	return g_failures != 0;
}
