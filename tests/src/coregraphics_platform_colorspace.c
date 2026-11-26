// Copyright (C) 2026 Lubos Dolezel
//
// CGColorSpaceCreateWithPlatformColorSpace(): the platform colour space is a ColorSync profile (the legacy
// profile reference of the 32-bit API is one), and CGAffineTransformMake() is an exported function.
#include <ApplicationServices/ApplicationServices.h>
#include <dlfcn.h>
#include <stdio.h>

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

int main(void)
{
	CFTypeRef profile = ColorSyncProfileCreateWithName(kColorSyncSRGBProfile);
	CGColorSpaceRef space;
	CFDataRef icc;

	CHECK(profile != NULL);
	space = CGColorSpaceCreateWithPlatformColorSpace(profile);
	CHECK(space != NULL && CGColorSpaceGetNumberOfComponents(space) == 3 && CGColorSpaceGetModel(space) == kCGColorSpaceModelRGB);
	icc = space ? CGColorSpaceCopyICCProfile(space) : NULL;
	CHECK(icc != NULL);
	if (icc)
		CFRelease(icc);
	if (space)
		CGColorSpaceRelease(space);

	// Nothing and something that is no profile give no colour space
	CHECK(CGColorSpaceCreateWithPlatformColorSpace(NULL) == NULL);
	CHECK(CGColorSpaceCreateWithPlatformColorSpace(CFSTR("no profile")) == NULL);
	if (profile)
		CFRelease(profile);

	// The symbol of the function, for programs built against a header that makes it a function
	CGAffineTransform (*make)(CGFloat, CGFloat, CGFloat, CGFloat, CGFloat, CGFloat) = dlsym(RTLD_DEFAULT, "CGAffineTransformMake");
	CHECK(make != NULL);
	if (make)
	{
		CGAffineTransform t = make(1, 2, 3, 4, 5, 6);
		CHECK(t.a == 1 && t.b == 2 && t.c == 3 && t.d == 4 && t.tx == 5 && t.ty == 6);
	}

	printf(g_failures ? "FAIL coregraphics_platform_colorspace\n" : "PASS coregraphics_platform_colorspace\n");
	return g_failures != 0;
}
